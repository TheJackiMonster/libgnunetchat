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
 * @file gnunet_chat_handle_intern.c
 */

#include "gnunet_chat_contact.h"
#include "gnunet_chat_context.h"
#include "gnunet_chat_file.h"
#include "gnunet_chat_group.h"
#include "gnunet_chat_handle.h"
#include "gnunet_chat_invitation.h"
#include "gnunet_chat_message.h"
#include "gnunet_chat_util.h"

#define GNUNET_UNUSED __attribute__ ((unused))

void
on_handle_shutdown(void *cls)
{
  struct GNUNET_CHAT_Handle *chat = cls;

  GNUNET_assert((chat) && (chat->shutdown_hook));
  chat->shutdown_hook = NULL;

  handle_destroy(chat);
}

void
on_handle_arm_connection(void *cls,
			 int connected)
{
  struct GNUNET_CHAT_Handle *chat = cls;

  GNUNET_assert((chat) && (chat->arm));

  if (GNUNET_YES == connected) {
    GNUNET_ARM_request_service_start(
	chat->arm, "messenger",
	GNUNET_OS_INHERIT_STD_NONE,
	NULL, NULL
    );

    GNUNET_ARM_request_service_start(
	chat->arm, "fs",
	GNUNET_OS_INHERIT_STD_NONE,
	NULL, NULL
    );
  } else {
    GNUNET_ARM_request_service_start(
	chat->arm, "arm",
	GNUNET_OS_INHERIT_STD_NONE,
	NULL, NULL
    );
  }
}

void*
notify_handle_fs_progress(void* cls, const struct GNUNET_FS_ProgressInfo* info)
{
  struct GNUNET_CHAT_Handle *chat = cls;

  GNUNET_assert(info);

  if (!chat)
    return NULL;

  switch (info->status) {
    case GNUNET_FS_STATUS_PUBLISH_START: {
      struct GNUNET_CHAT_File *file = info->value.publish.cctx;

      file_update_upload(
	  file,
	  0,
	  info->value.publish.size
      );

      return file;
    } case GNUNET_FS_STATUS_PUBLISH_PROGRESS: {
      struct GNUNET_CHAT_File *file = info->value.publish.cctx;

      file_update_upload(
	  file,
	  info->value.publish.completed,
	  info->value.publish.size
      );

      return file;
    } case GNUNET_FS_STATUS_PUBLISH_COMPLETED: {
      struct GNUNET_CHAT_File *file = info->value.publish.cctx;

      file->uri = GNUNET_FS_uri_dup(
	  info->value.publish.specifics.completed.chk_uri
      );

      file_update_upload(
      	  file,
	  info->value.publish.size,
      	  info->value.publish.size
      );

      file->publish = NULL;
      break;
    } case GNUNET_FS_STATUS_PUBLISH_ERROR: {
      break;
    } case GNUNET_FS_STATUS_DOWNLOAD_START: {
      struct GNUNET_CHAT_File *file = info->value.download.cctx;

      file_update_download(
	  file,
      	  0,
      	  info->value.download.size
      );

      return file;
    } case GNUNET_FS_STATUS_DOWNLOAD_ACTIVE: {
      return info->value.download.cctx;
    } case GNUNET_FS_STATUS_DOWNLOAD_INACTIVE: {
      return info->value.download.cctx;
    } case GNUNET_FS_STATUS_DOWNLOAD_PROGRESS: {
      struct GNUNET_CHAT_File *file = info->value.download.cctx;

      file_update_download(
      	  file,
	  info->value.download.completed,
	  info->value.download.size
      );

      return file;
    } case GNUNET_FS_STATUS_DOWNLOAD_COMPLETED: {
      struct GNUNET_CHAT_File *file = info->value.download.cctx;

      file_update_download(
	  file,
	  info->value.download.size,
	  info->value.download.size
      );

      file->download = NULL;
      break;
    } case GNUNET_FS_STATUS_DOWNLOAD_ERROR: {
      break;
    } case GNUNET_FS_STATUS_UNINDEX_START: {
      struct GNUNET_CHAT_File *file = info->value.unindex.cctx;

      file_update_unindex(
	  file,
      	  0,
      	  info->value.unindex.size
      );

      return file;
    } case GNUNET_FS_STATUS_UNINDEX_PROGRESS: {
      struct GNUNET_CHAT_File *file = info->value.unindex.cctx;

      file_update_unindex(
      	  file,
	  info->value.unindex.completed,
	  info->value.unindex.size
      );

      return file;
    } case GNUNET_FS_STATUS_UNINDEX_COMPLETED: {
      struct GNUNET_CHAT_File *file = info->value.unindex.cctx;

      file_update_unindex(
	  file,
      	  info->value.unindex.size,
      	  info->value.unindex.size
      );

      file->unindex = NULL;
      break;
    } default: {
      break;
    }
  }

  return NULL;
}

