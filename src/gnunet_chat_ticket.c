/*
   This file is part of GNUnet.
   Copyright (C) 2024 GNUnet e.V.

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
 * @file gnunet_chat_ticket.c
 */

#include "gnunet_chat_ticket.h"

#include "gnunet_chat_handle.h"
#include <gnunet/gnunet_messenger_service.h>

struct GNUNET_CHAT_Ticket*
ticket_create_from_message (struct GNUNET_CHAT_Handle *handle,
                            const struct GNUNET_MESSENGER_Contact *issuer,
                            const struct GNUNET_MESSENGER_MessageTicket *message)
{
  GNUNET_assert((handle) && (issuer) && (message));

  const struct GNUNET_CRYPTO_PublicKey *identity;
  const struct GNUNET_CRYPTO_PublicKey *audience;

  identity = GNUNET_MESSENGER_contact_get_key(issuer);
  audience = GNUNET_MESSENGER_get_key(handle->messenger);

  if ((!identity) || (!audience))
    return NULL;

  struct GNUNET_CHAT_Ticket *ticket = GNUNET_new(struct GNUNET_CHAT_Ticket);

  ticket->handle = handle;
  ticket->issuer = issuer;

  GNUNET_memcpy(&(ticket->ticket.identity), identity, sizeof(ticket->ticket.identity));
  GNUNET_memcpy(&(ticket->ticket.audience), audience, sizeof(ticket->ticket.audience));
  GNUNET_memcpy(&(ticket->ticket.rnd), &(message->identifier), sizeof(ticket->ticket.rnd));

  return ticket;
}

void
ticket_destroy (struct GNUNET_CHAT_Ticket *ticket)
{
  GNUNET_assert(ticket);

  GNUNET_free(ticket);
}
