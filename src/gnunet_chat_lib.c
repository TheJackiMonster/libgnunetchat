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
 * @file gnunet_chat_lib.c
 */

#include "gnunet_chat_lib.h"

#include <limits.h>

#include "gnunet_chat_contact.h"
#include "gnunet_chat_context.h"
#include "gnunet_chat_file.h"
#include "gnunet_chat_group.h"
#include "gnunet_chat_handle.h"
#include "gnunet_chat_invitation.h"
#include "gnunet_chat_message.h"
#include "gnunet_chat_util.h"

#include "gnunet_chat_lib_intern.c"

struct GNUNET_CHAT_Handle*
GNUNET_CHAT_start (const struct GNUNET_CONFIGURATION_Handle *cfg,
		   const char *directory,
		   const char *name,
		   GNUNET_CHAT_ContextMessageCallback msg_cb, void *msg_cls)
{
  if (!cfg)
    return NULL;

  return handle_create_from_config(
      cfg, directory, name,
      msg_cb, msg_cls
  );
}


void
GNUNET_CHAT_stop (struct GNUNET_CHAT_Handle *handle)
{
  if (!handle)
    return;

  handle_destroy(handle);
}


int
GNUNET_CHAT_update (struct GNUNET_CHAT_Handle *handle)
{
  if (!handle)
    return GNUNET_SYSERR;

  return GNUNET_MESSENGER_update(handle->messenger);
}


int
GNUNET_CHAT_set_name (struct GNUNET_CHAT_Handle *handle,
		      const char *name)
{
  if (!handle)
    return GNUNET_SYSERR;

  if (!name)
    return GNUNET_NO;

  return GNUNET_MESSENGER_set_name(handle->messenger, name);
}


const char*
GNUNET_CHAT_get_name (const struct GNUNET_CHAT_Handle *handle)
{
  if (!handle)
    return NULL;

  return GNUNET_MESSENGER_get_name(handle->messenger);
}


const char*
GNUNET_CHAT_get_key (const struct GNUNET_CHAT_Handle *handle)
{
  if (!handle)
    return NULL;

  return handle->public_key;
}


void
GNUNET_CHAT_set_user_pointer (struct GNUNET_CHAT_Handle *handle,
			      void *user_pointer)
{
  if (!handle)
    return;

  handle->user_pointer = user_pointer;
}


void*
GNUNET_CHAT_get_user_pointer (const struct GNUNET_CHAT_Handle *handle)
{
  if (!handle)
    return NULL;

  return handle->user_pointer;
}


int
GNUNET_CHAT_iterate_contacts (struct GNUNET_CHAT_Handle *handle,
			      GNUNET_CHAT_ContactCallback callback,
			      void *cls)
{
  if (!handle)
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_HandleIterateContacts it;
  it.handle = handle;
  it.cb = callback;
  it.cls = cls;

  return GNUNET_CONTAINER_multishortmap_iterate(
      handle->contacts, it_handle_iterate_contacts, &it
  );
}


