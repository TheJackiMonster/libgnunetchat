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
 * @file gnunet_chat_handle.c
 */

#include "gnunet_chat_handle.h"

#include "gnunet_chat_handle_intern.c"

struct GNUNET_CHAT_Handle*
handle_create_from_config (const struct GNUNET_CONFIGURATION_Handle* cfg,
			   const char *directory,
			   const char *name,
			   GNUNET_CHAT_ContextMessageCallback msg_cb,
			   void *msg_cls)
{
  GNUNET_assert(cfg);

  struct GNUNET_CHAT_Handle* handle = GNUNET_new(struct GNUNET_CHAT_Handle);

  handle->cfg = cfg;
  handle->shutdown_hook = GNUNET_SCHEDULER_add_shutdown(
      on_handle_shutdown, handle
  );

  if ((directory) &&
      (GNUNET_YES == GNUNET_DISK_directory_test(directory, GNUNET_YES)))
    handle->directory = GNUNET_strdup(directory);
  else
    handle->directory = NULL;

  handle->msg_cb = msg_cb;
  handle->msg_cls = msg_cls;

  handle->files = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  handle->contexts = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  handle->contacts = GNUNET_CONTAINER_multishortmap_create(8, GNUNET_NO);
  handle->groups = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);

  handle->arm = GNUNET_ARM_connect(
      handle->cfg,
      on_handle_arm_connection, handle
  );

  if (handle->arm)
    on_handle_arm_connection(handle, GNUNET_NO);

  char* fs_client_name = NULL;
  GNUNET_asprintf (
      &fs_client_name,
      "GNUNET_CHAT_%s%s",
      name? "_" : "anonymous",
      name? name : ""
  );

  handle->fs = GNUNET_FS_start(
      handle->cfg, fs_client_name,
      notify_handle_fs_progress, handle,
      GNUNET_FS_FLAGS_NONE,
      GNUNET_FS_OPTIONS_END
  );

  GNUNET_free(fs_client_name);

  handle->messenger = GNUNET_MESSENGER_connect(
      handle->cfg, name,
      on_handle_identity, handle,
      on_handle_message, handle
  );

  handle->public_key = NULL;
  handle->user_pointer = NULL;

  handle_update_key(handle);
  return handle;
}

void
handle_update_key (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert(handle);

  if (handle->public_key)
    GNUNET_free(handle->public_key);

  handle->public_key = NULL;

  if (!handle->messenger)
    return;

  const struct GNUNET_IDENTITY_PublicKey *pubkey;
  pubkey = GNUNET_MESSENGER_get_key(handle->messenger);

  if (pubkey)
    handle->public_key = GNUNET_IDENTITY_public_key_to_string(pubkey);
}

void
handle_destroy (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert((handle) &&
		(handle->groups) &&
		(handle->contacts) &&
		(handle->contexts) &&
		(handle->files));

  if (handle->shutdown_hook)
    GNUNET_SCHEDULER_cancel(handle->shutdown_hook);

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->groups, it_destroy_handle_groups, NULL
  );

  GNUNET_CONTAINER_multishortmap_iterate(
      handle->contacts, it_destroy_handle_contacts, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->contexts, it_destroy_handle_contexts, NULL
  );

  if (handle->public_key)
    GNUNET_free(handle->public_key);

  if (handle->messenger)
    GNUNET_MESSENGER_disconnect(handle->messenger);

  if (handle->files)
    GNUNET_FS_stop(handle->fs);

  if (handle->arm)
    GNUNET_ARM_disconnect(handle->arm);

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->files, it_destroy_handle_files, NULL
  );

  GNUNET_CONTAINER_multihashmap_destroy(handle->groups);
  GNUNET_CONTAINER_multishortmap_destroy(handle->contacts);
  GNUNET_CONTAINER_multihashmap_destroy(handle->contexts);
  GNUNET_CONTAINER_multihashmap_destroy(handle->files);

  if (handle->directory)
    GNUNET_free(handle->directory);

  GNUNET_free(handle);
}

