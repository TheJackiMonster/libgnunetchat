/*
   This file is part of GNUnet.
   Copyright (C) 2022 GNUnet e.V.

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
 * @file gnunet_chat_lobby_intern.c
 */

#include "gnunet_chat_context.h"

void
cont_lobby_write_records (void *cls,
			  GNUNET_UNUSED int32_t success,
                          const char *emsg)
{
  struct GNUNET_CHAT_Lobby *lobby = cls;

  if (!emsg)
    goto call_cb;

  handle_send_internal_message(
      lobby->handle,
      lobby->context,
      GNUNET_CHAT_FLAG_WARNING,
      emsg
  );

  if (lobby->uri)
    uri_destroy(lobby->uri);

  lobby->uri = NULL;

call_cb:
  if (lobby->callback)
    lobby->callback(lobby->cls, lobby->uri);
}

void
cont_lobby_identity_delete (void *cls,
                            const char *emsg)
{
  struct GNUNET_CHAT_Lobby *lobby = cls;

  if (!emsg)
    return;

  handle_send_internal_message(
      lobby->handle,
      lobby->context,
      GNUNET_CHAT_FLAG_WARNING,
      emsg
  );
}

void
cont_lobby_identity_create (void *cls,
			    const struct GNUNET_IDENTITY_PrivateKey *zone,
			    const char *emsg)
{
  struct GNUNET_CHAT_Lobby *lobby = cls;

  char *name;

  if (emsg)
  {
    handle_send_internal_message(
	lobby->handle,
    	NULL,
    	GNUNET_CHAT_FLAG_WARNING,
    	emsg
    );

    return;
  }

  struct GNUNET_MESSENGER_Room *room = GNUNET_MESSENGER_open_room(
      lobby->handle->messenger,
      NULL
  );

  if (!room)
    goto destroy_identity;

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(room);

  lobby->context = context_create_from_room(lobby->handle, room);

  handle_send_room_name(lobby->handle, room);

  if (GNUNET_OK != GNUNET_CONTAINER_multihashmap_put(
      lobby->handle->contexts, key, lobby->context,
      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST))
  {
    context_destroy(lobby->context);
    lobby->context = NULL;
    goto destroy_identity;
  }

  struct GNUNET_GNSRECORD_Data data [3];
  data[0].record_type = GNUNET_GNSRECORD_TYPE_MESSENGER_ROOM_ENTRY;
  data[0].data = &room;
  data[0].data_size = sizeof(room);
  data[0].expiration_time = lobby->expiration.abs_value_us;
  data[0].flags = GNUNET_GNSRECORD_RF_NONE;

  if (lobby->uri)
    uri_destroy(lobby->uri);

  struct GNUNET_IDENTITY_PublicKey public_zone;
  GNUNET_IDENTITY_key_get_public(zone, &public_zone);

  char *label;
  util_get_context_label(lobby->context->type, key, &label);

  lobby->uri = uri_create(&public_zone, label);
  GNUNET_free(label);

  GNUNET_NAMESTORE_records_store(
      lobby->handle->namestore,
      zone,
      lobby->uri->label,
      1,
      data,
      cont_lobby_write_records,
      lobby
  );

  context_write_records(lobby->context);

destroy_identity:
  util_lobby_name(key, &name);

  lobby->op_delete = GNUNET_IDENTITY_delete(
      lobby->handle->identity,
      name,
      cont_lobby_identity_delete,
      lobby
  );

  GNUNET_free(name);
}
