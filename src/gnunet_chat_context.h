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
 * @file gnunet_chat_context.h
 */

#ifndef GNUNET_CHAT_CONTEXT_H_
#define GNUNET_CHAT_CONTEXT_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_configuration_lib.h>
#include <gnunet/gnunet_gnsrecord_lib.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

#include "gnunet_chat_util.h"

struct GNUNET_CHAT_Handle;

struct GNUNET_CHAT_Context
{
  struct GNUNET_CHAT_Handle *handle;

  enum GNUNET_CHAT_ContextType type;
  char nick [256 + 1];
  char *topic;
  int deleted;

  struct GNUNET_CONTAINER_MultiShortmap *timestamps;
  struct GNUNET_CONTAINER_MultiHashMap *messages;
  struct GNUNET_CONTAINER_MultiHashMap *invites;
  struct GNUNET_CONTAINER_MultiHashMap *files;

  struct GNUNET_MESSENGER_Room *room;
  const struct GNUNET_MESSENGER_Contact *contact;

  void *user_pointer;

  struct GNUNET_CONTAINER_MultiShortmap *member_pointers;

  struct GNUNET_NAMESTORE_QueueEntry *query;
};

/**
 * Creates a chat context from a messenger <i>room</i>
 * with a selected chat <i>handle</i>.
 *
 * @param[in,out] handle Chat handle
 * @param[in,out] room Messenger room
 * @return New chat context
 */
struct GNUNET_CHAT_Context*
context_create_from_room (struct GNUNET_CHAT_Handle *handle,
			  struct GNUNET_MESSENGER_Room *room);

/**
 * Creates a chat context from a messenger <i>contact</i>
 * with a selected chat <i>handle</i>.
 *
 * @param[in,out] handle Chat handle
 * @param[in] contact Messenger contact
 * @return New chat context
 */
struct GNUNET_CHAT_Context*
context_create_from_contact (struct GNUNET_CHAT_Handle *handle,
			     const struct GNUNET_MESSENGER_Contact *contact);

/**
 * Destroys a chat <i>context</i> and frees its memory.
 *
 * @param[in,out] context Chat context
 */
void
context_destroy (struct GNUNET_CHAT_Context* context);

/**
 * Updates the connected messenger <i>room</i> of a
 * selected chat <i>context</i>.
 *
 * @param[in,out] context Chat context
 * @param[in,out] room Messenger room
 */
void
context_update_room (struct GNUNET_CHAT_Context *context,
		     struct GNUNET_MESSENGER_Room *room);

/**
 * Updates the <i>nick</i> of a selected chat <i>context</i>.
 *
 * @param[in,out] context Chat context
 * @param[in] nick Nick name
 */
void
context_update_nick (struct GNUNET_CHAT_Context *context,
		     const char *nick);

/**
 * Reads the <i>data</i> of records under a given <i>label</i>
 * and updates the chat <i>context</i> with it.
 *
 * @param[in,out] context Chat context
 * @param[in] label Label
 * @param[in] count Count of data
 * @param[in] data Records data
 */
void
context_read_records (struct GNUNET_CHAT_Context *context,
		      const char *label,
		      unsigned int count,
		      const struct GNUNET_GNSRECORD_Data *data);

/**
 * Writes the data from a selected chat <i>context</i> into
 * the namestore as private records.
 *
 * @param[in,out] context Chat context
 */
void
context_write_records (struct GNUNET_CHAT_Context *context);

#endif /* GNUNET_CHAT_CONTEXT_H_ */
