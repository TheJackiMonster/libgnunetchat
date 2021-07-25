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
 * @file gnunet_chat_message.c
 */

#include "gnunet_chat_message.h"

struct GNUNET_CHAT_Message*
message_create_from_msg (struct GNUNET_CHAT_Context *context,
			 const struct GNUNET_HashCode *hash,
			 enum GNUNET_MESSENGER_MessageFlags flags,
			 const struct GNUNET_MESSENGER_Message *msg)
{
  struct GNUNET_CHAT_Message *message = GNUNET_new(struct GNUNET_CHAT_Message);

  message->context = context;

  GNUNET_memcpy(&(message->hash), hash, sizeof(message->hash));
  message->flags = flags;

  message->msg = msg;

  return message;
}

void
message_destroy (struct GNUNET_CHAT_Message* message)
{
  GNUNET_free(message);
}
