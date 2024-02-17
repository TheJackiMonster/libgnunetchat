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
 * @file gnunet_chat_lib_intern.c
 */

#include "gnunet_chat_contact.h"
#include "gnunet_chat_handle.h"

#include <gnunet/gnunet_common.h>
#include <stdlib.h>

#define GNUNET_UNUSED __attribute__ ((unused))

void
task_handle_destruction (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_Handle *handle = (struct GNUNET_CHAT_Handle*) cls;

  struct GNUNET_CHAT_InternalAccounts *accounts = handle->accounts_head;
  while (accounts)
  {
    if ((accounts->op) && (GNUNET_YES == accounts->wait_for_completion))
      break;

    accounts = accounts->next;
  }

  if (accounts)
  {
    handle->destruction = GNUNET_SCHEDULER_add_at_with_priority(
      GNUNET_TIME_absolute_add(
          GNUNET_TIME_absolute_get(),
          GNUNET_TIME_relative_get_millisecond_()
      ),
      GNUNET_SCHEDULER_PRIORITY_IDLE,
      task_handle_destruction,
      handle
    );

    return;
  }

  handle->destruction = NULL;
  handle_destroy(handle);
}

void
task_handle_disconnection (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_Handle *handle = (struct GNUNET_CHAT_Handle*) cls;

  handle_disconnect(handle);
  handle->disconnection = NULL;
}

void
cb_lobby_lookup (void *cls,
                 uint32_t count,
                 const struct GNUNET_GNSRECORD_Data *data)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_UriLookups *lookups = (struct GNUNET_CHAT_UriLookups*) cls;

  if ((!(lookups->handle)) || (!(lookups->uri)))
    goto drop_lookup;

  struct GNUNET_CHAT_Context *context = handle_process_records(
    lookups->handle,
    lookups->uri->label,
    count,
    data
  );

  if (context)
    context_write_records(context);

drop_lookup:
  if (lookups->uri)
    uri_destroy(lookups->uri);

  if (lookups->handle)
    GNUNET_CONTAINER_DLL_remove(
      lookups->handle->lookups_head,
      lookups->handle->lookups_tail,
      lookups
    );

  GNUNET_free(lookups);
}

struct GNUNET_CHAT_HandleIterateContacts
{
  struct GNUNET_CHAT_Handle *handle;
  GNUNET_CHAT_ContactCallback cb;
  void *cls;
};

enum GNUNET_GenericReturnValue
it_handle_iterate_contacts (void *cls,
                            GNUNET_UNUSED const struct GNUNET_ShortHashCode *key,
                            void *value)
{
  GNUNET_assert((cls) && (value));

  struct GNUNET_CHAT_HandleIterateContacts *it = cls;

  if (!(it->cb))
    return GNUNET_YES;

  struct GNUNET_CHAT_Contact *contact = value;

  return it->cb(it->cls, it->handle, contact);
}

struct GNUNET_CHAT_HandleIterateGroups
{
  struct GNUNET_CHAT_Handle *handle;
  GNUNET_CHAT_GroupCallback cb;
  void *cls;
};

enum GNUNET_GenericReturnValue
it_handle_iterate_groups (void *cls,
                          GNUNET_UNUSED const struct GNUNET_HashCode *key,
                          void *value)
{
  GNUNET_assert((cls) && (value));

  struct GNUNET_CHAT_HandleIterateGroups *it = cls;

  if (!(it->cb))
    return GNUNET_YES;

  struct GNUNET_CHAT_Group *group = value;

  return it->cb(it->cls, it->handle, group);
}

typedef void
(*GNUNET_CHAT_ContactIterateContextCallback) (struct GNUNET_CHAT_Contact *contact,
                                              struct GNUNET_CHAT_Context *context,
                                              const char *tag);

struct GNUNET_CHAT_ContactIterateContexts
{
  struct GNUNET_CHAT_Contact *contact;
  const char *tag;

  GNUNET_CHAT_ContactIterateContextCallback cb;
};