int
intern_provide_contact_for_member(struct GNUNET_CHAT_Handle *handle,
				  const struct GNUNET_MESSENGER_Contact *member,
				  struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((handle) && (handle->contacts));

  if (!member)
    return GNUNET_OK;

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(member, &shorthash);

  struct GNUNET_CHAT_Contact *contact = GNUNET_CONTAINER_multishortmap_get(
      handle->contacts, &shorthash
  );

  if (contact)
  {
    if ((context) && (NULL == contact->context))
      contact->context = context;

    return GNUNET_OK;
  }

  contact = contact_create_from_member(
    handle, member
  );

  if (context)
    contact->context = context;

  if (GNUNET_OK == GNUNET_CONTAINER_multishortmap_put(
      handle->contacts, &shorthash, contact,
    GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
    return GNUNET_OK;

  contact_destroy(contact);
  return GNUNET_SYSERR;
}

struct GNUNET_CHAT_CheckHandleRoomMembers
{
  const struct GNUNET_IDENTITY_PublicKey *ignore_key;
  const struct GNUNET_MESSENGER_Contact *contact;
};

int
check_handle_room_members (void* cls,
			   GNUNET_UNUSED struct GNUNET_MESSENGER_Room *room,
                           const struct GNUNET_MESSENGER_Contact *member)
{
  struct GNUNET_CHAT_CheckHandleRoomMembers *check = cls;

  GNUNET_assert((check) && (member));

  const struct GNUNET_IDENTITY_PublicKey *member_key = (
      GNUNET_MESSENGER_contact_get_key(member)
  );

  if (0 == GNUNET_memcmp(member_key, check->ignore_key))
    return GNUNET_YES;

  if (check->contact)
  {
    check->contact = NULL;
    return GNUNET_NO;
  }

  check->contact = member;
  return GNUNET_YES;
}

int
request_handle_context_by_room (struct GNUNET_CHAT_Handle *handle,
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

  if ((context) && (GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN == context->type))
    goto check_context_type;
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

check_context_type:
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

    struct GNUNET_CHAT_Group *group = group_create_from_context(
      handle, context
    );

    group_load_config(group);

    if (group->topic)
      group_publish(group);

    if (GNUNET_OK == GNUNET_CONTAINER_multihashmap_put(
      handle->groups, key, group,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
      return GNUNET_OK;

    group_destroy(group);

    GNUNET_CONTAINER_multihashmap_remove(handle->contexts, key, context);
    context_destroy(context);
    return GNUNET_SYSERR;
  }

  return GNUNET_OK;
}

int
find_handle_rooms (void *cls,
		   struct GNUNET_MESSENGER_Room *room,
		   GNUNET_UNUSED const struct GNUNET_MESSENGER_Contact *member)
{
  struct GNUNET_CHAT_Handle *handle = cls;

  GNUNET_assert((handle) && (room));

  if (GNUNET_OK != request_handle_context_by_room(handle, room))
    return GNUNET_NO;

  return GNUNET_YES;
}

int
scan_handle_room_members (void* cls,
			  GNUNET_UNUSED struct GNUNET_MESSENGER_Room *room,
                          const struct GNUNET_MESSENGER_Contact *member)
{
  struct GNUNET_CHAT_Handle *handle = cls;

  if (GNUNET_OK == intern_provide_contact_for_member(handle, member, NULL))
    return GNUNET_YES;
  else
    return GNUNET_NO;
}

int
scan_handle_rooms (void *cls,
		   struct GNUNET_MESSENGER_Room *room,
		   GNUNET_UNUSED const struct GNUNET_MESSENGER_Contact *member)
{
  struct GNUNET_CHAT_Handle *handle = cls;

  GNUNET_assert((handle) &&
		(handle->groups) &&
		(room));

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(room);

  struct GNUNET_CHAT_Group *group = GNUNET_CONTAINER_multihashmap_get(
      handle->groups, key
  );

  if (!group)
    return GNUNET_YES;

  GNUNET_MESSENGER_iterate_members(room, scan_handle_room_members, handle);
  return GNUNET_YES;
}

void
on_handle_identity(void *cls,
		   GNUNET_UNUSED struct GNUNET_MESSENGER_Handle *messenger)
{
  struct GNUNET_CHAT_Handle *handle = cls;

  GNUNET_assert((handle) &&
		(handle->contexts) &&
		(handle->groups) &&
		(handle->contacts));

  handle_update_key(handle);

  if ((0 < GNUNET_CONTAINER_multihashmap_size(handle->contexts)) ||
      (0 < GNUNET_CONTAINER_multihashmap_size(handle->groups)) ||
      (0 < GNUNET_CONTAINER_multishortmap_size(handle->contacts)))
    return;

  GNUNET_assert(handle->messenger);

  GNUNET_MESSENGER_find_rooms(
      handle->messenger, NULL, find_handle_rooms, handle
  );

  GNUNET_MESSENGER_find_rooms(
      handle->messenger, NULL, scan_handle_rooms, handle
  );

  if (!handle->msg_cb)
    return;

  struct GNUNET_CHAT_Message *msg = message_create_internally(
      NULL, GNUNET_CHAT_FLAG_LOGIN, NULL
  );

  handle->msg_cb(handle->msg_cls, NULL, msg);
  message_destroy(msg);
}

void
on_handle_message (void *cls,
		   struct GNUNET_MESSENGER_Room *room,
		   const struct GNUNET_MESSENGER_Contact *sender,
		   const struct GNUNET_MESSENGER_Message *msg,
		   const struct GNUNET_HashCode *hash,
		   enum GNUNET_MESSENGER_MessageFlags flags)
{
  struct GNUNET_CHAT_Handle *handle = cls;

  GNUNET_assert((handle) &&
		(room) &&
		(msg) &&
		(hash));

  if ((GNUNET_OK != request_handle_context_by_room(handle, room)) ||
      (GNUNET_OK != intern_provide_contact_for_member(handle, sender, NULL)))
    return;

  struct GNUNET_CHAT_Context *context = GNUNET_CONTAINER_multihashmap_get(
      handle->contexts, GNUNET_MESSENGER_room_get_key(room)
  );

  const struct GNUNET_TIME_Absolute timestamp = GNUNET_TIME_absolute_ntoh(
      msg->header.timestamp
  );

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(sender, &shorthash);

  struct GNUNET_TIME_Absolute *time = GNUNET_CONTAINER_multishortmap_get(
      context->timestamps, &shorthash
  );

  if (!time)
  {
    time = GNUNET_new(struct GNUNET_TIME_Absolute);
    *time = timestamp;

    if (GNUNET_OK != GNUNET_CONTAINER_multishortmap_put(
	context->timestamps, &shorthash, time,
	GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
      GNUNET_free(time);
  }
  else
  {
    struct GNUNET_TIME_Relative delta = GNUNET_TIME_absolute_get_difference(
	timestamp, *time
    );

    if (GNUNET_TIME_relative_get_zero_().rel_value_us == delta.rel_value_us)
      *time = timestamp;
  }

  struct GNUNET_CHAT_Message *message = GNUNET_CONTAINER_multihashmap_get(
      context->messages, hash
  );

  if (message)
    return;

  message = message_create_from_msg(context, hash, flags, msg);

  switch (msg->header.kind)
  {
    case GNUNET_MESSENGER_KIND_KEY:
    {
      struct GNUNET_CHAT_Contact *contact = GNUNET_CONTAINER_multishortmap_get(
	  handle->contacts, &shorthash
      );

      contact_update_key(contact);
      break;
    }
    case GNUNET_MESSENGER_KIND_INVITE:
    {
      struct GNUNET_CHAT_Invitation *invitation = invitation_create_from_message(
	  context, &(msg->body.invite)
      );

      if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
	  context->invites, hash, invitation,
	  GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
	invitation_destroy(invitation);
      break;
    }
    case GNUNET_MESSENGER_KIND_FILE:
    {
      GNUNET_CONTAINER_multihashmap_put(
	    context->files, hash, NULL,
	    GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST
      );

      struct GNUNET_CHAT_File *file = file_create_from_message(
	  context->handle, &(msg->body.file)
      );

      if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
	  context->handle->files, &(file->hash), file,
	  GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
	file_destroy(file);
      break;
    }
    case GNUNET_MESSENGER_KIND_DELETE:
    {
      struct GNUNET_CHAT_Message *target = GNUNET_CONTAINER_multihashmap_get(
	  context->messages, &(msg->body.deletion.hash)
      );

      if (target)
	target->msg = NULL;
      break;
    }
    default:
      break;
  }

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      context->messages, hash, message,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
  {
    message_destroy(message);
    return;
  }

  if (handle->msg_cb)
    handle->msg_cb(handle->msg_cls, context, message);
}

int
it_destroy_handle_groups (GNUNET_UNUSED void *cls,
			  GNUNET_UNUSED const struct GNUNET_HashCode *key,
			  void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_Group *group = value;
  group_save_config(group);
  group_destroy(group);
  return GNUNET_YES;
}

int
it_destroy_handle_contacts (GNUNET_UNUSED void *cls,
			    GNUNET_UNUSED const struct GNUNET_ShortHashCode *key,
			    void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_Contact *contact = value;
  contact_destroy(contact);
  return GNUNET_YES;
}

int
it_destroy_handle_contexts (GNUNET_UNUSED void *cls,
			    GNUNET_UNUSED const struct GNUNET_HashCode *key,
			    void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_Context *context = value;
  context_save_config(context);
  context_destroy(context);
  return GNUNET_YES;
}

int
it_destroy_handle_files (GNUNET_UNUSED void *cls,
			 GNUNET_UNUSED const struct GNUNET_HashCode *key,
			 void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_File *file = value;
  file_destroy(file);
  return GNUNET_YES;
}
