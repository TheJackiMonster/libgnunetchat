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
 * @file gnunet_chat_context.h
 */

#ifndef GNUNET_CHAT_CONTEXT_H_
#define GNUNET_CHAT_CONTEXT_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_configuration_lib.h>
#include <gnunet/gnunet_container_lib.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

enum GNUNET_CHAT_ContextType
{
  GNUNET_CHAT_CONTEXT_TYPE_CONTACT = 1,
  GNUNET_CHAT_CONTEXT_TYPE_GROUP   = 2,
  GNUNET_CHAT_CONTEXT_TYPE_UNKNOWN = 0
};

struct GNUNET_CHAT_Handle;

struct GNUNET_CHAT_Context
{
  struct GNUNET_CHAT_Handle *handle;

  enum GNUNET_CHAT_ContextType type;
  char *nick;

  struct GNUNET_CONTAINER_MultiShortmap *timestamps;
  struct GNUNET_CONTAINER_MultiHashMap *messages;
  struct GNUNET_CONTAINER_MultiHashMap *invites;
  struct GNUNET_CONTAINER_MultiHashMap *files;

  struct GNUNET_MESSENGER_Room *room;
  const struct GNUNET_MESSENGER_Contact *contact;

  void *user_pointer;
};

struct GNUNET_CHAT_Context*
context_create_from_room (struct GNUNET_CHAT_Handle *handle,
			  struct GNUNET_MESSENGER_Room *room);

struct GNUNET_CHAT_Context*
context_create_from_contact (struct GNUNET_CHAT_Handle *handle,
			     const struct GNUNET_MESSENGER_Contact *contact);

void
context_destroy (struct GNUNET_CHAT_Context* context);

void
context_load_config (struct GNUNET_CHAT_Context *context);

void
context_save_config (const struct GNUNET_CHAT_Context *context);

#endif /* GNUNET_CHAT_CONTEXT_H_ */
