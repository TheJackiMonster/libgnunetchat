/*
   This file is part of GNUnet.
   Copyright (C) 2021--2023 GNUnet e.V.

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

static const unsigned int initial_map_size_of_handle = 8;
static const unsigned int minimum_amount_of_other_members_in_group = 2;

struct GNUNET_CHAT_Handle*
handle_create_from_config (const struct GNUNET_CONFIGURATION_Handle* cfg,
			   GNUNET_CHAT_ContextMessageCallback msg_cb,
			   void *msg_cls)
{
  GNUNET_assert(cfg);

  struct GNUNET_CHAT_Handle* handle = GNUNET_new(struct GNUNET_CHAT_Handle);

  handle->cfg = cfg;
  handle->shutdown_hook = GNUNET_SCHEDULER_add_shutdown(
      on_handle_shutdown, handle
  );

  handle->destruction = NULL;

  handle->internal_head = NULL;
  handle->internal_tail = NULL;

  handle->directory = NULL;

  if (GNUNET_OK != GNUNET_CONFIGURATION_get_value_filename(cfg,
		     GNUNET_MESSENGER_SERVICE_NAME,
		     "MESSENGER_DIR",
		     &(handle->directory)))
  {
    if (handle->directory)
      GNUNET_free(handle->directory);

    handle->directory = NULL;
  }
  else if ((GNUNET_YES != GNUNET_DISK_directory_test(handle->directory, GNUNET_YES)) &&
	   (GNUNET_OK != GNUNET_DISK_directory_create(handle->directory)))
  {
    GNUNET_free(handle->directory);

    handle->directory = NULL;
  }

  if (handle->directory)
  {
    char *chat_directory = NULL;
    util_get_dirname(handle->directory, "chat", &chat_directory);

    if ((GNUNET_YES != GNUNET_DISK_directory_test(chat_directory, GNUNET_YES)) &&
    	(GNUNET_OK != GNUNET_DISK_directory_create(chat_directory)))
      GNUNET_free(chat_directory);
    else
    {
      GNUNET_free(handle->directory);
      handle->directory = chat_directory;
    }
  }

  handle->msg_cb = msg_cb;
  handle->msg_cls = msg_cls;

  handle->accounts_head = NULL;
  handle->accounts_tail = NULL;

  handle->current = NULL;
  handle->monitor = NULL;

  handle->lobbies_head = NULL;
  handle->lobbies_tail = NULL;

  handle->lobbies_head = NULL;
  handle->lobbies_tail = NULL;

  handle->files = NULL;
  handle->contexts = NULL;
  handle->contacts = NULL;
  handle->groups = NULL;

  handle->arm = GNUNET_ARM_connect(
      handle->cfg,
      on_handle_arm_connection, handle
  );

  if (handle->arm)
    on_handle_arm_connection(handle, GNUNET_NO);

  handle->identity = GNUNET_IDENTITY_connect(
      handle->cfg,
      on_handle_gnunet_identity,
      handle
  );

  handle->namestore = GNUNET_NAMESTORE_connect(
      handle->cfg
  );

  handle->fs = NULL;
  handle->gns = NULL;
  handle->messenger = NULL;

  handle->public_key = NULL;
  handle->user_pointer = NULL;
  return handle;
}

void
handle_update_key (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert(handle);

  if (handle->public_key)
    GNUNET_free(handle->public_key);

  handle->public_key = NULL;

  if (!(handle->messenger))
    return;

  const struct GNUNET_CRYPTO_PublicKey *pubkey;
  pubkey = GNUNET_MESSENGER_get_key(handle->messenger);

  if (pubkey)
    handle->public_key = GNUNET_CRYPTO_public_key_to_string(pubkey);
}

void
handle_destroy (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert(handle);

  if (handle->shutdown_hook)
    GNUNET_SCHEDULER_cancel(handle->shutdown_hook);

  if (handle->destruction)
    GNUNET_SCHEDULER_cancel(handle->destruction);

  if (handle->monitor)
    GNUNET_NAMESTORE_zone_monitor_stop(handle->monitor);

  if (handle->current)
    handle_disconnect(handle);

  if (handle->namestore)
    GNUNET_NAMESTORE_disconnect(handle->namestore);

  struct GNUNET_CHAT_InternalAccounts *accounts;

  while (handle->accounts_head)
  {
    accounts = handle->accounts_head;

    if (accounts->op)
      GNUNET_IDENTITY_cancel(accounts->op);

    if (accounts->account)
      account_destroy(accounts->account);

    GNUNET_CONTAINER_DLL_remove(
      handle->accounts_head,
      handle->accounts_tail,
      accounts
    );

    if (accounts->identifier)
      GNUNET_free(accounts->identifier);

    GNUNET_free(accounts);
  }

  if (handle->identity)
    GNUNET_IDENTITY_disconnect(handle->identity);

  if (handle->arm)
    GNUNET_ARM_disconnect(handle->arm);

  if (handle->directory)
    GNUNET_free(handle->directory);

  struct GNUNET_CHAT_InternalMessages *internal;
  while (handle->internal_head)
  {
    internal = handle->internal_head;

    if (internal->msg)
      message_destroy(internal->msg);

    GNUNET_CONTAINER_DLL_remove(
	handle->internal_head,
	handle->internal_tail,
	internal
    );

    GNUNET_free(internal);
  }

  GNUNET_free(handle);
}

static void
handle_update_identity(struct GNUNET_CHAT_Handle *handle)
{
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

  handle_send_internal_message(
      handle,
      NULL,
      GNUNET_CHAT_FLAG_LOGIN,
      NULL
  );

  const struct GNUNET_CRYPTO_PrivateKey *zone = handle_get_key(handle);

  if ((!zone) || (handle->monitor))
    return;

  handle->monitor = GNUNET_NAMESTORE_zone_monitor_start(
      handle->cfg,
      zone,
      GNUNET_YES,
      NULL,
      NULL,
      on_monitor_namestore_record,
      handle,
      NULL,
      NULL
  );
}

void
handle_connect (struct GNUNET_CHAT_Handle *handle,
		const struct GNUNET_CHAT_Account *account)
{
  GNUNET_assert((handle) && (account) &&
		(!(handle->current)) &&
		(!(handle->groups)) &&
		(!(handle->contacts)) &&
		(!(handle->contexts)) &&
		(!(handle->files)));

  if (handle->monitor)
  {
    GNUNET_NAMESTORE_zone_monitor_stop(handle->monitor);
    handle->monitor = NULL;
  }

  handle->files = GNUNET_CONTAINER_multihashmap_create(
      initial_map_size_of_handle, GNUNET_NO);
  handle->contexts = GNUNET_CONTAINER_multihashmap_create(
      initial_map_size_of_handle, GNUNET_NO);
  handle->contacts = GNUNET_CONTAINER_multishortmap_create(
      initial_map_size_of_handle, GNUNET_NO);
  handle->groups = GNUNET_CONTAINER_multihashmap_create(
      initial_map_size_of_handle, GNUNET_NO);

  const char *name = account->name;

  const struct GNUNET_CRYPTO_PrivateKey *key = NULL;
  if (account->ego)
    key = GNUNET_IDENTITY_ego_get_private_key(account->ego);

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

  handle->gns = GNUNET_GNS_connect(handle->cfg);

  handle->messenger = GNUNET_MESSENGER_connect(
      handle->cfg, name, key,
      on_handle_message,
      handle
  );

  handle->current = account;
  handle_update_identity(handle);
}

void
handle_disconnect (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert((handle) &&
		(handle->current) &&
		(handle->groups) &&
		(handle->contacts) &&
		(handle->contexts) &&
		(handle->files));

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->groups, it_destroy_handle_groups, NULL
  );

  GNUNET_CONTAINER_multishortmap_iterate(
      handle->contacts, it_destroy_handle_contacts, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->contexts, it_destroy_handle_contexts, NULL
  );

  struct GNUNET_CHAT_InternalMessages *internal;
  while (handle->internal_head)
  {
    internal = handle->internal_head;

    if (!(internal->msg->context))
      break;

    if (internal->msg)
      message_destroy(internal->msg);

    GNUNET_CONTAINER_DLL_remove(
      handle->internal_head,
      handle->internal_tail,
      internal
    );

    GNUNET_free(internal);
  }

  if (handle->messenger)
    GNUNET_MESSENGER_disconnect(handle->messenger);

  struct GNUNET_CHAT_UriLookups *lookups;
  while (handle->lookups_head)
  {
    lookups = handle->lookups_head;

    if (lookups->request)
      GNUNET_GNS_lookup_cancel(lookups->request);

    if (lookups->uri)
      uri_destroy(lookups->uri);

    GNUNET_CONTAINER_DLL_remove(
      handle->lookups_head,
      handle->lookups_tail,
      lookups
    );

    GNUNET_free(lookups);
  }

  if (handle->gns)
    GNUNET_GNS_disconnect(handle->gns);

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->files, it_destroy_handle_files, NULL
  );

  if (handle->fs)
    GNUNET_FS_stop(handle->fs);

  handle->fs = NULL;
  handle->gns = NULL;
  handle->messenger = NULL;

  struct GNUNET_CHAT_InternalLobbies *lobbies;
  while (handle->lobbies_head)
  {
    lobbies = handle->lobbies_head;

    if (lobbies->lobby)
      lobby_destroy(lobbies->lobby);

    GNUNET_CONTAINER_DLL_remove(
      handle->lobbies_head,
      handle->lobbies_tail,
      lobbies
    );

    GNUNET_free(lobbies);
  }

  GNUNET_CONTAINER_multihashmap_destroy(handle->groups);
  GNUNET_CONTAINER_multishortmap_destroy(handle->contacts);
  GNUNET_CONTAINER_multihashmap_destroy(handle->contexts);
  GNUNET_CONTAINER_multihashmap_destroy(handle->files);

  handle->files = NULL;
  handle->contexts = NULL;
  handle->contacts = NULL;
  handle->groups = NULL;

  handle->current = NULL;
  handle_update_key(handle);
}

static struct GNUNET_CHAT_InternalAccounts*
find_accounts_by_name (struct GNUNET_CHAT_Handle *handle,
		       const char *name)
{
  GNUNET_assert((handle) && (name));

  struct GNUNET_CHAT_InternalAccounts *accounts = handle->accounts_head;
  while (accounts)
  {
    if (!(accounts->account))
      goto skip_account;

    if ((accounts->account->name) &&
      (0 == strcmp(accounts->account->name, name)))
      break;

  skip_account:
    accounts = accounts->next;
  }

  return accounts;
}

static int
update_accounts_operation (struct GNUNET_CHAT_InternalAccounts **out_accounts,
			   struct GNUNET_CHAT_Handle *handle,
			   const char *name,
			   int wait_for_completion)
{
  GNUNET_assert(handle);

  struct GNUNET_CHAT_InternalAccounts *accounts = *out_accounts;

  if (!accounts)
  {
    accounts = GNUNET_new(struct GNUNET_CHAT_InternalAccounts);

    if (!accounts)
      return GNUNET_SYSERR;

    accounts->account = NULL;
    accounts->handle = handle;

    GNUNET_CONTAINER_DLL_insert_tail(
	    handle->accounts_head,
    	handle->accounts_tail,
    	accounts
    );
  }
  else
  {
    if (accounts->identifier)
      GNUNET_free(accounts->identifier);

    if (accounts->op)
      GNUNET_IDENTITY_cancel(accounts->op);
  }

  accounts->identifier = name ? GNUNET_strdup(name) : NULL;
  accounts->wait_for_completion = wait_for_completion;

  *out_accounts = accounts;

  return GNUNET_OK;
}

int
handle_create_account (struct GNUNET_CHAT_Handle *handle,
		       const char *name)
{
  GNUNET_assert((handle) && (name));

  struct GNUNET_CHAT_InternalAccounts *accounts;
  accounts = find_accounts_by_name(handle, name);

  if (accounts)
    return GNUNET_NO;

  int result = update_accounts_operation(&accounts, handle, NULL, GNUNET_NO);

  if (GNUNET_OK != result)
    return result;

  accounts->op = GNUNET_IDENTITY_create(
      handle->identity,
      name,
      NULL,
      GNUNET_PUBLIC_KEY_TYPE_ECDSA,
      cb_account_creation,
      accounts
  );

  if (!accounts->op)
    return GNUNET_SYSERR;

  return result;
}

int
handle_delete_account (struct GNUNET_CHAT_Handle *handle,
		       const char *name)
{
  GNUNET_assert((handle) && (name));

  struct GNUNET_CHAT_InternalAccounts *accounts;
  accounts = find_accounts_by_name(handle, name);

  int result = update_accounts_operation(&accounts, handle, NULL, GNUNET_YES);

  if (GNUNET_OK != result)
    return result;

  accounts->op = GNUNET_IDENTITY_delete(
      handle->identity,
      name,
      cb_account_deletion,
      accounts
  );

  if (!accounts->op)
    return GNUNET_SYSERR;

  return result;
}

int
handle_rename_account (struct GNUNET_CHAT_Handle *handle,
		       const char *old_name,
		       const char *new_name)
{
  GNUNET_assert((handle) && (old_name) && (new_name));

  struct GNUNET_CHAT_InternalAccounts *accounts;
  accounts = find_accounts_by_name(handle, old_name);

  int result = update_accounts_operation(&accounts, handle, NULL, GNUNET_YES);

  if (GNUNET_OK != result)
    return result;

  accounts->op = GNUNET_IDENTITY_rename(
      handle->identity,
      old_name,
      new_name,
      cb_account_rename,
      accounts
  );

  if (!accounts->op)
    return GNUNET_SYSERR;

  return result;
}

const char*
handle_get_directory (const struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert(handle);

  if (!(handle->directory))
    return NULL;

  if (!(handle->current))
    return handle->directory;
  else
    return handle->current->directory;
}

int
handle_update (struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert((handle) && (handle->current));

  const char *name = handle->current->name;

  if (!name)
    return GNUNET_SYSERR;

  struct GNUNET_CHAT_InternalAccounts *accounts;
  accounts = find_accounts_by_name(handle, name);

  int result = update_accounts_operation(&accounts, handle, name, GNUNET_YES);

  if (GNUNET_OK != result)
    return result;

  accounts->op = GNUNET_IDENTITY_delete(
      handle->identity,
      name,
      cb_account_update,
      accounts
  );

  if (!accounts->op)
    return GNUNET_SYSERR;

  return result;
}

const struct GNUNET_CRYPTO_PrivateKey*
handle_get_key (const struct GNUNET_CHAT_Handle *handle)
{
  GNUNET_assert(handle);

  if ((!(handle->current)) || (!(handle->current->ego)))
    return NULL;

  return GNUNET_IDENTITY_ego_get_private_key(
      handle->current->ego
  );
}

void
handle_send_internal_message (struct GNUNET_CHAT_Handle *handle,
			      struct GNUNET_CHAT_Context *context,
			      enum GNUNET_CHAT_MessageFlag flag,
			      const char *warning)
{
  GNUNET_assert((handle) && (GNUNET_CHAT_FLAG_NONE != flag));

  if ((handle->destruction) || (!(handle->msg_cb)))
    return;

  struct GNUNET_CHAT_InternalMessages *internal = GNUNET_new(
      struct GNUNET_CHAT_InternalMessages
  );

  internal->msg = message_create_internally(
      context, flag, warning
  );

  if (!(internal->msg))
  {
    GNUNET_free(internal);
    return;
  }

  handle->msg_cb(handle->msg_cls, context, internal->msg);

  if (context)
    GNUNET_CONTAINER_DLL_insert(
        handle->internal_head,
        handle->internal_tail,
        internal
    );
  else
    GNUNET_CONTAINER_DLL_insert_tail(
    	handle->internal_head,
    	handle->internal_tail,
    	internal
    );
}

void
handle_send_room_name (struct GNUNET_CHAT_Handle *handle,
		       struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert((handle) && (handle->messenger) && (room));

  if (handle->destruction)
    return;

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
  {
    context->deleted = GNUNET_YES;
    context_write_records(context);

    context->type = GNUNET_CHAT_CONTEXT_TYPE_CONTACT;
    context->deleted = GNUNET_NO;

    context_write_records(context);
  }
  else if (checks >= minimum_amount_of_other_members_in_group)
  {
    context->deleted = GNUNET_YES;
    context_write_records(context);

    context->type = GNUNET_CHAT_CONTEXT_TYPE_GROUP;
    context->deleted = GNUNET_NO;

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

  if (context->topic)
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

    context_write_records(context);
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

struct GNUNET_CHAT_Context*
handle_process_records (struct GNUNET_CHAT_Handle *handle,
			const char *label,
			unsigned int count,
			const struct GNUNET_GNSRECORD_Data *data)
{
  GNUNET_assert((handle) && (data));

  if (!count)
    return NULL;

  const struct GNUNET_MESSENGER_RoomEntryRecord *record = NULL;

  for (unsigned int i = 0; i < count; i++)
  {
    if (GNUNET_YES == GNUNET_GNSRECORD_is_expired(data + i))
      continue;

    if ((GNUNET_GNSRECORD_TYPE_MESSENGER_ROOM_ENTRY == data[i].record_type) &&
      (!(GNUNET_GNSRECORD_RF_SUPPLEMENTAL & data[i].flags)))
    {
      record = data[i].data;
      break;
    }
  }

  if (!record)
    return NULL;

  struct GNUNET_CHAT_Context *context = GNUNET_CONTAINER_multihashmap_get(
      handle->contexts,
      &(record->key)
  );

  if (context)
  {
    context_read_records(context, label, count, data);
    return NULL;
  }

  struct GNUNET_MESSENGER_Room *room = GNUNET_MESSENGER_enter_room(
      handle->messenger,
      &(record->door),
      &(record->key)
  );

  if (!room)
    return NULL;

  context = context_create_from_room(handle, room);
  context_read_records(context, label, count, data);

  handle_send_room_name(handle, room);

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      handle->contexts, &(record->key), context,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
  {
    context_destroy(context);
    GNUNET_MESSENGER_close_room(room);
    return NULL;
  }

  if (GNUNET_CHAT_CONTEXT_TYPE_GROUP != context->type)
    return context;

  struct GNUNET_CHAT_Group *group = group_create_from_context(handle, context);

  if (context->topic)
    group_publish(group);

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      handle->groups, &(record->key), group,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
    group_destroy(group);

  return context;
}
