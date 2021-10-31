/*
   This file is part of GNUnet.
   Copyright (C) 2021 GNUnet e.V.

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

  context->user_pointer = NULL;

  return context;
}

void
context_destroy (struct GNUNET_CHAT_Context* context)
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

  if (GNUNET_OK != GNUNET_CONFIGURATION_load(config, directory))
    goto destroy_config;

  char* name = NULL;

  if (GNUNET_OK == GNUNET_CONFIGURATION_get_value_string(
      config, "chat", "name", &name))
    util_set_name_field(name, &(context->nick));

  if (name)
    GNUNET_free(name);

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

  const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
      context->room
  );

  struct GNUNET_CONFIGURATION_Handle *config = GNUNET_CONFIGURATION_create();

  if (context->nick)
    GNUNET_CONFIGURATION_set_value_string(
	config, "chat", "name", context->nick
    );

  char* filename;
  util_get_filename(directory, "chats", hash, &filename);

  if (GNUNET_OK == GNUNET_DISK_directory_create_for_file(filename))
    GNUNET_CONFIGURATION_write(config, filename);

  GNUNET_CONFIGURATION_destroy(config);

  GNUNET_free(filename);
}
