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
 * @file gnunet_chat_uri.h
 */

#ifndef GNUNET_CHAT_URI_H_
#define GNUNET_CHAT_URI_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_util_lib.h>

#include "gnunet_chat_util.h"

struct GNUNET_CHAT_Uri
{
  struct GNUNET_IDENTITY_PublicKey zone;
  char *label;
};

/**
 * Creates a chat uri with a selected key as <i>zone</i>
 * and a <i>label</i>.
 *
 * @param[in] zone URI zone
 * @param[in] label URI label
 * @return New chat uri
 */
struct GNUNET_CHAT_Uri*
uri_create (const struct GNUNET_IDENTITY_PublicKey *zone,
	    const char *label);

/**
 * Destroys a chat <i>uri</i> and frees its memory.
 *
 * @param[in,out] uri Chat uri
 */
void
uri_destroy (struct GNUNET_CHAT_Uri *uri);

#endif /* GNUNET_CHAT_URI_H_ */
