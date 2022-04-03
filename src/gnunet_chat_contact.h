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
 * @file gnunet_chat_contact.h
 */

#ifndef GNUNET_CHAT_CONTACT_H_
#define GNUNET_CHAT_CONTACT_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

struct GNUNET_CHAT_Handle;
struct GNUNET_CHAT_Context;

struct GNUNET_CHAT_Contact
{
  struct GNUNET_CHAT_Handle *handle;
  struct GNUNET_CHAT_Context *context;

  const struct GNUNET_MESSENGER_Contact *member;

  char *public_key;
  void *user_pointer;

  int is_owned;
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
 * Destroys a chat <i>contact</i> and frees its memory.
 *
 * @param[in,out] contact Chat contact
 */
void
contact_destroy (struct GNUNET_CHAT_Contact* contact);

#endif /* GNUNET_CHAT_CONTACT_H_ */
