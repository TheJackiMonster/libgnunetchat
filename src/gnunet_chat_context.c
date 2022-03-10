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
#include "gnunet_chat_group.h"
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
  context->topic = NULL;

  context->timestamps = GNUNET_CONTAINER_multishortmap_create(8, GNUNET_NO);
  context->messages = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  context->invites = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  context->files = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);

  context->room = room;
  context->contact = NULL;

  context->user_pointer = NULL;

  context->member_pointers = GNUNET_CONTAINER_multishortmap_create(
      8, GNUNET_NO
  );

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
  context->topic = NULL;

  context->timestamps = GNUNET_CONTAINER_multishortmap_create(4, GNUNET_NO);
  context->messages = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);
  context->invites = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);
  context->files = GNUNET_CONTAINER_multihashmap_create(4, GNUNET_NO);

  context->room = NULL;
  context->contact = contact;

  context->user_pointer = NULL;

  context->member_pointers = GNUNET_CONTAINER_multishortmap_create(
      8, GNUNET_NO
  );

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

  GNUNET_CONTAINER_multishortmap_destroy(context->member_pointers);

  GNUNET_CONTAINER_multishortmap_destroy(context->timestamps);
  GNUNET_CONTAINER_multihashmap_destroy(context->messages);
  GNUNET_CONTAINER_multihashmap_destroy(context->invites);
  GNUNET_CONTAINER_multihashmap_destroy(context->files);

  if (context->topic)
    GNUNET_free(context->topic);

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

  if (!(context->room))
    return;

  context_write_records(context);
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
context_read_records (struct GNUNET_CHAT_Context *context,
		      const char *label,
		      unsigned int count,
		      const struct GNUNET_GNSRECORD_Data *data)
{
  GNUNET_assert((context) &&
  		(context->room));

  char *nick = NULL;
  char *topic = NULL;

  for (unsigned int i = 0; i < count; i++)
  {
    if (!(GNUNET_GNSRECORD_RF_SUPPLEMENTAL & data[i].flags))
      continue;

    if (GNUNET_GNSRECORD_TYPE_NICK == data[i].record_type)
    {
      if (nick)
      continue;

      nick = GNUNET_strndup(data[i].data, data[i].data_size);
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

  if (topic)
  {
    struct GNUNET_HashCode topic_hash;
    GNUNET_CRYPTO_hash(topic, strlen(topic), &topic_hash);

    const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
	context->room
    );

    if (0 != GNUNET_CRYPTO_hash_cmp(&topic_hash, hash))
    {
      GNUNET_free(topic);
      topic = NULL;
    }
  }

  util_set_name_field(topic, &(context->topic));

  if (topic)
    GNUNET_free(topic);

  if (0 == strncmp(label, "group_", 6))
    context->type = GNUNET_CHAT_CONTEXT_TYPE_GROUP;
  else if (0 == strncmp(label, "contact_", 8))
    context->type = GNUNET_CHAT_CONTEXT_TYPE_CONTACT;
  else
    context->type = GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN;
}

void
context_write_records (struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((context) &&
		(context->handle) &&
		(context->room));

  const struct GNUNET_IDENTITY_PrivateKey *zone = handle_get_key(
      context->handle
  );

  if (!zone)
    return;

  const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
      context->room
  );

  struct GNUNET_TIME_Absolute expiration = GNUNET_TIME_absolute_get_forever_();

  struct GNUNET_MESSENGER_RoomEntryRecord room;
  GNUNET_CRYPTO_get_peer_identity(context->handle->cfg, &(room.door));

  GNUNET_memcpy(
      &(room.key),
      hash,
      sizeof(room.key)
  );

  const char *nick = context->nick;
  const char *topic = context->topic;

  if (topic)
  {
    struct GNUNET_HashCode topic_hash;
    GNUNET_CRYPTO_hash(topic, strlen(topic), &topic_hash);

    if (0 != GNUNET_CRYPTO_hash_cmp(&topic_hash, hash))
      topic = NULL;
  }

  unsigned int count = 1;

  struct GNUNET_GNSRECORD_Data data [3];
  data[0].record_type = GNUNET_GNSRECORD_TYPE_MESSENGER_ROOM_ENTRY;
  data[0].data = &room;
  data[0].data_size = sizeof(room);
  data[0].expiration_time = expiration.abs_value_us;
  data[0].flags = GNUNET_GNSRECORD_RF_PRIVATE;

  if (nick)
  {
    data[count].record_type = GNUNET_GNSRECORD_TYPE_NICK;
    data[count].data = nick;
    data[count].data_size = strlen(nick);
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

  const char *type_string = "chat";

  switch (context->type)
  {
    case GNUNET_CHAT_CONTEXT_TYPE_CONTACT:
      type_string = "contact";
      break;
    case GNUNET_CHAT_CONTEXT_TYPE_GROUP:
      type_string = "group";
      break;
    default:
      break;
  }

  char *label;
  GNUNET_asprintf (
      &label,
      "%s_%s",
      type_string,
      GNUNET_h2s(hash)
  );

  GNUNET_NAMESTORE_records_store(
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

void
context_delete_records (struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((context) &&
  		(context->handle) &&
  		(context->room));

  const struct GNUNET_IDENTITY_PrivateKey *zone = handle_get_key(
      context->handle
  );

  if (!zone)
    return;

  const struct GNUNET_HashCode *hash = GNUNET_MESSENGER_room_get_key(
      context->room
  );

  const char *type_string = "chat";

  switch (context->type)
  {
    case GNUNET_CHAT_CONTEXT_TYPE_CONTACT:
      type_string = "contact";
      break;
    case GNUNET_CHAT_CONTEXT_TYPE_GROUP:
      type_string = "group";
      break;
    default:
      break;
  }

  char *label;
  GNUNET_asprintf (
      &label,
      "%s_%s",
      type_string,
      GNUNET_h2s(hash)
  );

  GNUNET_NAMESTORE_records_store(
      context->handle->namestore,
      zone,
      label,
      0,
      NULL,
      cont_context_write_records,
      context
  );

  GNUNET_free(label);
}
