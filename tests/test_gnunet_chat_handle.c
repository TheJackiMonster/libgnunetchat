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
 * @file test_gnunet_chat_handle.c
 */

#include "test_gnunet_chat.h"

void
call_gnunet_chat_handle_init(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  struct GNUNET_CHAT_Handle *handle;

  handle = GNUNET_CHAT_start(cfg, NULL, NULL);
  ck_assert_ptr_ne(handle, NULL);

  GNUNET_CHAT_stop(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_init, call_gnunet_chat_handle_init)

struct GNUNET_CHAT_Handle *accounts_handle;
int accounts_stage;

int
on_gnunet_chat_handle_accounts_it(__attribute__ ((unused)) void *cls,
				  __attribute__ ((unused)) const struct GNUNET_CHAT_Handle *handle,
				  struct GNUNET_CHAT_Account *account)
{
  const char *name = GNUNET_CHAT_account_get_name(account);

  ck_assert_ptr_ne(name, NULL);

  if (0 == strcmp(name, GNUNET_CHAT_TEST_ACCOUNT))
    accounts_stage |= 2;

  return GNUNET_YES;
}

int
on_gnunet_chat_handle_accounts_msg(void *cls,
				   struct GNUNET_CHAT_Context *context,
				   const struct GNUNET_CHAT_Message *message)
{
  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  ck_assert(kind == GNUNET_CHAT_KIND_REFRESH);
  ck_assert_ptr_eq(cls, NULL);
  ck_assert_ptr_eq(context, NULL);

  GNUNET_CHAT_iterate_accounts(
      accounts_handle,
      on_gnunet_chat_handle_accounts_it,
      NULL
  );

  if (2 & accounts_stage)
  {
    if (3 == accounts_stage)
      ck_assert_int_eq(GNUNET_CHAT_account_delete(
	  accounts_handle,
	  GNUNET_CHAT_TEST_ACCOUNT
      ), GNUNET_OK);

    accounts_stage = 4;
  }
  else if (4 == accounts_stage)
    GNUNET_CHAT_stop(accounts_handle);
  else if (0 == accounts_stage)
  {
    ck_assert_int_eq(GNUNET_CHAT_account_create(
	accounts_handle,
	GNUNET_CHAT_TEST_ACCOUNT
    ), GNUNET_OK);

    accounts_stage = 1;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_handle_accounts(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  accounts_handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_accounts_msg, NULL);
  accounts_stage = 0;

  ck_assert_ptr_ne(accounts_handle, NULL);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_accounts, call_gnunet_chat_handle_accounts)

struct GNUNET_CHAT_Handle *connection_handle;

int
on_gnunet_chat_handle_connection_msg(void *cls,
				     struct GNUNET_CHAT_Context *context,
				     const struct GNUNET_CHAT_Message *message)
{
  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);
  ck_assert_ptr_eq(cls, NULL);
  ck_assert_ptr_eq(context, NULL);

  GNUNET_CHAT_stop(connection_handle);
  return GNUNET_YES;
}

void
call_gnunet_chat_handle_connection(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  connection_handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_connection_msg, NULL);
  ck_assert_ptr_ne(connection_handle, NULL);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_connection, call_gnunet_chat_handle_connection)

struct GNUNET_CHAT_Handle *update_handle;

int
on_gnunet_chat_handle_update_msg(void *cls,
				 struct GNUNET_CHAT_Context *context,
				 const struct GNUNET_CHAT_Message *message)
{
  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  ck_assert_ptr_eq(cls, NULL);
  ck_assert_ptr_eq(context, NULL);

  GNUNET_CHAT_stop(update_handle);
  return GNUNET_YES;
}

void
call_gnunet_chat_handle_update(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  update_handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_update_msg, NULL);
  ck_assert_ptr_ne(update_handle, NULL);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_update, call_gnunet_chat_handle_update)


START_SUITE(handle_suite, "Handle")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_init, "Start/Stop")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_accounts, "Accounts")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_connection, "Connect/Disconnect")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_update, "Update")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