enum GNUNET_GenericReturnValue
it_contact_iterate_contexts (void *cls,
                             const struct GNUNET_HashCode *key,
                             GNUNET_UNUSED void *value)
{
  GNUNET_assert((cls) && (key));

  struct GNUNET_CHAT_ContactIterateContexts *it = cls;

  if (!(it->cb))
    return GNUNET_YES;

  struct GNUNET_CHAT_Handle *handle = it->contact->handle;
  struct GNUNET_CHAT_Context *context = GNUNET_CONTAINER_multihashmap_get(
    handle->contexts, key);
  
  if (! context)
    return GNUNET_YES;

  it->cb(it->contact, context, it->tag);
  return GNUNET_YES;
}

struct GNUNET_CHAT_RoomFindContact
{
  const struct GNUNET_CRYPTO_PublicKey *ignore_key;
  const struct GNUNET_MESSENGER_Contact *contact;
};

enum GNUNET_GenericReturnValue
it_room_find_contact (void *cls,
                      GNUNET_UNUSED struct GNUNET_MESSENGER_Room *room,
                      const struct GNUNET_MESSENGER_Contact *member)
{
  GNUNET_assert((cls) && (member));

  const struct GNUNET_CRYPTO_PublicKey *key = GNUNET_MESSENGER_contact_get_key(
      member
  );

  struct GNUNET_CHAT_RoomFindContact *find = cls;

  if ((find->ignore_key) && (key) &&
      (0 == GNUNET_memcmp(find->ignore_key, key)))
    return GNUNET_YES;

  find->contact = member;
  return GNUNET_NO;
}

void
task_group_destruction (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_Group *group = (struct GNUNET_CHAT_Group*) cls;

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

  group->context->deleted = GNUNET_YES;
  context_write_records(group->context);

  group->destruction = NULL;

  context_destroy(group->context);
  group_destroy(group);
}

struct GNUNET_CHAT_GroupIterateContacts
{
  const struct GNUNET_CHAT_Group *group;
  GNUNET_CHAT_GroupContactCallback cb;
  void *cls;
};

enum GNUNET_GenericReturnValue
it_group_iterate_contacts (void* cls,
			                     GNUNET_UNUSED struct GNUNET_MESSENGER_Room *room,
                           const struct GNUNET_MESSENGER_Contact *member)
{
  GNUNET_assert((cls) && (member));

  struct GNUNET_CHAT_GroupIterateContacts *it = cls;

  if (!(it->cb))
    return GNUNET_YES;

  return it->cb(it->cls, it->group, handle_get_contact_from_messenger(
    it->group->handle, member
  ));
}

struct GNUNET_CHAT_ContextIterateMessages
{
  struct GNUNET_CHAT_Context *context;
  GNUNET_CHAT_ContextMessageCallback cb;
  void *cls;
};

enum GNUNET_GenericReturnValue
it_context_iterate_messages (void *cls,
                             GNUNET_UNUSED const struct GNUNET_HashCode *key,
                             void *value)
{
  GNUNET_assert((cls) && (value));

  struct GNUNET_CHAT_ContextIterateMessages *it = cls;

  if (!(it->cb))
    return GNUNET_YES;

  struct GNUNET_CHAT_Message *message = value;

  return it->cb(it->cls, it->context, message);
}

struct GNUNET_CHAT_ContextIterateFiles
{
  struct GNUNET_CHAT_Context *context;
  GNUNET_CHAT_ContextFileCallback cb;
  void *cls;
};

enum GNUNET_GenericReturnValue
it_context_iterate_files (void *cls,
                          const struct GNUNET_HashCode *key,
                          GNUNET_UNUSED void *value)
{
  GNUNET_assert((cls) && (key));

  struct GNUNET_CHAT_ContextIterateFiles *it = cls;

  if (!(it->cb))
    return GNUNET_YES;

  struct GNUNET_CHAT_Message *message = GNUNET_CONTAINER_multihashmap_get(
    it->context->messages, key
  );

  if (!message)
    return GNUNET_YES;

  struct GNUNET_CHAT_File *file = GNUNET_CONTAINER_multihashmap_get(
    it->context->handle->files, &(message->hash)
  );

  if (!file)
    return GNUNET_YES;

  return it->cb(it->cls, it->context, file);
}

