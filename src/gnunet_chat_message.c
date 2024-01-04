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
 * @file gnunet_chat_message.c
 */

#include "gnunet_chat_message.h"

struct GNUNET_CHAT_Message*
message_create_from_msg (struct GNUNET_CHAT_Context *context,
                         const struct GNUNET_HashCode *hash,
                         enum GNUNET_MESSENGER_MessageFlags flags,
                         const struct GNUNET_MESSENGER_Message *msg)
{
  GNUNET_assert((context) && (hash) && (msg));

  struct GNUNET_CHAT_Message *message = GNUNET_new(struct GNUNET_CHAT_Message);

  message->context = context;
  message->task = NULL;

  GNUNET_memcpy(&(message->hash), hash, sizeof(message->hash));
  message->flags = flags;
  message->flag = GNUNET_CHAT_FLAG_NONE;

  message->msg = msg;

  return message;
}

struct GNUNET_CHAT_Message*
message_create_internally (struct GNUNET_CHAT_Context *context,
                           enum GNUNET_CHAT_MessageFlag flag,
                           const char *warning)
{
  struct GNUNET_CHAT_Message *message = GNUNET_new(struct GNUNET_CHAT_Message);

  message->context = context;
  message->task = NULL;

  memset(&(message->hash), 0, sizeof(message->hash));
  message->flags = GNUNET_MESSENGER_FLAG_PRIVATE;
  message->flag = flag;

  message->warning = warning;

  return message;
}

void
message_destroy (struct GNUNET_CHAT_Message* message)
{
  GNUNET_assert(message);

  if (message->task)
    GNUNET_SCHEDULER_cancel(message->task);

  GNUNET_free(message);
}
