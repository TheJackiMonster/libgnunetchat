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
 * @file gnunet_chat_contact.c
 */

#include "gnunet_chat_contact.h"
#include "gnunet_chat_context.h"
#include "gnunet_chat_handle.h"

#include "gnunet_chat_contact_intern.c"

struct GNUNET_CHAT_Contact*
contact_create_from_member (struct GNUNET_CHAT_Handle *handle,
			    const struct GNUNET_MESSENGER_Contact *member)
{
  GNUNET_assert((handle) && (member));

  struct GNUNET_CHAT_Contact* contact = GNUNET_new(struct GNUNET_CHAT_Contact);

  contact->handle = handle;
  contact->context = NULL;

  contact->member = member;

  contact->public_key = NULL;
  contact->user_pointer = NULL;

  contact->is_owned = GNUNET_NO;

  contact_update_key (contact);
  return contact;
}

void
contact_update_key (struct GNUNET_CHAT_Contact *contact)
{
  GNUNET_assert(contact);

  if (contact->public_key)
    GNUNET_free(contact->public_key);

  contact->public_key = NULL;

  if (!contact->member)
    return;

  const struct GNUNET_IDENTITY_PublicKey *pubkey;
  pubkey = GNUNET_MESSENGER_contact_get_key(contact->member);

  if (pubkey)
    contact->public_key = GNUNET_IDENTITY_public_key_to_string(pubkey);
}

struct GNUNET_CHAT_Context*
contact_find_context (struct GNUNET_CHAT_Contact *contact)
{
  GNUNET_assert(contact);

  if (contact->context)
    return contact->context;

  struct GNUNET_CHAT_ContactFindRoom find;
  find.member_count = 0;
  find.room = NULL;

  GNUNET_MESSENGER_find_rooms(
      contact->handle->messenger,
      contact->member,
      it_contact_find_room,
      &find
  );

  if (!(find.room))
    return NULL;

  return GNUNET_CONTAINER_multihashmap_get(
      contact->handle->contexts,
      GNUNET_MESSENGER_room_get_key(find.room)
  );
}

void
contact_destroy (struct GNUNET_CHAT_Contact* contact)
{
  GNUNET_assert(contact);

  if (contact->public_key)
    GNUNET_free(contact->public_key);

  GNUNET_free(contact);
}
