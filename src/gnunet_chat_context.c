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
 * @file gnunet_chat_context.c
 */

#include "gnunet_chat_context.h"
#include "gnunet_chat_handle.h"
#include "gnunet_chat_util.h"

#include "gnunet_chat_context_intern.c"
#include <gnunet/gnunet_namestore_service.h>

static const unsigned int initial_map_size_of_room = 8;
static const unsigned int initial_map_size_of_contact = 4;

static void
init_new_context (struct GNUNET_CHAT_Context *context,
                  unsigned int initial_map_size)
{
  GNUNET_assert(context);

  context->nick[0] = '\0';
  context->topic = NULL;
  context->deleted = GNUNET_NO;

  context->timestamps = GNUNET_CONTAINER_multishortmap_create(
    initial_map_size, GNUNET_NO);
  context->dependencies = GNUNET_CONTAINER_multihashmap_create(
    initial_map_size, GNUNET_NO);
  context->messages = GNUNET_CONTAINER_multihashmap_create(
    initial_map_size, GNUNET_NO);
  context->invites = GNUNET_CONTAINER_multihashmap_create(
    initial_map_size, GNUNET_NO);
  context->files = GNUNET_CONTAINER_multihashmap_create(
    initial_map_size, GNUNET_NO);
  
  context->user_pointer = NULL;

  context->member_pointers = GNUNET_CONTAINER_multishortmap_create(
    initial_map_size, GNUNET_NO);

  context->query = NULL;
}

struct GNUNET_CHAT_Context*
context_create_from_room (struct GNUNET_CHAT_Handle *handle,
			                    struct GNUNET_MESSENGER_Room *room)
{
  GNUNET_assert((handle) && (room));

  struct GNUNET_CHAT_Context* context = GNUNET_new(struct GNUNET_CHAT_Context);

  context->handle = handle;
  context->type = GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN;
  
  init_new_context(context, initial_map_size_of_room);

  context->room = room;
  context->contact = NULL;

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

  init_new_context(context, initial_map_size_of_contact);

  context->room = NULL;
  context->contact = contact;

  return context;
}

void
context_destroy (struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert(
    (context) &&
		(context->timestamps) &&
    (context->dependencies) &&
		(context->messages) &&
		(context->invites) &&
		(context->files)
  );

  if (context->query)
    GNUNET_NAMESTORE_cancel(context->query);

  GNUNET_CONTAINER_multishortmap_iterate(
    context->timestamps, it_destroy_context_timestamps, NULL
  );

  GNUNET_CONTAINER_multihashmap_clear(context->dependencies);
  GNUNET_CONTAINER_multihashmap_iterate(
    context->messages, it_destroy_context_messages, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
    context->invites, it_destroy_context_invites, NULL
  );

  GNUNET_CONTAINER_multishortmap_destroy(context->member_pointers);

  GNUNET_CONTAINER_multishortmap_destroy(context->timestamps);
  GNUNET_CONTAINER_multihashmap_destroy(context->dependencies);
  GNUNET_CONTAINER_multihashmap_destroy(context->messages);
  GNUNET_CONTAINER_multihashmap_destroy(context->invites);
  GNUNET_CONTAINER_multihashmap_destroy(context->files);

  if (context->topic)
    GNUNET_free(context->topic);

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
  context->timestamps = GNUNET_CONTAINER_multishortmap_create(
    initial_map_size_of_room, GNUNET_NO);

  GNUNET_CONTAINER_multihashmap_clear(context->messages);
  GNUNET_CONTAINER_multihashmap_clear(context->invites);
  GNUNET_CONTAINER_multihashmap_clear(context->files);

  context->room = room;

  if (!(context->room))
    return;

  context_write_records(context);
}

