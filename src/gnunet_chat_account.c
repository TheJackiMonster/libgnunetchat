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
 * @file gnunet_chat_account.c
 */

#include "gnunet_chat_account.h"
#include "gnunet_chat_util.h"

struct GNUNET_CHAT_Account*
account_create_from_ego(struct GNUNET_IDENTITY_Ego *ego,
			const char *name)
{
  GNUNET_assert((ego) && (name));

  struct GNUNET_CHAT_Account *account = GNUNET_new(struct GNUNET_CHAT_Account);

  account->ego = ego;
  account->directory = NULL;
  account->name = NULL;

  util_set_name_field(name, &(account->name));

  account->user_pointer = NULL;

  return account;
}

void
account_update_directory (struct GNUNET_CHAT_Account *account,
			  const char *base_directory)
{
  GNUNET_assert((account) && (base_directory));

  if (account->directory)
    GNUNET_free(account->directory);

  struct GNUNET_CRYPTO_PublicKey key;
  GNUNET_IDENTITY_ego_get_public_key(account->ego, &key);

  char *key_string = GNUNET_CRYPTO_public_key_to_string(&key);

  if (!key_string)
  {
    account->directory = NULL;
    return;
  }

  util_get_dirname(base_directory, key_string, &(account->directory));
  GNUNET_free(key_string);
}

void
account_destroy(struct GNUNET_CHAT_Account *account)
{
  GNUNET_assert(account);

  if (account->name)
    GNUNET_free(account->name);

  if (account->directory)
    GNUNET_free(account->directory);

  GNUNET_free(account);
}
