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
 * @file gnunet_chat_message.h
 */

#ifndef GNUNET_CHAT_MESSAGE_H_
#define GNUNET_CHAT_MESSAGE_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

struct GNUNET_CHAT_Context;
struct GNUNET_CHAT_Message;

struct GNUNET_CHAT_MessageList
{
  struct GNUNET_CHAT_Message *message;

  struct GNUNET_CHAT_MessageList *prev;
  struct GNUNET_CHAT_MessageList *next;
};

enum GNUNET_CHAT_MessageFlag
{
  GNUNET_CHAT_FLAG_NONE = 0,
  GNUNET_CHAT_FLAG_WARNING = 1,
  GNUNET_CHAT_FLAG_REFRESH = 2,
  GNUNET_CHAT_FLAG_LOGIN = 3,
  GNUNET_CHAT_FLAG_UPDATE = 4
};

struct GNUNET_CHAT_Message
{
  struct GNUNET_CHAT_Context *context;

  union {
    const struct GNUNET_MESSENGER_Message *msg;
    const char *warning;
  };

  struct GNUNET_HashCode hash;
  enum GNUNET_MESSENGER_MessageFlags flags;
  enum GNUNET_CHAT_MessageFlag flag;
};

/**
 * Creates a chat message representing an actual message
 * from the messenger service in a given chat <i>context</i>
 * with a valid <i>hash</i> and message <i>flags</i>
 *
 * @param[in,out] context Chat context
 * @param[in] hash Message hash
 * @param[in] flags Message flags
 * @param[in] msg Messenger message
 * @return New chat message
 */
struct GNUNET_CHAT_Message*
message_create_from_msg (struct GNUNET_CHAT_Context *context,
			 const struct GNUNET_HashCode *hash,
			 enum GNUNET_MESSENGER_MessageFlags flags,
			 const struct GNUNET_MESSENGER_Message *msg);

/**
 * Creates an internal chat message with an optional chat
 * <i>context</i>, a custom <i>flag</i> and an optional
 * <i>warning</i> text.
 *
 * @param[in,out] context Chat context or NULL
 * @param[in] flag Chat message flag
 * @param[in] warning Warning text
 * @return New internal chat message
 */
struct GNUNET_CHAT_Message*
message_create_internally (struct GNUNET_CHAT_Context *context,
			   enum GNUNET_CHAT_MessageFlag flag,
			   const char *warning);

/**
 * Destroys a chat <i>message</i> and frees its memory.
 *
 * @param[in,out] message Chat message
 */
void
message_destroy (struct GNUNET_CHAT_Message* message);

#endif /* GNUNET_CHAT_MESSAGE_H_ */
