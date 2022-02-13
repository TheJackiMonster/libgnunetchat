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
  account->name = NULL;

  util_set_name_field(name, &(account->name));

  account->user_pointer = NULL;

  return account;
}

void
account_destroy(struct GNUNET_CHAT_Account *account)
{
  GNUNET_assert(account);

  if (account->name)
    GNUNET_free(account->name);

  GNUNET_free(account);
}
