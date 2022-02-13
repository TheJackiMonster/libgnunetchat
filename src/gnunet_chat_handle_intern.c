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
notify_handle_fs_progress(void* cls,
			  const struct GNUNET_FS_ProgressInfo* info)
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

void
on_handle_gnunet_identity(void *cls,
			  struct GNUNET_IDENTITY_Ego *ego,
			  GNUNET_UNUSED void **ctx,
                          const char *name)
{
  struct GNUNET_CHAT_Handle* handle = cls;

  if (!name)
    return;

  struct GNUNET_CHAT_InternalIdentities *identities = GNUNET_new(
      struct GNUNET_CHAT_InternalIdentities
  );

  identities->name = GNUNET_strdup(name);
  identities->ego = ego;

  GNUNET_CONTAINER_DLL_insert_tail(
      handle->identities_head,
      handle->identities_tail,
      identities
  );
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
    {
      contact->context = context;
      context->contact = member;
    }

    return GNUNET_OK;
  }

  contact = contact_create_from_member(
    handle, member
  );

  if (context)
  {
    contact->context = context;
    context->contact = member;
  }

  if (GNUNET_OK == GNUNET_CONTAINER_multishortmap_put(
      handle->contacts, &shorthash, contact,
    GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
    return GNUNET_OK;

  if (context)
    context->contact = NULL;

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

  if ((member_key) && (check->ignore_key) &&
      (0 == GNUNET_memcmp(member_key, check->ignore_key)))
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
  context_scan_configs(handle);

  handle_send_internal_message(
      handle,
      NULL,
      GNUNET_CHAT_FLAG_LOGIN,
      NULL
  );
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

  if ((GNUNET_OK != handle_request_context_by_room(handle, room)) ||
      (GNUNET_OK != intern_provide_contact_for_member(handle, sender, NULL)))
    return;

  GNUNET_MESSENGER_get_message(room, &(msg->header.previous));

  struct GNUNET_CHAT_Context *context = GNUNET_CONTAINER_multihashmap_get(
      handle->contexts, GNUNET_MESSENGER_room_get_key(room)
  );

  const struct GNUNET_TIME_Absolute timestamp = GNUNET_TIME_absolute_ntoh(
      msg->header.timestamp
  );

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(sender, &shorthash);

  struct GNUNET_CHAT_Contact *contact = GNUNET_CONTAINER_multishortmap_get(
      handle->contacts, &shorthash
  );

  if (flags & GNUNET_MESSENGER_FLAG_SENT)
    contact->is_owned = GNUNET_YES;

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

      struct GNUNET_CHAT_File *file = GNUNET_CONTAINER_multihashmap_get(
	  context->handle->files, &(msg->body.file.hash)
      );

      if (file)
	break;

      file = file_create_from_message(
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
