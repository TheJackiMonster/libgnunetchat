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
 * @file gnunet_chat_invitation.c
 */

#include "gnunet_chat_invitation.h"

struct GNUNET_CHAT_Invitation*
invitation_create_from_message (struct GNUNET_CHAT_Context *context,
				const struct GNUNET_MESSENGER_MessageInvite *message)
{
  GNUNET_assert((context) && (message));

  struct GNUNET_CHAT_Invitation *invitation = GNUNET_new(struct GNUNET_CHAT_Invitation);

  invitation->context = context;

  GNUNET_memcpy(&(invitation->key), &(message->key), sizeof(invitation->key));
  invitation->door = GNUNET_PEER_intern(&(message->door));

  return invitation;
}

void
invitation_destroy (struct GNUNET_CHAT_Invitation *invitation)
{
  GNUNET_assert(invitation);

  GNUNET_PEER_decrement_rcs(&(invitation->door), 1);

  GNUNET_free(invitation);
}
