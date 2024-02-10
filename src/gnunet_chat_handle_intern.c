/*
   This file is part of GNUnet.
   Copyright (C) 2021--2024 GNUnet e.V.

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
#include "gnunet_chat_ticket.h"
#include "gnunet_chat_util.h"

#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_reclaim_service.h>
#include <gnunet/gnunet_scheduler_lib.h>
#include <gnunet/gnunet_util_lib.h>
#include <stdio.h>
#include <string.h>

#define GNUNET_UNUSED __attribute__ ((unused))

static const char gnunet_service_name_arm [] = "arm";
static const char gnunet_service_name_fs [] = "fs";
static const char gnunet_service_name_gns [] = "gns";
static const char gnunet_service_name_identity [] = "identity";
static const char gnunet_service_name_messenger [] = "messenger";
static const char gnunet_service_name_namestore [] = "namestore";
static const char gnunet_service_name_reclaim [] = "reclaim";

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
    	chat->arm, gnunet_service_name_identity,
    	GNUNET_OS_INHERIT_STD_NONE,
    	NULL, NULL
    );

    GNUNET_ARM_request_service_start(
      chat->arm, gnunet_service_name_messenger,
      GNUNET_OS_INHERIT_STD_NONE,
      NULL, NULL
    );

    GNUNET_ARM_request_service_start(
      chat->arm, gnunet_service_name_fs,
      GNUNET_OS_INHERIT_STD_NONE,
      NULL, NULL
    );

    GNUNET_ARM_request_service_start(
    	chat->arm, gnunet_service_name_gns,
    	GNUNET_OS_INHERIT_STD_NONE,
    	NULL, NULL
    );

    GNUNET_ARM_request_service_start(
    	chat->arm, gnunet_service_name_namestore,
    	GNUNET_OS_INHERIT_STD_NONE,
    	NULL, NULL
    );

    GNUNET_ARM_request_service_start(
    	chat->arm, gnunet_service_name_reclaim,
    	GNUNET_OS_INHERIT_STD_NONE,
    	NULL, NULL
    );
  } else {
    GNUNET_ARM_request_service_start(
      chat->arm, gnunet_service_name_arm,
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
      const char *directory = handle_get_directory(chat);

      if (!directory)
        break;

      char *filename;
      util_get_filename (
        directory, "files", &(file->hash), &filename
      );

      if (GNUNET_YES == GNUNET_DISK_file_test_read(filename))
        remove(filename);

      GNUNET_free(filename);
      break;
    } default: {
      break;
    }
  }

  return NULL;
}

void
cb_task_finish_ticket_update (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_TicketProcess *tickets = (
    (struct GNUNET_CHAT_TicketProcess*) cls
  );

  struct GNUNET_CHAT_Handle *handle = tickets->handle;

  if (tickets->iter)
    GNUNET_RECLAIM_ticket_iteration_stop(tickets->iter);

  if (tickets->op)
    GNUNET_RECLAIM_cancel(tickets->op);

  GNUNET_CONTAINER_DLL_remove(
    handle->tickets_head,
    handle->tickets_tail,
    tickets
  );

  GNUNET_free(tickets);
}

void
cb_task_error_ticket_update (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_TicketProcess *tickets = (
    (struct GNUNET_CHAT_TicketProcess*) cls
  );

  handle_send_internal_message(
    tickets->handle,
    NULL,
    GNUNET_CHAT_FLAG_WARNING,
    "Ticket update failed!"
  );

  cb_task_finish_ticket_update(cls);
}

static void
cont_revoke_ticket_with_status (void *cls,
                                int32_t success,
                                const char *emsg)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_TicketProcess *tickets = (
    (struct GNUNET_CHAT_TicketProcess*) cls
  );

  tickets->op = NULL;

  struct GNUNET_CHAT_Handle *handle = tickets->handle;

  if (GNUNET_SYSERR == success)
  {
    handle_send_internal_message(
      handle,
      NULL,
      GNUNET_CHAT_KIND_WARNING,
      emsg
    );

    if (tickets->iter)
      GNUNET_RECLAIM_ticket_iteration_stop(tickets->iter);

    return;
  }

  if (tickets->iter)
    GNUNET_RECLAIM_ticket_iteration_next(tickets->iter);
}

void
cb_iterate_ticket_update (void *cls,
                          const struct GNUNET_RECLAIM_Ticket *ticket)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_TicketProcess *tickets = (
    (struct GNUNET_CHAT_TicketProcess*) cls
  );

  struct GNUNET_CHAT_Handle *handle = tickets->handle;

  if (tickets->op)
    GNUNET_RECLAIM_cancel(tickets->op);

  tickets->op = GNUNET_RECLAIM_ticket_revoke(
    handle->reclaim,
    &(tickets->identity),
    ticket,
    cont_revoke_ticket_with_status,
    tickets
  );
}

void
on_handle_gnunet_identity (void *cls,
                           struct GNUNET_IDENTITY_Ego *ego,
                           void **ctx,
                           const char *name)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_Handle* handle = cls;

  if (!ctx)
    return;

  if (!ego)
    goto send_refresh;

  struct GNUNET_CHAT_InternalAccounts *accounts = handle->accounts_head;

  while (accounts)
  {
    if (!(accounts->account))
      goto skip_account;

    if (ego != accounts->account->ego)
      goto check_matching_name;

    if (name)
    {
      util_set_name_field(name, &(accounts->account->name));

      if (handle->current == accounts->account)
	      handle_send_internal_message(
          handle,
          NULL,
          GNUNET_CHAT_FLAG_LOGIN,
          NULL
        );
    }
    else
    {
      const struct GNUNET_CRYPTO_PrivateKey *key;
      key = GNUNET_IDENTITY_ego_get_private_key(ego);

      if (key)
        handle_update_tickets(handle, key);

      if (handle->current == accounts->account)
	      handle_disconnect(handle);

      account_destroy(accounts->account);

      if (accounts->op)
      {
        accounts->account = NULL;
        goto send_refresh;
      }

      GNUNET_CONTAINER_DLL_remove(
        handle->accounts_head,
        handle->accounts_tail,
        accounts
      );

      if (accounts->identifier)
	      GNUNET_free(accounts->identifier);

      GNUNET_free(accounts);
    }

    goto send_refresh;

check_matching_name:
    if ((name) && (accounts->account->name) &&
	      (0 == strcmp(accounts->account->name, name)))
    {
      accounts->account->ego = ego;
      goto send_refresh;
    }

skip_account:
    accounts = accounts->next;
  }

  if (!name)
    return;

  accounts = GNUNET_new(struct GNUNET_CHAT_InternalAccounts);
  accounts->account = account_create_from_ego(ego, name);

  accounts->handle = handle;
  accounts->op = NULL;

  accounts->wait_for_completion = GNUNET_NO;

  if (handle->directory)
    account_update_directory(accounts->account, handle->directory);

  GNUNET_CONTAINER_DLL_insert_tail(
    handle->accounts_head,
    handle->accounts_tail,
    accounts
  );

send_refresh:
  handle_send_internal_message(handle, NULL, GNUNET_CHAT_FLAG_REFRESH, NULL);
}

void
cb_account_creation (void *cls,
                     const struct GNUNET_CRYPTO_PrivateKey *key,
                     enum GNUNET_ErrorCode ec)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_InternalAccounts *accounts = (
    (struct GNUNET_CHAT_InternalAccounts*) cls
  );

  struct GNUNET_CHAT_Handle *handle = accounts->handle;

  GNUNET_CONTAINER_DLL_remove(
    handle->accounts_head,
    handle->accounts_tail,
    accounts
  );

  if (accounts->identifier)
    GNUNET_free(accounts->identifier);

  GNUNET_free(accounts);

  if (GNUNET_EC_NONE != ec)
  {
    handle_send_internal_message(
	    handle,
      NULL,
      GNUNET_CHAT_FLAG_WARNING,
      GNUNET_ErrorCode_get_hint(ec)
    );

    return;
  }
  else if (key)
    handle_send_internal_message(handle, NULL, GNUNET_CHAT_FLAG_REFRESH, NULL);
}

void
cb_account_deletion (void *cls,
		                 enum GNUNET_ErrorCode ec)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_InternalAccounts *accounts = (
    (struct GNUNET_CHAT_InternalAccounts*) cls
  );

  struct GNUNET_CHAT_Handle *handle = accounts->handle;

  GNUNET_CONTAINER_DLL_remove(
    handle->accounts_head,
    handle->accounts_tail,
    accounts
  );

  if (accounts->identifier)
    GNUNET_free(accounts->identifier);

  GNUNET_free(accounts);

  if (GNUNET_EC_NONE != ec)
  {
    handle_send_internal_message(
      handle,
      NULL,
      GNUNET_CHAT_FLAG_WARNING,
      GNUNET_ErrorCode_get_hint(ec)
    );

    return;
  }
}

void
cb_account_rename (void *cls,
		               enum GNUNET_ErrorCode ec)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_InternalAccounts *accounts = (
    (struct GNUNET_CHAT_InternalAccounts*) cls
  );

  struct GNUNET_CHAT_Handle *handle = accounts->handle;

  GNUNET_CONTAINER_DLL_remove(
    handle->accounts_head,
    handle->accounts_tail,
    accounts
  );

  if (accounts->identifier)
    GNUNET_free(accounts->identifier);

  GNUNET_free(accounts);

  if (GNUNET_EC_NONE != ec)
  {
    handle_send_internal_message(
      handle,
      NULL,
      GNUNET_CHAT_FLAG_WARNING,
      GNUNET_ErrorCode_get_hint(ec)
    );

    return;
  }
}

static void
cb_account_update_completion (void *cls,
                              const struct GNUNET_CRYPTO_PrivateKey *key,
                              enum GNUNET_ErrorCode ec)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_InternalAccounts *accounts = (
    (struct GNUNET_CHAT_InternalAccounts*) cls
  );

  struct GNUNET_CHAT_Handle *handle = accounts->handle;

  if ((GNUNET_EC_NONE == ec) && (key))
  {
    const struct GNUNET_CRYPTO_PrivateKey *prev= handle_get_key(handle);

    if (prev)
      handle_update_tickets(handle, prev);

    GNUNET_MESSENGER_set_key(handle->messenger, key);
  }

  cb_account_creation(cls, key, ec);
}

void
cb_account_update (void *cls,
		               enum GNUNET_ErrorCode ec)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_InternalAccounts *accounts = (
    (struct GNUNET_CHAT_InternalAccounts*) cls
  );

  struct GNUNET_CHAT_Handle *handle = accounts->handle;

  if ((GNUNET_EC_NONE != ec) || (!accounts->identifier))
  {
    cb_account_deletion(cls, ec);
    return;
  }

  accounts->op = GNUNET_IDENTITY_create(
    handle->identity,
    accounts->identifier,
    NULL,
    GNUNET_PUBLIC_KEY_TYPE_ECDSA,
    cb_account_update_completion,
    accounts
  );
}

int
intern_provide_contact_for_member(struct GNUNET_CHAT_Handle *handle,
                                  const struct GNUNET_MESSENGER_Contact *member,
                                  struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((handle) && (handle->contacts));

  if (!member)
    return GNUNET_SYSERR;

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
  const struct GNUNET_CRYPTO_PublicKey *ignore_key;
  const struct GNUNET_MESSENGER_Contact *contact;
};

int
check_handle_room_members (void* cls,
			                     GNUNET_UNUSED struct GNUNET_MESSENGER_Room *room,
                           const struct GNUNET_MESSENGER_Contact *member)
{
  struct GNUNET_CHAT_CheckHandleRoomMembers *check = cls;

  GNUNET_assert((check) && (member));

  const struct GNUNET_CRYPTO_PublicKey *member_key = (
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
on_monitor_namestore_record(void *cls,
                            GNUNET_UNUSED const
                            struct GNUNET_CRYPTO_PrivateKey *zone,
                            const char *label,
                            unsigned int count,
                            const struct GNUNET_GNSRECORD_Data *data)
{
  struct GNUNET_CHAT_Handle *chat = cls;

  if (chat->destruction)
  {
    GNUNET_NAMESTORE_zone_monitor_stop(chat->monitor);
    chat->monitor = NULL;
    return;
  }

  handle_process_records(chat, label, count, data);

  if (chat->monitor)
    GNUNET_NAMESTORE_zone_monitor_next(chat->monitor, 1);
}

void
on_handle_message_callback(void *cls);

static enum GNUNET_GenericReturnValue
it_context_iterate_dependencies(void *cls,
                                const struct GNUNET_HashCode *key,
                                void *value)
{
  struct GNUNET_CHAT_Message *message = (struct GNUNET_CHAT_Message*) value;

  if ((message) && (!message->task))
    message->task = GNUNET_SCHEDULER_add_now(
      on_handle_message_callback, message);

  return GNUNET_YES;
}

void
on_handle_message_callback(void *cls)
{
  struct GNUNET_CHAT_Message *message = (struct GNUNET_CHAT_Message*) cls;

  GNUNET_assert(
    (message) &&
		(message->context) &&
		(message->context->handle)
  );

  message->task = NULL;

  if (! message->msg)
    return;

  struct GNUNET_CHAT_Context *context = message->context;
  struct GNUNET_CHAT_Handle *handle = context->handle;

  if (!(handle->msg_cb))
    goto clear_dependencies;

  const struct GNUNET_MESSENGER_Contact *sender;
  sender = GNUNET_MESSENGER_get_sender(context->room, &(message->hash));

  if (!sender)
    goto clear_dependencies;

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(sender, &shorthash);

  struct GNUNET_CHAT_Contact *contact = GNUNET_CONTAINER_multishortmap_get(
    handle->contacts, &shorthash
  );

  if ((!contact) || (GNUNET_YES == contact->blocked))
    goto clear_dependencies;

  handle->msg_cb(handle->msg_cls, context, message);

clear_dependencies:
  GNUNET_CONTAINER_multihashmap_get_multiple(context->dependencies,
                                             &(message->hash),
                                             it_context_iterate_dependencies,
                                             NULL);
  GNUNET_CONTAINER_multihashmap_remove_all(context->dependencies,
                                           &(message->hash));
}

void
on_handle_message (void *cls,
                   struct GNUNET_MESSENGER_Room *room,
                   const struct GNUNET_MESSENGER_Contact *sender,
                   const struct GNUNET_MESSENGER_Contact *recipient,
                   const struct GNUNET_MESSENGER_Message *msg,
                   const struct GNUNET_HashCode *hash,
                   enum GNUNET_MESSENGER_MessageFlags flags)
{
  struct GNUNET_CHAT_Handle *handle = cls;

  GNUNET_assert(
    (handle) &&
		(room) &&
		(msg) &&
		(hash)
  );

  if ((handle->destruction) ||
      (GNUNET_OK != handle_request_context_by_room(handle, room)))
    return;

  GNUNET_MESSENGER_get_message(room, &(msg->header.previous));

  if (GNUNET_MESSENGER_KIND_MERGE == msg->header.kind)
    GNUNET_MESSENGER_get_message(room, &(msg->body.merge.previous));

  if ((GNUNET_CHAT_KIND_UNKNOWN == util_message_kind_from_kind(msg->header.kind)) ||
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

  struct GNUNET_CHAT_Contact *contact = GNUNET_CONTAINER_multishortmap_get(
    handle->contacts, &shorthash
  );

  if (flags & GNUNET_MESSENGER_FLAG_SENT)
    contact->owned = GNUNET_YES;

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

  struct GNUNET_SCHEDULER_Task *task = NULL;
  const struct GNUNET_HashCode *dependency = NULL;

  struct GNUNET_CHAT_Message *message = GNUNET_CONTAINER_multihashmap_get(
    context->messages, hash
  );

  if (message)
  {
    message_update_msg (message, flags, msg);

    if (message->flags & GNUNET_MESSENGER_FLAG_UPDATE)
      goto handle_callback;
    else
      return;
  }
  else if (msg->header.kind == GNUNET_MESSENGER_KIND_DELETE)
  {
    message = GNUNET_CONTAINER_multihashmap_get(
      context->messages, &(msg->body.deletion.hash)
    );

    if ((!message) || (message->msg) || 
        (0 == (message->flags & GNUNET_MESSENGER_FLAG_DELETE)))
      return;
  }

  message = message_create_from_msg(context, hash, flags, msg);

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      context->messages, hash, message,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
  {
    if (task)
      GNUNET_SCHEDULER_cancel(task);

    message_destroy(message);
    return;
  }

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
	      context, hash, &(msg->body.invite)
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
      struct GNUNET_TIME_Relative delay = GNUNET_TIME_relative_ntoh(
	      msg->body.deletion.delay
      );

      task = GNUNET_SCHEDULER_add_delayed(
	      GNUNET_TIME_absolute_get_difference(
          GNUNET_TIME_absolute_get(),
          GNUNET_TIME_absolute_add(timestamp, delay)
	      ),
        on_handle_message_callback,
        message
      );
      break;
    }
    case GNUNET_MESSENGER_KIND_TICKET:
    {
      struct GNUNET_CHAT_InternalTickets *tickets = GNUNET_new(
        struct GNUNET_CHAT_InternalTickets
      );

      if (!tickets)
        break;

      tickets->ticket = ticket_create_from_message(
	      handle, sender, &(msg->body.ticket)
      );

      if (!tickets->ticket)
      {
        GNUNET_free(tickets);
        break;
      }

      GNUNET_CONTAINER_DLL_insert_tail(
        contact->tickets_head,
        contact->tickets_tail,
        tickets
      );
      break;
    }
    case GNUNET_MESSENGER_KIND_TAG:
    {
      if (msg->body.tag.tag)
        break;

      GNUNET_CONTAINER_multihashmap_put(
        context->rejections, &(msg->body.tag.hash), 
        message, GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE);

      break;
    }
    default:
      break;
  }

handle_callback:
  switch (msg->header.kind)
  {
    case GNUNET_MESSENGER_KIND_DELETE:
    {
      dependency = &(msg->body.deletion.hash);
      break;
    }
    case GNUNET_MESSENGER_KIND_TRANSCRIPT:
    {
      dependency = &(msg->body.transcript.hash);
      break;
    }
    case GNUNET_MESSENGER_KIND_TAG:
    {
      dependency = &(msg->body.tag.hash);
      break;
    }
    default:
      break;
  }

  if ((dependency) && 
      (GNUNET_YES != GNUNET_CONTAINER_multihashmap_contains(context->messages, dependency)))
  {
    GNUNET_CONTAINER_multihashmap_put(
      context->dependencies,
      dependency,
      message,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE
    );

    GNUNET_MESSENGER_get_message(room, dependency);
  }
  else if (!task)
    on_handle_message_callback(message);

  message->task = task;
}

int
it_destroy_handle_groups (GNUNET_UNUSED void *cls,
                          GNUNET_UNUSED const struct GNUNET_HashCode *key,
                          void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_Group *group = value;
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
