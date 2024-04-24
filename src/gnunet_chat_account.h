/*
   This file is part of GNUnet.
   Copyright (C) 2022--2024 GNUnet e.V.

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
 * @file gnunet_chat_account.h
 */

#ifndef GNUNET_CHAT_ACCOUNT_H_
#define GNUNET_CHAT_ACCOUNT_H_

#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_util_lib.h>

struct GNUNET_CHAT_Account
{
  struct GNUNET_IDENTITY_Ego *ego;
  char *directory;
  char *name;

  void *user_pointer;
};

/**
 * Creates a chat account using a given <i>ego</i> and
 * a matching <i>name</i>.
 *
 * @param[in] ego EGO
 * @param[in] name Name
 * @return New chat account
 */
struct GNUNET_CHAT_Account*
account_create_from_ego(struct GNUNET_IDENTITY_Ego *ego,
			                  const char *name);

/**
 * Updates the stored directory path by a chat <i>account</i>
 * using its current ego and key information.
 *
 * @param[in,out] account Chat account
 * @param[in] base_directory The base directory for the accounts
 */
void
account_update_directory (struct GNUNET_CHAT_Account *account,
			                    const char *base_directory);

/**
 * Returns the private key from a given chat
 * <i>account</i>.
 *
 * @param[in] account Chat account
 * @return EGOs private key or NULL
 */
const struct GNUNET_CRYPTO_PrivateKey*
account_get_key (const struct GNUNET_CHAT_Account *account);

/**
 * Destroys a chat <i>account</i> and frees its memory.
 *
 * @param[in,out] account Chat account
 */
void
account_destroy(struct GNUNET_CHAT_Account *account);

#endif /* GNUNET_CHAT_ACCOUNT_H_ */
