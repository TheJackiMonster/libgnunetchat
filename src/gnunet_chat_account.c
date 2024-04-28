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
 * @file gnunet_chat_account.c
 */

#include "gnunet_chat_account.h"
#include "gnunet_chat_handle.h"
#include "gnunet_chat_util.h"

#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_messenger_service.h>

struct GNUNET_CHAT_Account*
account_create(const char *name)
{
  GNUNET_assert(name);

  struct GNUNET_CHAT_Account *account = GNUNET_new(struct GNUNET_CHAT_Account);

  account->ego = NULL;
  account->directory = NULL;
  account->name = NULL;

  util_set_name_field(name, &(account->name));

  account->user_pointer = NULL;

  return account;
}

struct GNUNET_CHAT_Account*
account_create_from_ego(struct GNUNET_IDENTITY_Ego *ego,
			                  const char *name)
{
  GNUNET_assert((ego) && (name));

  struct GNUNET_CHAT_Account *account = account_create(name);
  
  account->ego = ego;

  return account;
}

void
account_update_directory (struct GNUNET_CHAT_Account *account,
			                    const char *base_directory)
{
  GNUNET_assert((account) && (base_directory));

  if (account->directory)
    GNUNET_free(account->directory);

  if (!(account->ego))
  {
    account->directory = NULL;
    return;
  }

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

const struct GNUNET_CRYPTO_PrivateKey*
account_get_key (const struct GNUNET_CHAT_Account *account)
{
  GNUNET_assert(account);

  if (!(account->ego))
    return NULL;

  return GNUNET_IDENTITY_ego_get_private_key(
    account->ego
  );
}

void
account_update_ego(struct GNUNET_CHAT_Account *account,
                   struct GNUNET_CHAT_Handle *handle,
                   struct GNUNET_IDENTITY_Ego *ego)
{
  GNUNET_assert((account) && (handle) && (ego) && (account->ego != ego));

  enum GNUNET_CHAT_MessageFlag flag;
  if ((!(account->ego)) && (handle->current != account))
    flag = GNUNET_CHAT_FLAG_CREATE_ACCOUNT;
  else
    flag = GNUNET_CHAT_FLAG_UPDATE_ACCOUNT;

  account->ego = ego;

  if (handle->directory)
    account_update_directory(account, handle->directory);

  if ((handle->current == account) && (handle->messenger))
  {
    GNUNET_MESSENGER_set_key(
      handle->messenger,
      GNUNET_IDENTITY_ego_get_private_key(account->ego)
    );

    handle_update_key(handle);
  }

  handle_send_internal_message(
    handle,
    account,
    NULL,
    flag,
    NULL
  );
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
