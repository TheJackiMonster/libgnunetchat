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
 * @file gnunet_chat_ticket_process.h
 */

#ifndef GNUNET_CHAT_INTERNAL_TICKET_PROCESS_H_
#define GNUNET_CHAT_INTERNAL_TICKET_PROCESS_H_

#include <gnunet/gnunet_reclaim_service.h>

#include "gnunet_chat_lib.h"

struct GNUNET_CHAT_Handle;
struct GNUNET_CHAT_Contact;

struct GNUNET_CHAT_TicketProcess
{
  struct GNUNET_CHAT_Handle *handle;
  struct GNUNET_CHAT_Contact *contact;

  struct GNUNET_RECLAIM_Ticket *ticket;
  char *name;

  GNUNET_CHAT_ContactAttributeCallback callback;
  void *closure;

  struct GNUNET_RECLAIM_TicketIterator *iter;
  struct GNUNET_RECLAIM_Operation *op;

  struct GNUNET_CHAT_TicketProcess *next;
  struct GNUNET_CHAT_TicketProcess *prev;
};

struct GNUNET_CHAT_TicketProcess*
internal_tickets_create(struct GNUNET_CHAT_Handle *handle,
                        struct GNUNET_CHAT_Contact *contact,
                        const char *name);

struct GNUNET_CHAT_TicketProcess*
internal_tickets_copy(const struct GNUNET_CHAT_TicketProcess* tickets,
                      const struct GNUNET_RECLAIM_Ticket *ticket);

void
internal_tickets_destroy(struct GNUNET_CHAT_TicketProcess *tickets);

void
internal_tickets_next_iter(struct GNUNET_CHAT_TicketProcess *tickets);

void
internal_tickets_stop_iter(struct GNUNET_CHAT_TicketProcess *tickets);

#endif /* GNUNET_CHAT_INTERNAL_TICKET_PROCESS_H_ */