struct GNUNET_CHAT_MessageIterateReadReceipts
{
  const struct GNUNET_CHAT_Message *message;
  GNUNET_CHAT_MessageReadReceiptCallback cb;
  void *cls;
};

enum GNUNET_GenericReturnValue
it_message_iterate_read_receipts (void *cls,
                                  GNUNET_UNUSED struct GNUNET_MESSENGER_Room *room,
                                  const struct GNUNET_MESSENGER_Contact *member)
{
  GNUNET_assert((cls) && (member));

  struct GNUNET_CHAT_MessageIterateReadReceipts *it = cls;
  struct GNUNET_CHAT_Handle *handle = it->message->context->handle;

  if (!handle)
    return GNUNET_NO;

  struct GNUNET_ShortHashCode shorthash;
  util_shorthash_from_member(member, &shorthash);

  struct GNUNET_CHAT_Contact *contact = GNUNET_CONTAINER_multishortmap_get(
      handle->contacts, &shorthash
  );

  if (!contact)
    return GNUNET_YES;

  struct GNUNET_TIME_Absolute *timestamp = GNUNET_CONTAINER_multishortmap_get(
    it->message->context->timestamps, &shorthash
  );

  if (!timestamp)
    return GNUNET_YES;

  struct GNUNET_TIME_Relative delta = GNUNET_TIME_absolute_get_difference(
    *timestamp, GNUNET_CHAT_message_get_timestamp(it->message)
  );

  int read_receipt;
  if (GNUNET_TIME_relative_get_zero_().rel_value_us == delta.rel_value_us)
    read_receipt = GNUNET_YES;
  else
    read_receipt = GNUNET_NO;

  if (it->cb)
    it->cb(it->cls, it->message, contact, read_receipt);

  return GNUNET_YES;
}

void
cont_update_attribute_with_status (void *cls,
                                   int32_t success,
                                   const char *emsg)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_AttributeProcess *attributes = (
    (struct GNUNET_CHAT_AttributeProcess*) cls
  );

  attributes->op = NULL;

  struct GNUNET_CHAT_Handle *handle = attributes->handle;

  if (GNUNET_SYSERR == success)
  {
    handle_send_internal_message(
      handle,
      NULL,
      GNUNET_CHAT_KIND_WARNING,
      emsg
    );

    return;
  }

  GNUNET_CONTAINER_DLL_remove(
    handle->attributes_head,
    handle->attributes_tail,
    attributes
  );
}

void
cb_task_finish_iterate_attribute (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_AttributeProcess *attributes = (
    (struct GNUNET_CHAT_AttributeProcess*) cls
  );

  struct GNUNET_CHAT_Handle *handle = attributes->handle;

  if (attributes->iter)
    GNUNET_RECLAIM_get_attributes_stop(attributes->iter);

  GNUNET_CONTAINER_DLL_remove(
    handle->attributes_head,
    handle->attributes_tail,
    attributes
  );

  GNUNET_free(attributes);
}

void
cb_task_error_iterate_attribute (void *cls)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_AttributeProcess *attributes = (
    (struct GNUNET_CHAT_AttributeProcess*) cls
  );

  handle_send_internal_message(
    attributes->handle,
    NULL,
    GNUNET_CHAT_FLAG_WARNING,
    "Attribute iteration failed!"
  );

  cb_task_finish_iterate_attribute(cls);
}

void
cb_iterate_attribute (void *cls,
                      const struct GNUNET_CRYPTO_PublicKey *identity,
                      const struct GNUNET_RECLAIM_Attribute *attribute)
{
  GNUNET_assert(cls);

  struct GNUNET_CHAT_AttributeProcess *attributes = (
    (struct GNUNET_CHAT_AttributeProcess*) cls
  );

  struct GNUNET_CHAT_Handle *handle = attributes->handle;

  const char *value = GNUNET_RECLAIM_attribute_value_to_string(
    attribute->type,
    attribute->data,
    attribute->data_size
  );

  if (attributes->callback)
    attributes->callback(attributes->closure, handle, attribute->name, value);

  if (attributes->iter)
    GNUNET_RECLAIM_get_attributes_next(attributes->iter);
}
