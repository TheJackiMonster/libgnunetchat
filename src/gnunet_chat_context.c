/*
   This file is part of GNUnet.
   Copyright (C) 2021--2022 GNUnet e.V.

   GNUnet is free software: you can redistribute it and/or modify it
   under the terms of the GNU Affero General Public License as published
   by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   GNUnet is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   SPDX-License-Identifier: AGPL3.0-or-later
 */
/*
 * @author Tobias Frisch
 * @file gnunet_chat_context.c
 */

#include "gnunet_chat_context.h"
#include "gnunet_chat_handle.h"
#include "gnunet_chat_util.h"

#include "gnunet_chat_context_intern.c"

struct GNUNET_CHAT_Context*
context_create_from_room (struct GNUNET_CHAT_Handle *handle,
			  struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert((handle) && (room));

  struct GNUNET_CHAT_Context* context = GNUNET_new(struct GNUNET_CHAT_Context);

  context->handle = handle;

  context->type = GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN;
  context->nick = NULL;

  context->timestamps = GNUNET_CONTAINER_multishortmap_create(8, GNUNET_NO);
  context->messages = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  context->invites = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  context->files = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);

  context->room = room;
  context->contact = NULL;

  context->user_pointer = NULL;

  return context;
}

struct GNUNET_CHAT_Context*
context_create_from_contact (struct GNUNET_CHAT_Handle *handle,
			     const struct GNUNET_MESSENGER_Contact *contact)
{
  GNUNET_assert((handle) && (contact));

  struct GNUNET_CHAT_Context* context = GNUNET_new(struct GNUNET_CHAT_Context);

  context->handle = handle;

  context->type = GNUNET_CHAT_CONTEXT_TYPE_CONTACT;
  context->nick = NULL;

  context->timestamps = GNUNET_CONTAINER_multishortmap_create(4, GNUNET_NO);
  context->messages = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);
  context->invites = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);
  context->files = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);

  context->room = NULL;
  context->contact = contact;

  context->user_pointer = NULL;

  return context;
}

void
context_destroy (struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((context) &&
		(context->timestamps) &&
		(context->messages) &&
		(context->invites) &&
		(context->files));

  GNUNET_CONTAINER_multishortmap_iterate(
      context->timestamps, it_destroy_context_timestamps, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      context->messages, it_destroy_context_messages, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      context->invites, it_destroy_context_invites, NULL
  );

  GNUNET_CONTAINER_multishortmap_destroy(context->timestamps);
  GNUNET_CONTAINER_multihashmap_destroy(context->messages);
  GNUNET_CONTAINER_multihashmap_destroy(context->invites);
  GNUNET_CONTAINER_multihashmap_destroy(context->files);

  if (context->nick)
    GNUNET_free(context->nick);

  GNUNET_free(context);
}

void
context_update_room (struct GNUNET_CHAT_Context *context,
		     struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert(context);

  if (room == context->room)
    return;

  GNUNET_CONTAINER_multishortmap_iterate(
      context->timestamps, it_destroy_context_timestamps, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      context->messages, it_destroy_context_messages, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      context->invites, it_destroy_context_invites, NULL
  );

  GNUNET_CONTAINER_multishortmap_destroy(context->timestamps);
  context->timestamps = GNUNET_CONTAINER_multishortmap_create(8, GNUNET_NO);

  GNUNET_CONTAINER_multihashmap_clear(context->messages);
  GNUNET_CONTAINER_multihashmap_clear(context->invites);
  GNUNET_CONTAINER_multihashmap_clear(context->files);

  context->room = room;
}

void
context_update_nick (struct GNUNET_CHAT_Context *context,
		     const char *nick)
{
  GNUNET_assert(context);

  util_set_name_field(nick, &(context->nick));

  if (!(context->handle))
    return;

  handle_send_internal_message(
      context->handle,
      context,
      GNUNET_CHAT_FLAG_UPDATE,
      NULL
  );
}

