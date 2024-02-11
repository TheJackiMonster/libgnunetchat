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
 * @file gnunet_chat_contact.h
 */

#ifndef GNUNET_CHAT_CONTACT_H_
#define GNUNET_CHAT_CONTACT_H_

#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

struct GNUNET_CHAT_Handle;
struct GNUNET_CHAT_Context;
struct GNUNET_CHAT_Ticket;

struct GNUNET_CHAT_InternalTickets
{
  struct GNUNET_CHAT_Ticket *ticket;
  struct GNUNET_CHAT_InternalTickets *next;
  struct GNUNET_CHAT_InternalTickets *prev;
};

struct GNUNET_CHAT_Contact
{
  struct GNUNET_CHAT_Handle *handle;
  struct GNUNET_CHAT_Context *context;

  const struct GNUNET_MESSENGER_Contact *member;
  struct GNUNET_CONTAINER_MultiHashMap *joined;

  struct GNUNET_CHAT_InternalTickets *tickets_head;
  struct GNUNET_CHAT_InternalTickets *tickets_tail;

  char *public_key;
  void *user_pointer;

  enum GNUNET_GenericReturnValue owned;
};

/**
 * Creates a chat contact using a given messenger <i>contact</i>
 * with a selected chat <i>handle</i>.
 *
 * @param[in,out] handle Chat handle
 * @param[in] member Messenger contact
 * @return New chat contact
 */
struct GNUNET_CHAT_Contact*
contact_create_from_member (struct GNUNET_CHAT_Handle *handle,
			                      const struct GNUNET_MESSENGER_Contact *member);

/**
 * Updates the latest <i>hash</i> of a join message from a given
 * chat <i>contact</i> in a current chat <i>context</i>.
 *
 * @param[in,out] contact Chat contact
 * @param[in,out] context Chat context
 * @param[in] hash Join message hash
 * @param[in] flags Join message flags
 */
void
contact_update_join (struct GNUNET_CHAT_Contact *contact,
                     struct GNUNET_CHAT_Context *context,
                     const struct GNUNET_HashCode *hash,
                     enum GNUNET_MESSENGER_MessageFlags flags);

/**
 * Updates the string representation of the public key from
 * a given chat <i>contact</i>.
 *
 * @param[in,out] contact Chat contact
 */
void
contact_update_key (struct GNUNET_CHAT_Contact *contact);

/**
 * Searches for a chat context containing a given chat
 * <i>contact</i> and the least amount of other members.
 *
 * @param[in] contact Chat contact
 * @return Chat context or NULL
 */
struct GNUNET_CHAT_Context*
contact_find_context (const struct GNUNET_CHAT_Contact *contact);

/**
 * Returns whether a chat <i>contact</i> is blocked in
 * a given chat <i>context</i>.
 *
 * @param[in] contact Chat contact
 * @param[in] context Chat context or NULL (optional)
 * @return #GNUNET_YES if blocked, otherwise #GNUNET_NO
 */
enum GNUNET_GenericReturnValue
contact_is_blocked (const struct GNUNET_CHAT_Contact *contact,
                    const struct GNUNET_CHAT_Context *context);

/**
 * Unblocks a given chat <i>contact</i> in
 * a given chat <i>context</i>.
 *
 * @param[in,out] contact Chat contact
 * @param[in,out] context Chat context
 */
void
contact_unblock (struct GNUNET_CHAT_Contact *contact,
                 struct GNUNET_CHAT_Context *context);

/**
 * Blocks a given chat <i>contact</i> in
 * a given chat <i>context</i>.
 *
 * @param[in,out] contact Chat contact
 * @param[in,out] context Chat context
 */
void
contact_block (struct GNUNET_CHAT_Contact *contact,
               struct GNUNET_CHAT_Context *context);

/**
 * Destroys a chat <i>contact</i> and frees its memory.
 *
 * @param[in,out] contact Chat contact
 */
void
contact_destroy (struct GNUNET_CHAT_Contact* contact);

#endif /* GNUNET_CHAT_CONTACT_H_ */