struct GNUNET_CHAT_Group *
GNUNET_CHAT_group_create (struct GNUNET_CHAT_Handle *handle,
			  const char* topic)
{
  if (!handle)
    return NULL;

  struct GNUNET_HashCode key;

  if (topic)
    GNUNET_CRYPTO_hash(topic, strlen(topic), &key);
  else
    GNUNET_CRYPTO_random_block(GNUNET_CRYPTO_QUALITY_WEAK, &key, sizeof(key));

  if (GNUNET_YES == GNUNET_CONTAINER_multihashmap_contains(
      handle->contexts, &key))
    return NULL;

  struct GNUNET_MESSENGER_Room *room = GNUNET_MESSENGER_open_room(
      handle->messenger, &key
  );

  if (!room)
    return NULL;

  struct GNUNET_CHAT_Context *context = context_create_from_room(handle, room);

  handle_send_room_name(handle, room);

  context->type = GNUNET_CHAT_CONTEXT_TYPE_GROUP;

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      handle->contexts, &key, context,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
    goto destroy_context;

  struct GNUNET_CHAT_Group *group = group_create_from_context(handle, context);

  util_set_name_field(topic, &(group->topic));

  if (group->topic)
    group_publish(group);

  if (GNUNET_OK == GNUNET_CONTAINER_multihashmap_put(
      handle->groups, &key, group,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
    return group;

  group_destroy(group);

  GNUNET_CONTAINER_multihashmap_remove(handle->contexts, &key, context);

destroy_context:
  context_destroy(context);
  return NULL;
}


int
GNUNET_CHAT_iterate_groups (struct GNUNET_CHAT_Handle *handle,
			    GNUNET_CHAT_GroupCallback callback,
			    void *cls)
{
  if (!handle)
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_HandleIterateGroups it;
  it.handle = handle;
  it.cb = callback;
  it.cls = cls;

  return GNUNET_CONTAINER_multihashmap_iterate(
      handle->groups, it_handle_iterate_groups, &it
  );
}


int
GNUNET_CHAT_contact_delete (struct GNUNET_CHAT_Contact *contact)
{
  if (!contact)
    return GNUNET_SYSERR;

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(contact->member, &shorthash);

  GNUNET_CONTAINER_multishortmap_remove(
      contact->handle->contacts, &shorthash, contact
  );

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(
      contact->context->room
  );

  GNUNET_CONTAINER_multihashmap_remove(
      contact->handle->contexts, key, contact->context
  );

  GNUNET_MESSENGER_close_room(contact->context->room);

  context_destroy(contact->context);
  contact_destroy(contact);
  return GNUNET_OK;
}


void
GNUNET_CHAT_contact_set_name (struct GNUNET_CHAT_Contact *contact,
			      const char *name)
{
  if ((!contact) || (!(contact->context)))
    return;

  context_update_nick(contact->context, name);
}


const char*
GNUNET_CHAT_contact_get_name (const struct GNUNET_CHAT_Contact *contact)
{
  if (!contact)
    return NULL;

  if ((contact->context) && (contact->context->nick))
    return contact->context->nick;

  return GNUNET_MESSENGER_contact_get_name(contact->member);
}


const char*
GNUNET_CHAT_contact_get_key (const struct GNUNET_CHAT_Contact *contact)
{
  if (!contact)
    return NULL;

  return contact->public_key;
}


struct GNUNET_CHAT_Context*
GNUNET_CHAT_contact_get_context (struct GNUNET_CHAT_Contact *contact)
{
  if (!contact)
    return NULL;

  if (contact->context)
    return contact->context;

  struct GNUNET_CHAT_Context *context = contact_find_context(contact);

  if ((context) && (GNUNET_CHAT_CONTEXT_TYPE_CONTACT == context->type))
    return context;

  context = context_create_from_contact(contact->handle, contact->member);

  if (context)
    contact->context = context;

  return context;
}


void
GNUNET_CHAT_contact_set_user_pointer (struct GNUNET_CHAT_Contact *contact,
				      void *user_pointer)
{
  if (!contact)
    return;

  contact->user_pointer = user_pointer;
}


void*
GNUNET_CHAT_contact_get_user_pointer (const struct GNUNET_CHAT_Contact *contact)
{
  if (!contact)
    return NULL;

  return contact->user_pointer;
}


int
GNUNET_CHAT_contact_is_owned (const struct GNUNET_CHAT_Contact *contact)
{
  if (!contact)
    return GNUNET_SYSERR;

  return contact->is_owned;
}


int
GNUNET_CHAT_group_leave (struct GNUNET_CHAT_Group *group)
{
  if (!group)
    return GNUNET_SYSERR;

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(
      group->context->room
  );

  GNUNET_CONTAINER_multihashmap_remove(
      group->handle->groups, key, group
  );

  GNUNET_CONTAINER_multihashmap_remove(
      group->handle->contexts, key, group->context
  );

  GNUNET_MESSENGER_close_room(group->context->room);

  context_destroy(group->context);
  group_destroy(group);
  return GNUNET_OK;
}


void
GNUNET_CHAT_group_set_name (struct GNUNET_CHAT_Group *group,
			    const char *name)
{
  if (!group)
    return;

  util_set_name_field(name, &(group->context->nick));
}


const char*
GNUNET_CHAT_group_get_name (const struct GNUNET_CHAT_Group *group)
{
  if (!group)
    return NULL;

  return group->context->nick;
}


void
GNUNET_CHAT_group_set_user_pointer (struct GNUNET_CHAT_Group *group,
				    void *user_pointer)
{
  if (!group)
    return;

  group->user_pointer = user_pointer;
}


void*
GNUNET_CHAT_group_get_user_pointer (const struct GNUNET_CHAT_Group *group)
{
  if (!group)
    return NULL;

  return group->user_pointer;
}


void
GNUNET_CHAT_group_invite_contact (struct GNUNET_CHAT_Group *group,
				  struct GNUNET_CHAT_Contact *contact)
{
  if ((!group) || (!contact))
    return;

  struct GNUNET_CHAT_Context *context = contact_find_context(contact);

  if (!context)
    return;

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(
      group->context->room
  );

  handle_send_room_name(group->handle, GNUNET_MESSENGER_open_room(
      group->handle->messenger, key
  ));

  struct GNUNET_MESSENGER_Message msg;
  msg.header.kind = GNUNET_MESSENGER_KIND_INVITE;
  GNUNET_CRYPTO_get_peer_identity(group->handle->cfg, &(msg.body.invite.door));
  GNUNET_memcpy(&(msg.body.invite.key), key, sizeof(msg.body.invite.key));

  GNUNET_MESSENGER_send_message(context->room, &msg, contact->member);
}


int
GNUNET_CHAT_group_iterate_contacts (const struct GNUNET_CHAT_Group *group,
				    GNUNET_CHAT_GroupContactCallback callback,
				    void *cls)
{
  if (!group)
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_GroupIterateContacts it;
  it.group = group;
  it.cb = callback;
  it.cls = cls;

  return GNUNET_MESSENGER_iterate_members(
      group->context->room, it_group_iterate_contacts, &it
  );
}


struct GNUNET_CHAT_Context*
GNUNET_CHAT_group_get_context (struct GNUNET_CHAT_Group *group)
{
  if (!group)
    return NULL;

  return group->context;
}


int
GNUNET_CHAT_context_get_status (const struct GNUNET_CHAT_Context *context)
{
  if ((!context) || (!(context->room)))
    return GNUNET_SYSERR;

  if (1 >= GNUNET_MESSENGER_iterate_members(context->room, NULL, NULL))
    return GNUNET_NO;

  return GNUNET_OK;
}


void
GNUNET_CHAT_context_request (struct GNUNET_CHAT_Context *context)
{
  if ((!context) || (context->room))
    return;

  struct GNUNET_CHAT_Handle *handle = context->handle;

  if ((!handle) || (!(context->contact)))
    return;

  struct GNUNET_CHAT_Contact *contact = contact_create_from_member(
      handle, context->contact
  );

  if (!contact)
    return;

  context->type = GNUNET_CHAT_CONTEXT_TYPE_CONTACT;

  struct GNUNET_CHAT_Context *other = contact_find_context(contact);

  if ((!other) || (!(other->room)))
    return;

  struct GNUNET_HashCode key;
  GNUNET_CRYPTO_random_block(GNUNET_CRYPTO_QUALITY_WEAK, &key, sizeof(key));

  if (GNUNET_YES == GNUNET_CONTAINER_multihashmap_contains(
      handle->contexts, &key))
    return;

  struct GNUNET_MESSENGER_Room *room = GNUNET_MESSENGER_open_room(
      handle->messenger, &key
  );

  if (!room)
    return;

  context_update_room(context, room);
  handle_send_room_name(handle, room);

  struct GNUNET_MESSENGER_Message msg;
  msg.header.kind = GNUNET_MESSENGER_KIND_INVITE;
  GNUNET_CRYPTO_get_peer_identity(handle->cfg, &(msg.body.invite.door));
  GNUNET_memcpy(&(msg.body.invite.key), &key, sizeof(msg.body.invite.key));

  GNUNET_MESSENGER_send_message(other->room, &msg, context->contact);
}


const struct GNUNET_CHAT_Contact*
GNUNET_CHAT_context_get_contact (const struct GNUNET_CHAT_Context *context)
{
  if ((!context) || (GNUNET_CHAT_CONTEXT_TYPE_CONTACT != context->type))
    return NULL;

  if (context->contact)
    return handle_get_contact_from_messenger(context->handle, context->contact);

  struct GNUNET_MESSENGER_Room *room = context->room;
  struct GNUNET_CHAT_RoomFindContact find;

  find.ignore_key = GNUNET_MESSENGER_get_key(context->handle->messenger);
  find.contact = NULL;

  int member_count = GNUNET_MESSENGER_iterate_members(
      room,
      it_room_find_contact,
      &find
  );

  if ((!find.contact) || (member_count > 2))
    return NULL;

  return handle_get_contact_from_messenger(context->handle, find.contact);
}


const struct GNUNET_CHAT_Group*
GNUNET_CHAT_context_get_group (const struct GNUNET_CHAT_Context *context)
{
  if ((!context) || (GNUNET_CHAT_CONTEXT_TYPE_GROUP != context->type))
    return NULL;

  if (!context->room)
    return NULL;

  return handle_get_group_from_messenger(context->handle, context->room);
}


void
GNUNET_CHAT_context_set_user_pointer (struct GNUNET_CHAT_Context *context,
				      void *user_pointer)
{
  if (!context)
    return;

  context->user_pointer = user_pointer;
}


void*
GNUNET_CHAT_context_get_user_pointer (const struct GNUNET_CHAT_Context *context)
{
  if (!context)
    return NULL;

  return context->user_pointer;
}


int
GNUNET_CHAT_context_send_text (struct GNUNET_CHAT_Context *context,
			       const char *text)
{
  if ((!context) || (!text) || (!(context->room)))
    return GNUNET_SYSERR;

  struct GNUNET_MESSENGER_Message msg;
  msg.header.kind = GNUNET_MESSENGER_KIND_TEXT;
  msg.body.text.text = GNUNET_strdup(text);

  GNUNET_MESSENGER_send_message(context->room, &msg, NULL);

  GNUNET_free(msg.body.text.text);
  return GNUNET_OK;
}


int
GNUNET_CHAT_context_send_file (struct GNUNET_CHAT_Context *context,
			       const char *path,
			       GNUNET_CHAT_FileUploadCallback callback,
			       void *cls)
{
  if ((!context) || (!path) || (!(context->room)))
    return GNUNET_SYSERR;

  if (!(context->handle->directory))
    return GNUNET_SYSERR;

  struct GNUNET_HashCode hash;
  if (GNUNET_OK != util_hash_file(path, &hash))
    return GNUNET_SYSERR;

  char *filename;
  util_get_filename (
      context->handle->directory, "files", &hash, &filename
  );

  if ((GNUNET_OK != GNUNET_DISK_directory_create_for_file(filename)) ||
      (GNUNET_OK != GNUNET_DISK_file_copy(path, filename)))
  {
    GNUNET_free(filename);
    return GNUNET_SYSERR;
  }

  struct GNUNET_CRYPTO_SymmetricSessionKey key;
  GNUNET_CRYPTO_symmetric_create_session_key(&key);

  if (GNUNET_OK != util_encrypt_file(filename, &key))
  {
    GNUNET_free(filename);
    return GNUNET_SYSERR;
  }

  char* p = GNUNET_strdup(path);

  struct GNUNET_CHAT_File *file = file_create_from_disk(
      context->handle, basename(p), &hash, &key
  );

  GNUNET_free(p);

  file_bind_upload(file, callback, cls);

  struct GNUNET_FS_BlockOptions bo;

  bo.anonymity_level = 1;
  bo.content_priority = 100;
  bo.replication_level = 1;

  bo.expiration_time = GNUNET_TIME_absolute_add(
      GNUNET_TIME_absolute_get(), GNUNET_TIME_relative_get_hour_()
  );

  struct GNUNET_FS_FileInformation* fi = GNUNET_FS_file_information_create_from_file(
      context->handle->fs,
      file,
      filename,
      NULL,
      file->meta,
      GNUNET_YES,
      &bo
  );

  file->publish = GNUNET_FS_publish_start(
      context->handle->fs, fi,
      NULL, NULL, NULL,
      GNUNET_FS_PUBLISH_OPTION_NONE
  );

  GNUNET_free(filename);
  return GNUNET_OK;
}


int
GNUNET_CHAT_context_share_file (struct GNUNET_CHAT_Context *context,
				const struct GNUNET_CHAT_File *file)
{
  if ((!context) || (!file) || (strlen(file->name) > NAME_MAX) ||
      (!(context->room)))
    return GNUNET_SYSERR;

  struct GNUNET_MESSENGER_Message msg;
  msg.header.kind = GNUNET_MESSENGER_KIND_FILE;
  GNUNET_memcpy(&(msg.body.file.key), &(file->key), sizeof(file->key));
  GNUNET_memcpy(&(msg.body.file.hash), &(file->hash), sizeof(file->hash));
  GNUNET_strlcpy(msg.body.file.name, file->name, NAME_MAX);
  msg.body.file.uri = GNUNET_FS_uri_to_string(file->uri);

  GNUNET_MESSENGER_send_message(context->room, &msg, NULL);

  GNUNET_free(msg.body.file.uri);
  return GNUNET_OK;
}


int
GNUNET_CHAT_context_iterate_messages (struct GNUNET_CHAT_Context *context,
				      GNUNET_CHAT_ContextMessageCallback callback,
				      void *cls)
{
  if (!context)
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_ContextIterateMessages it;
  it.context = context;
  it.cb = callback;
  it.cls = cls;

  return GNUNET_CONTAINER_multihashmap_iterate(
      context->messages, it_context_iterate_messages, &it
  );
}


int
GNUNET_CHAT_context_iterate_files (struct GNUNET_CHAT_Context *context,
				   GNUNET_CHAT_ContextFileCallback callback,
				   void *cls)
{
  if (!context)
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_ContextIterateFiles it;
  it.context = context;
  it.cb = callback;
  it.cls = cls;

  return GNUNET_CONTAINER_multihashmap_iterate(
      context->files, it_context_iterate_files, &it
  );
}


enum GNUNET_CHAT_MessageKind
GNUNET_CHAT_message_get_kind (const struct GNUNET_CHAT_Message *message)
{
  if (!message)
    return GNUNET_CHAT_KIND_UNKNOWN;

  switch (message->flag)
  {
    case GNUNET_CHAT_FLAG_WARNING:
      return GNUNET_CHAT_KIND_WARNING;
    case GNUNET_CHAT_FLAG_LOGIN:
      return GNUNET_CHAT_KIND_LOGIN;
    case GNUNET_CHAT_FLAG_UPDATE:
      return GNUNET_CHAT_KIND_UPDATE;
    default:
      break;
  }

  if (!(message->msg))
    return GNUNET_CHAT_KIND_UNKNOWN;

  switch (message->msg->header.kind)
  {
    case GNUNET_MESSENGER_KIND_JOIN:
      return GNUNET_CHAT_KIND_JOIN;
    case GNUNET_MESSENGER_KIND_LEAVE:
      return GNUNET_CHAT_KIND_LEAVE;
    case GNUNET_MESSENGER_KIND_NAME:
    case GNUNET_MESSENGER_KIND_KEY:
    case GNUNET_MESSENGER_KIND_ID:
      return GNUNET_CHAT_KIND_CONTACT;
    case GNUNET_MESSENGER_KIND_INVITE:
      return GNUNET_CHAT_KIND_INVITATION;
    case GNUNET_MESSENGER_KIND_TEXT:
      return GNUNET_CHAT_KIND_TEXT;
    case GNUNET_MESSENGER_KIND_FILE:
      return GNUNET_CHAT_KIND_FILE;
    case GNUNET_MESSENGER_KIND_DELETE:
      return GNUNET_CHAT_KIND_DELETION;
    default:
      return GNUNET_CHAT_KIND_UNKNOWN;
  }
}


struct GNUNET_TIME_Absolute
GNUNET_CHAT_message_get_timestamp (const struct GNUNET_CHAT_Message *message)
{
  if ((!message) || (!(message->msg)))
    return GNUNET_TIME_absolute_get_zero_();

  return GNUNET_TIME_absolute_ntoh(message->msg->header.timestamp);
}


struct GNUNET_CHAT_Contact*
GNUNET_CHAT_message_get_sender (const struct GNUNET_CHAT_Message *message)
{
  if ((!message) || (GNUNET_CHAT_FLAG_NONE != message->flag) ||
      (!message->context) || (!message->context->room))
    return NULL;

  const struct GNUNET_MESSENGER_Contact *sender = GNUNET_MESSENGER_get_sender(
      message->context->room, &(message->hash)
  );

  if (!sender)
    return NULL;

  return handle_get_contact_from_messenger(message->context->handle, sender);
}


int
GNUNET_CHAT_message_is_sent (const struct GNUNET_CHAT_Message *message)
{
  if (!message)
    return GNUNET_SYSERR;

  if (message->flags & GNUNET_MESSENGER_FLAG_SENT)
    return GNUNET_YES;
  else
    return GNUNET_NO;
}


int
GNUNET_CHAT_message_is_private (const struct GNUNET_CHAT_Message *message)
{
  if (!message)
    return GNUNET_SYSERR;

  if (message->flags & GNUNET_MESSENGER_FLAG_PRIVATE)
    return GNUNET_YES;
  else
    return GNUNET_NO;
}


int
GNUNET_CHAT_message_get_read_receipt (const struct GNUNET_CHAT_Message *message,
				      GNUNET_CHAT_MessageReadReceiptCallback callback,
				      void *cls)
{
  if ((!message) || (!(message->msg)) || (!(message->context)))
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_MessageIterateReadReceipts it;
  it.message = message;
  it.cb = callback;
  it.cls = cls;

  return GNUNET_MESSENGER_iterate_members(
      message->context->room, it_message_iterate_read_receipts, &it
  );
}


const char*
GNUNET_CHAT_message_get_text (const struct GNUNET_CHAT_Message *message)
{
  if ((!message) || (!(message->msg)))
    return NULL;

  if (GNUNET_CHAT_FLAG_WARNING == message->flag)
    return message->warning;
  if (GNUNET_MESSENGER_KIND_TEXT != message->msg->header.kind)
    return NULL;

  return message->msg->body.text.text;
}


struct GNUNET_CHAT_File*
GNUNET_CHAT_message_get_file (const struct GNUNET_CHAT_Message *message)
{
  if ((!message) || (!(message->msg)) || (!(message->context)))
    return NULL;

  if (GNUNET_MESSENGER_KIND_FILE != message->msg->header.kind)
    return NULL;

  return GNUNET_CONTAINER_multihashmap_get(
      message->context->handle->files,
      &(message->msg->body.file.hash)
  );
}


struct GNUNET_CHAT_Invitation*
GNUNET_CHAT_message_get_invitation (const struct GNUNET_CHAT_Message *message)
{
  if ((!message) || (!(message->msg)) || (!(message->context)))
    return NULL;

  if (GNUNET_MESSENGER_KIND_INVITE != message->msg->header.kind)
    return NULL;

  return GNUNET_CONTAINER_multihashmap_get(
      message->context->invites,
      &(message->hash)
  );
}


int
GNUNET_CHAT_message_delete (const struct GNUNET_CHAT_Message *message,
			    struct GNUNET_TIME_Relative delay)
{
  if ((!message) || (!(message->msg)) || (!(message->context)))
    return GNUNET_SYSERR;

  struct GNUNET_MESSENGER_Message msg;
  msg.header.kind = GNUNET_MESSENGER_KIND_DELETE;
  GNUNET_memcpy(&(msg.body.deletion.hash), &(message->hash), sizeof(message->hash));
  msg.body.deletion.delay = GNUNET_TIME_relative_hton(delay);

  GNUNET_MESSENGER_send_message(message->context->room, &msg, NULL);
  return GNUNET_OK;
}


const struct GNUNET_HashCode*
GNUNET_CHAT_file_get_hash (const struct GNUNET_CHAT_File *file)
{
  if (!file)
    return NULL;

  return &(file->hash);
}


uint64_t
GNUNET_CHAT_file_get_size (const struct GNUNET_CHAT_File *file)
{
  if (!file)
    return 0;

  if (file->uri)
    return GNUNET_FS_uri_chk_get_file_size(file->uri);

  char *filename;
  util_get_filename (
      file->handle->directory, "files", &(file->hash), &filename
  );

  uint64_t size;
  if (GNUNET_OK != GNUNET_DISK_file_size(filename, &size, GNUNET_NO, GNUNET_YES))
    size = 0;

  GNUNET_free(filename);
  return size;
}


int
GNUNET_CHAT_file_is_local (const struct GNUNET_CHAT_File *file)
{
  if (!file)
    return GNUNET_SYSERR;

  char *filename;
  util_get_filename (
      file->handle->directory, "files", &(file->hash), &filename
  );

  int result = GNUNET_DISK_file_test(filename);

  GNUNET_free(filename);
  return result;
}


void
GNUNET_CHAT_file_set_user_pointer (struct GNUNET_CHAT_File *file,
				   void *user_pointer)
{
  if (!file)
    return;

  file->user_pointer = user_pointer;
}


void*
GNUNET_CHAT_file_get_user_pointer (const struct GNUNET_CHAT_File *file)
{
  if (!file)
    return NULL;

  return file->user_pointer;
}


int
GNUNET_CHAT_file_start_download (struct GNUNET_CHAT_File *file,
				 GNUNET_CHAT_FileDownloadCallback callback,
				 void *cls)
{
  if ((!file) || (!(file->uri)))
    return GNUNET_SYSERR;

  if (file->download)
  {
    file_bind_downlaod(file, callback, cls);

    GNUNET_FS_download_resume(file->download);
    return GNUNET_OK;
  }

  const uint64_t size = GNUNET_FS_uri_chk_get_file_size(file->uri);

  char *filename;
  util_get_filename (
      file->handle->directory, "files", &(file->hash), &filename
  );

  uint64_t offset;
  if (GNUNET_OK != GNUNET_DISK_file_size(filename, &offset, GNUNET_NO, GNUNET_YES))
    offset = 0;

  if (offset >= size)
  {
    if (callback)
      callback(cls, file, size, size);

    return GNUNET_OK;
  }

  file_bind_downlaod(file, callback, cls);

  const uint64_t remaining = (size - offset);

  file->download = GNUNET_FS_download_start(
      file->handle->fs,
      file->uri,
      file->meta,
      filename,
      NULL,
      offset,
      remaining,
      1,
      GNUNET_FS_DOWNLOAD_OPTION_NONE,
      file,
      NULL
  );

  GNUNET_free(filename);
  return GNUNET_OK;
}


int
GNUNET_CHAT_file_pause_download (struct GNUNET_CHAT_File *file)
{
  if (!file)
    return GNUNET_SYSERR;

  GNUNET_FS_download_suspend(file->download);
  return GNUNET_OK;
}


int
GNUNET_CHAT_file_resume_download (struct GNUNET_CHAT_File *file)
{
  if (!file)
    return GNUNET_SYSERR;

  GNUNET_FS_download_resume(file->download);
  return GNUNET_OK;
}


int
GNUNET_CHAT_file_stop_download (struct GNUNET_CHAT_File *file)
{
  if (!file)
    return GNUNET_SYSERR;

  GNUNET_FS_download_stop(file->download, GNUNET_YES);
  file->download = NULL;
  return GNUNET_OK;
}


int
GNUNET_CHAT_file_unindex (struct GNUNET_CHAT_File *file,
			  GNUNET_CHAT_FileUnindexCallback callback,
			  void *cls)
{
  if (!file)
    return GNUNET_SYSERR;

  if (file->publish)
  {
    GNUNET_FS_publish_stop(file->publish);
    file->publish = NULL;
    return GNUNET_OK;
  }

  file_bind_unindex(file, callback, cls);

  if (file->unindex)
    return GNUNET_OK;

  char *filename;
  util_get_filename (
      file->handle->directory, "files", &(file->hash), &filename
  );

  file->unindex = GNUNET_FS_unindex_start(
      file->handle->fs, filename, file
  );

  GNUNET_free(filename);
  return GNUNET_OK;
}


void
GNUNET_CHAT_invitation_accept (struct GNUNET_CHAT_Invitation *invitation)
{
  if (!invitation)
    return;

  struct GNUNET_PeerIdentity door;
  GNUNET_PEER_resolve(invitation->door, &door);

  GNUNET_MESSENGER_enter_room(
      invitation->context->handle->messenger,
      &door, &(invitation->key)
  );
}

