/*
   This file is part of GNUnet.
   Copyright (C) 2021--2024 GNUnet e.V.

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
 * @file test_gnunet_chat_handle_connection.c
 */

#include "test_gnunet_chat.h"

#define TEST_CONNECTION_ID "gnunet_chat_handle_connection"

enum GNUNET_GenericReturnValue
on_gnunet_chat_handle_connection_msg(void *cls,
                                     struct GNUNET_CHAT_Context *context,
                                     const struct GNUNET_CHAT_Message *message)
{
  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_nonnull(handle);
  ck_assert_ptr_null(context);
  ck_assert_ptr_nonnull(message);

  const struct GNUNET_CHAT_Account *connected;
  const struct GNUNET_CHAT_Account *account;
  const char *name;

  connected = GNUNET_CHAT_get_connected(handle);
  account = GNUNET_CHAT_message_get_account(message);

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_WARNING:
      ck_abort_msg("%s\n", GNUNET_CHAT_message_get_text(message));
      break;
    case GNUNET_CHAT_KIND_REFRESH:
      break;
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_nonnull(connected);
      ck_assert_ptr_nonnull(account);
      ck_assert_ptr_eq(connected, account);

      name = GNUNET_CHAT_account_get_name(account);

      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_CONNECTION_ID);
      
      GNUNET_CHAT_disconnect(handle);
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_ptr_nonnull(connected);
      ck_assert_ptr_nonnull(account);
      ck_assert_ptr_eq(connected, account);

      name = GNUNET_CHAT_account_get_name(account);

      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_CONNECTION_ID);

      ck_assert_int_eq(GNUNET_CHAT_account_delete(
        handle, TEST_CONNECTION_ID
      ), GNUNET_OK);
      break;
    case GNUNET_CHAT_KIND_CREATED_ACCOUNT:
      ck_assert_ptr_null(connected);
      ck_assert_ptr_nonnull(account);

      name = GNUNET_CHAT_account_get_name(account);

      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_CONNECTION_ID);

      GNUNET_CHAT_connect(handle, account);
      break;
    case GNUNET_CHAT_KIND_DELETED_ACCOUNT:
      ck_assert_ptr_null(connected);
      ck_assert_ptr_nonnull(account);

      name = GNUNET_CHAT_account_get_name(account);

      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_CONNECTION_ID);

      GNUNET_CHAT_stop(handle);
      break;
    default:
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_handle_connection(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_connection_msg, &handle);

  ck_assert_ptr_nonnull(handle);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
    handle, TEST_CONNECTION_ID
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_connection, call_gnunet_chat_handle_connection)

START_SUITE(handle_suite, "Handle")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_connection, "Connect/Disconnect")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)