void
handle_send_internal_message (struct GNUNET_CHAT_Handle *handle,
			      struct GNUNET_CHAT_Context *context,
			      enum GNUNET_CHAT_MessageFlag flag,
			      const char *warning)
{
  GNUNET_assert((handle) && (GNUNET_CHAT_FLAG_NONE != flag));

  if (!handle->msg_cb)
    return;

  struct GNUNET_CHAT_Message *msg = message_create_internally(
      context, flag, warning
  );

  handle->msg_cb(handle->msg_cls, context, msg);
  message_destroy(msg);
}

void
handle_send_room_name (struct GNUNET_CHAT_Handle *handle,
		       struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert((handle) && (handle->messenger) && (room));

  const char *name = GNUNET_MESSENGER_get_name(handle->messenger);

  if (!name)
    return;

  struct GNUNET_MESSENGER_Message msg;

  msg.header.kind = GNUNET_MESSENGER_KIND_NAME;
  msg.body.name.name = GNUNET_strdup(name);

  GNUNET_MESSENGER_send_message(room, &msg, NULL);

  GNUNET_free(msg.body.name.name);
}

int
handle_request_context_by_room (struct GNUNET_CHAT_Handle *handle,
				struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert((handle) &&
		(handle->contexts) &&
		(room));

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(room);

  struct GNUNET_CHAT_Context *context = GNUNET_CONTAINER_multihashmap_get(
      handle->contexts, key
  );

  struct GNUNET_CHAT_CheckHandleRoomMembers check;

  if ((context) && (context->type == GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN))
    goto check_type;
  else if (context)
    return GNUNET_OK;

  context = context_create_from_room(handle, room);
  context_load_config(context);

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      handle->contexts, key, context,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
  {
    context_destroy(context);
    return GNUNET_SYSERR;
  }

  if (GNUNET_CHAT_CONTEXT_TYPE_GROUP == context->type)
    goto setup_group;

check_type:
  check.ignore_key = GNUNET_MESSENGER_get_key(handle->messenger);
  check.contact = NULL;

  const int checks = GNUNET_MESSENGER_iterate_members(
      room, check_handle_room_members, &check
  );

  if ((check.contact) &&
      (GNUNET_OK == intern_provide_contact_for_member(handle,
						      check.contact,
						      context)))
    context->type = GNUNET_CHAT_CONTEXT_TYPE_CONTACT;
  else if (checks >= 2)
  {
    context->type = GNUNET_CHAT_CONTEXT_TYPE_GROUP;

    if (context->contact)
    {
      struct GNUNET_CHAT_Contact *contact = handle_get_contact_from_messenger(
	  handle, check.contact
      );

      if ((contact) && (contact->context == context))
	contact->context = NULL;

      context->contact = NULL;
    }

    goto setup_group;
  }

  return GNUNET_OK;

setup_group:
  GNUNET_MESSENGER_iterate_members(room, scan_handle_room_members, handle);

  struct GNUNET_CHAT_Group *group = group_create_from_context(
    handle, context
  );

  group_load_config(group);

  if (group->topic)
    group_publish(group);

  if (GNUNET_OK == GNUNET_CONTAINER_multihashmap_put(
    handle->groups, key, group,
    GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
  {
    handle_send_internal_message(
	handle,
	context,
	GNUNET_CHAT_FLAG_UPDATE,
	NULL
    );

    return GNUNET_OK;
  }

  group_destroy(group);

  GNUNET_CONTAINER_multihashmap_remove(handle->contexts, key, context);
  context_destroy(context);
  return GNUNET_SYSERR;
}

struct GNUNET_CHAT_Contact*
handle_get_contact_from_messenger (const struct GNUNET_CHAT_Handle *handle,
				   const struct GNUNET_MESSENGER_Contact *contact)
{
  GNUNET_assert((handle) && (handle->contacts) && (contact));

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(contact, &shorthash);

  return GNUNET_CONTAINER_multishortmap_get(
      handle->contacts, &shorthash
  );
}

struct GNUNET_CHAT_Group*
handle_get_group_from_messenger (const struct GNUNET_CHAT_Handle *handle,
				 const struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert((handle) && (handle->groups) && (room));

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(room);

  if (!key)
    return NULL;

  return GNUNET_CONTAINER_multihashmap_get(
      handle->groups, key
  );
}
