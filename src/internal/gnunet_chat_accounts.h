/*
   This file is part of GNUnet.
   Copyright (C) 2024 GNUnet e.V.

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
 * @file gnunet_chat_accounts.h
 */

#ifndef GNUNET_CHAT_INTERNAL_ACCOUNTS_H_
#define GNUNET_CHAT_INTERNAL_ACCOUNTS_H_

#include <gnunet/gnunet_identity_service.h>

enum GNUNET_CHAT_AccountMethod
{
  GNUNET_CHAT_ACCOUNT_CREATION = 1,
  GNUNET_CHAT_ACCOUNT_DELETION = 2,
  GNUNET_CHAT_ACCOUNT_RENAMING = 3,
  GNUNET_CHAT_ACCOUNT_UPDATING = 4,

  GNUNET_CHAT_ACCOUNT_NONE = 0
};

struct GNUNET_CHAT_Handle;
struct GNUNET_CHAT_Account;

struct GNUNET_CHAT_InternalAccounts
{
  struct GNUNET_CHAT_Handle *handle;
  struct GNUNET_CHAT_Account *account;

  char *identifier;
  struct GNUNET_IDENTITY_Operation *op;
  enum GNUNET_CHAT_AccountMethod method;

  struct GNUNET_CHAT_InternalAccounts *next;
  struct GNUNET_CHAT_InternalAccounts *prev;
};

struct GNUNET_CHAT_InternalAccounts*
internal_accounts_create(struct GNUNET_CHAT_Handle *handle,
                         struct GNUNET_CHAT_Account *account);

void
internal_accounts_destroy(struct GNUNET_CHAT_InternalAccounts *accounts);

void
internal_accounts_start_method(struct GNUNET_CHAT_InternalAccounts *accounts,
                               enum GNUNET_CHAT_AccountMethod method,
                               const char *identifier);

void
internal_accounts_stop_method(struct GNUNET_CHAT_InternalAccounts *accounts);

#endif /* GNUNET_CHAT_ACCOUNTS_H_ */