void
context_update_nick (struct GNUNET_CHAT_Context *context,
		                 const char *nick)
{
  GNUNET_assert(context);

  size_t len = nick? strlen(nick) : 0;
  if (len >= sizeof(context->nick))
    len = sizeof(context->nick) - 1;

  GNUNET_memcpy(context->nick, nick, len);
  context->nick[len] = '\0';

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
context_read_records (struct GNUNET_CHAT_Context *context,
                      const char *label,
                      unsigned int count,
                      const struct GNUNET_GNSRECORD_Data *data)
{
  GNUNET_assert((context) && (context->room));

  char *nick = NULL;
  char *topic = NULL;
  uint32_t flags = 0;

  for (unsigned int i = 0; i < count; i++)
  {
    if (!(GNUNET_GNSRECORD_RF_SUPPLEMENTAL & data[i].flags))
      continue;

    if (GNUNET_GNSRECORD_TYPE_MESSENGER_ROOM_DETAILS == data[i].record_type)
    {
      if (nick)
	      continue;

      const struct GNUNET_MESSENGER_RoomDetailsRecord *record = data[i].data;

      nick = GNUNET_strndup(record->name, sizeof(record->name));
      flags = record->flags;
    }

    if (GNUNET_DNSPARSER_TYPE_TXT == data[i].record_type)
    {
      if (topic)
	      continue;

      topic = GNUNET_strndup(data[i].data, data[i].data_size);
    }
  }

  context_update_nick(context, nick);

  if (nick)
    GNUNET_free(nick);

  const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
    context->room
  );

  if (topic)
  {
    struct GNUNET_HashCode topic_hash;
    GNUNET_CRYPTO_hash(topic, strlen(topic), &topic_hash);

    if (0 != GNUNET_CRYPTO_hash_cmp(&topic_hash, hash))
    {
      GNUNET_free(topic);
      topic = NULL;
    }
  }

  util_set_name_field(topic, &(context->topic));

  if (topic)
    GNUNET_free(topic);

  context->type = util_get_context_label_type(label, hash);
}

void
context_write_records (struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((context) && (context->handle) && (context->room));

  const struct GNUNET_CRYPTO_PrivateKey *zone = handle_get_key(
    context->handle
  );

  if (!zone)
    return;

  const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
    context->room
  );

  struct GNUNET_TIME_Absolute expiration = GNUNET_TIME_absolute_get_forever_();

  struct GNUNET_MESSENGER_RoomEntryRecord room_entry;
  GNUNET_CRYPTO_get_peer_identity(context->handle->cfg, &(room_entry.door));

  GNUNET_memcpy(
    &(room_entry.key),
    hash,
    sizeof(room_entry.key)
  );

  struct GNUNET_MESSENGER_RoomDetailsRecord room_details;
  const char *topic = context->topic;

  if (topic)
  {
    struct GNUNET_HashCode topic_hash;
    GNUNET_CRYPTO_hash(topic, strlen(topic), &topic_hash);

    if (0 != GNUNET_CRYPTO_hash_cmp(&topic_hash, hash))
      topic = NULL;
  }

  char *label;
  util_get_context_label(context->type, hash, &label);

  unsigned int count = 0;
  struct GNUNET_GNSRECORD_Data data [3];

  if (GNUNET_YES == context->deleted)
    goto skip_record_data;

  data[count].record_type = GNUNET_GNSRECORD_TYPE_MESSENGER_ROOM_ENTRY;
  data[count].data = &room_entry;
  data[count].data_size = sizeof(room_entry);
  data[count].expiration_time = expiration.abs_value_us;
  data[count].flags = GNUNET_GNSRECORD_RF_PRIVATE;
  count++;

  if (context->nick)
  {
    GNUNET_memcpy(room_details.name, context->nick, sizeof(room_details.name));
    room_details.flags = 0;

    data[count].record_type = GNUNET_GNSRECORD_TYPE_MESSENGER_ROOM_DETAILS;
    data[count].data = &room_details;
    data[count].data_size = sizeof(room_details);
    data[count].expiration_time = expiration.abs_value_us;
    data[count].flags = (
      GNUNET_GNSRECORD_RF_PRIVATE |
      GNUNET_GNSRECORD_RF_SUPPLEMENTAL
    );

    count++;
  }

  if (topic)
  {
    data[count].record_type = GNUNET_DNSPARSER_TYPE_TXT;
    data[count].data = topic;
    data[count].data_size = strlen(topic);
    data[count].expiration_time = expiration.abs_value_us;
    data[count].flags = (
      GNUNET_GNSRECORD_RF_PRIVATE |
      GNUNET_GNSRECORD_RF_SUPPLEMENTAL
    );

    count++;
  }

skip_record_data:
  if (context->query)
    GNUNET_NAMESTORE_cancel(context->query);

  context->query = GNUNET_NAMESTORE_record_set_store(
    context->handle->namestore,
    zone,
    label,
    count,
    data,
    cont_context_write_records,
    context
  );

  GNUNET_free(label);
}
