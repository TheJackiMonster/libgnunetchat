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
 * @file gnunet_chat_contact.h
 */

#ifndef GNUNET_CHAT_CONTACT_H_
#define GNUNET_CHAT_CONTACT_H_

#include "gnunet_chat_context.h"

struct GNUNET_CHAT_Contact
{
  struct GNUNET_CHAT_Handle *handle;
  struct GNUNET_CHAT_Context *context;

  const struct GNUNET_MESSENGER_Contact *contact;
};

struct GNUNET_CHAT_Contact*
contact_create (struct GNUNET_CHAT_Handle *handle,
		const struct GNUNET_MESSENGER_Contact *msg_contact);

void
contact_destroy (struct GNUNET_CHAT_Contact *contact);

int
contact_call (struct GNUNET_CHAT_Handle *handle,
	      const struct GNUNET_MESSENGER_Contact *msg_contact,
	      GNUNET_CHAT_ContactCallback callback,
	      void *cls);

#endif /* GNUNET_CHAT_CONTACT_H_ */
