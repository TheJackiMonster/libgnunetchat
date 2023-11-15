/*
   This file is part of GNUnet.
   Copyright (C) 2022--2023 GNUnet e.V.

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
 * @file gnunet_chat_uri.c
 */

#include "gnunet_chat_uri.h"

struct GNUNET_CHAT_Uri*
uri_create (const struct GNUNET_CRYPTO_PublicKey *zone,
	    const char *label)
{
  GNUNET_assert((zone) && (label));

  struct GNUNET_CHAT_Uri *uri = GNUNET_new(struct GNUNET_CHAT_Uri);

  GNUNET_memcpy(&(uri->zone), zone, sizeof(uri->zone));

  uri->label = GNUNET_strdup(label);

  return uri;
}

void
uri_destroy (struct GNUNET_CHAT_Uri *uri)
{
  GNUNET_assert(uri);

  if (uri->label)
    GNUNET_free(uri->label);

  GNUNET_free(uri);
}