void
context_load_config (struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((context) &&
		(context->handle) &&
		(context->room));

  const char *directory = context->handle->directory;

  if (!directory)
    return;

  const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
      context->room
  );

  char* filename;
  util_get_filename(directory, "chats", hash, &filename);

  if (GNUNET_YES != GNUNET_DISK_file_test(filename))
    goto free_filename;

  struct GNUNET_CONFIGURATION_Handle *config = GNUNET_CONFIGURATION_create();

  if (GNUNET_OK != GNUNET_CONFIGURATION_load(config, filename))
    goto destroy_config;

  char* name = NULL;

  if (GNUNET_OK == GNUNET_CONFIGURATION_get_value_string(
      config, "chat", "name", &name))
    context_update_nick(context, name);

  if (name)
    GNUNET_free(name);

  unsigned long long type_number;

  if (GNUNET_OK == GNUNET_CONFIGURATION_get_value_number(
      config, "chat", "type", &type_number))
    switch (type_number) {
      case GNUNET_CHAT_CONTEXT_TYPE_CONTACT:
	context->type = GNUNET_CHAT_CONTEXT_TYPE_CONTACT;
	break;
      case GNUNET_CHAT_CONTEXT_TYPE_GROUP:
      	context->type = GNUNET_CHAT_CONTEXT_TYPE_GROUP;
      	break;
      default:
	context->type = GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN;
	break;
    }

destroy_config:
  GNUNET_CONFIGURATION_destroy(config);

free_filename:
  GNUNET_free(filename);
}

void
context_save_config (const struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((context) &&
  		(context->handle) &&
  		(context->room));

  const char *directory = context->handle->directory;

  if (!directory)
    return;

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(
      context->room
  );

  struct GNUNET_CONFIGURATION_Handle *config = GNUNET_CONFIGURATION_create();

  if (context->room)
    GNUNET_CONFIGURATION_set_value_string(
	config, "chat", "key", GNUNET_h2s_full(key)
    );

  if (context->nick)
    GNUNET_CONFIGURATION_set_value_string(
	config, "chat", "name", context->nick
    );

  if (GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN != context->type)
    GNUNET_CONFIGURATION_set_value_number(
	config, "chat", "type", context->type
    );

  char* filename;
  util_get_filename(directory, "chats", key, &filename);

  if (GNUNET_OK == GNUNET_DISK_directory_create_for_file(filename))
    GNUNET_CONFIGURATION_write(config, filename);

  GNUNET_CONFIGURATION_destroy(config);

  GNUNET_free(filename);
}

enum GNUNET_GenericReturnValue
callback_scan_for_configs (void *cls,
			   const char *filename)
{
  struct GNUNET_CHAT_Handle *handle = (struct GNUNET_CHAT_Handle*) cls;
  struct GNUNET_PeerIdentity door;
  struct GNUNET_HashCode key;

  memset(&door, 0, sizeof(door));
  memset(&key, 0, sizeof(key));

  if ((!filename) ||
      (GNUNET_OK != GNUNET_CRYPTO_get_peer_identity(handle->cfg, &door)))
    return GNUNET_YES;

  struct GNUNET_CONFIGURATION_Handle *config = GNUNET_CONFIGURATION_create();

  if (GNUNET_OK != GNUNET_CONFIGURATION_load(config, filename))
    goto destroy_config;

  char* key_value = NULL;

  if ((GNUNET_OK == GNUNET_CONFIGURATION_get_value_string(
      config, "chat", "key", &key_value)) &&
      (GNUNET_OK == GNUNET_CRYPTO_hash_from_string(key_value, &key)))
  {
    handle_send_room_name(handle, GNUNET_MESSENGER_enter_room(
    	handle->messenger, &door, &key
    ));
  }

  if (key_value)
    GNUNET_free(key_value);

destroy_config:
  GNUNET_CONFIGURATION_destroy(config);
  return GNUNET_YES;
}

void
context_scan_configs (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert((handle) && (handle->messenger));

  const char *directory = handle->directory;

  if (!directory)
    return;

  char* dirname;
  util_get_dirname(handle->directory, "chats", &dirname);

  if (GNUNET_YES != GNUNET_DISK_directory_test(dirname, GNUNET_YES))
    goto free_dirname;

  GNUNET_DISK_directory_scan(
      dirname,
      callback_scan_for_configs,
      handle
  );

free_dirname:
  GNUNET_free(dirname);
}
