/*
   This file is part of GNUnet.
   Copyright (C) 2022 GNUnet e.V.

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
 * @file gnunet_chat_lobby.h
 */

#ifndef GNUNET_CHAT_LOBBY_H_
#define GNUNET_CHAT_LOBBY_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_util_lib.h>

#include "gnunet_chat_lib.h"

struct GNUNET_CHAT_Handle;
struct GNUNET_CHAT_Context;
struct GNUNET_CHAT_Uri;

struct GNUNET_CHAT_Lobby
{
  struct GNUNET_CHAT_Handle *handle;

  struct GNUNET_CHAT_Context *context;
  struct GNUNET_CHAT_Uri *uri;

  struct GNUNET_IDENTITY_Operation *op_create;
  struct GNUNET_IDENTITY_Operation *op_delete;

  struct GNUNET_TIME_Absolute expiration;
  GNUNET_CHAT_LobbyCallback callback;
  void *cls;
};

struct GNUNET_CHAT_Lobby*
lobby_create (struct GNUNET_CHAT_Handle *handle);

void
lobby_destroy (struct GNUNET_CHAT_Lobby *lobby);

void
lobby_open (struct GNUNET_CHAT_Lobby *lobby,
	    struct GNUNET_TIME_Relative delay,
	    GNUNET_CHAT_LobbyCallback callback,
	    void *cls);

#endif /* GNUNET_CHAT_LOBBY_H_ */
