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
 * @file test_gnunet_chat_handle.c
 */

#include "test_gnunet_chat.h"
#include <check.h>
#include <gnunet/gnunet_chat_lib.h>
#include <string.h>

#define TEST_ACCOUNTS_ID   "gnunet_chat_handle_accounts"
#define TEST_CONNECTION_ID "gnunet_chat_handle_connection"
#define TEST_UPDATE_ID     "gnunet_chat_handle_update"
#define TEST_RENAME_ID_A   "gnunet_chat_handle_rename_a"
#define TEST_RENAME_ID_B   "gnunet_chat_handle_rename_b"

void
call_gnunet_chat_handle_init(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  struct GNUNET_CHAT_Handle *handle;

  handle = GNUNET_CHAT_start(cfg, NULL, NULL);
  ck_assert_ptr_ne(handle, NULL);

  GNUNET_CHAT_stop(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_init, call_gnunet_chat_handle_init)

int
on_gnunet_chat_handle_accounts_it(void *cls,
                                  const struct GNUNET_CHAT_Handle *handle,
                                  struct GNUNET_CHAT_Account *account)
{
  unsigned int *accounts_stage = cls;

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_ne(account, NULL);
  ck_assert_int_eq(*accounts_stage, 2);

  const char *name = GNUNET_CHAT_account_get_name(account);

  ck_assert_ptr_ne(name, NULL);

  if (0 == strcmp(name, TEST_ACCOUNTS_ID))
  {
    *accounts_stage = 3;
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

enum GNUNET_GenericReturnValue
on_gnunet_chat_handle_accounts_msg(void *cls,
                                   struct GNUNET_CHAT_Context *context,
                                   const struct GNUNET_CHAT_Message *message)
{
  static unsigned int accounts_stage = 0;

  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_eq(context, NULL);

  const struct GNUNET_CHAT_Account *account;
  account = GNUNET_CHAT_message_get_account(message);

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_REFRESH:
      if (0 == accounts_stage)
      {
        ck_assert_int_eq(GNUNET_CHAT_account_create(
          handle,
          TEST_ACCOUNTS_ID
        ), GNUNET_OK);

        accounts_stage = 1;
      } else
      if (2 == accounts_stage)
      {
        ck_assert_int_ge(GNUNET_CHAT_iterate_accounts(
          handle,
          on_gnunet_chat_handle_accounts_it,
          &accounts_stage
        ), 1);
      }

      if (3 == accounts_stage)
      {
        ck_assert_int_eq(GNUNET_CHAT_account_delete(
          handle,
          TEST_ACCOUNTS_ID
        ), GNUNET_OK);

        accounts_stage = 4;
      }

      break;
    case GNUNET_CHAT_KIND_CREATED_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);

      if (0 == strcmp(GNUNET_CHAT_account_get_name(account),
                      TEST_ACCOUNTS_ID))
        accounts_stage = 2;
      
      break;
    case GNUNET_CHAT_KIND_DELETED_ACCOUNT:
      ck_assert_int_eq(accounts_stage, 4);

      if (0 == strcmp(GNUNET_CHAT_account_get_name(account),
                      TEST_ACCOUNTS_ID))
        GNUNET_CHAT_stop(handle);
      
      break;
    default:
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_handle_accounts(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_accounts_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_accounts, call_gnunet_chat_handle_accounts)

int
on_gnunet_chat_handle_connection_msg(void *cls,
                                     struct GNUNET_CHAT_Context *context,
                                     const struct GNUNET_CHAT_Message *message)
{
  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_eq(context, NULL);
  ck_assert_ptr_ne(message, NULL);

  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  if (GNUNET_CHAT_KIND_LOGOUT == kind)
  {
    ck_assert_int_eq(GNUNET_CHAT_account_delete(
      handle,
      TEST_CONNECTION_ID
    ), GNUNET_OK);

    GNUNET_CHAT_stop(handle);
    return GNUNET_YES;
  }

  if (GNUNET_CHAT_KIND_CREATED_ACCOUNT == kind)
  {
    const struct GNUNET_CHAT_Account *account;
    account = GNUNET_CHAT_message_get_account(message);

    ck_assert_ptr_ne(account, NULL);

    if (0 == strcmp(GNUNET_CHAT_account_get_name(account),
                    TEST_CONNECTION_ID))
      GNUNET_CHAT_connect(handle, account);
  }

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

  GNUNET_CHAT_disconnect(handle);
  return GNUNET_YES;
}

void
call_gnunet_chat_handle_connection(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_connection_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
    handle,
    TEST_CONNECTION_ID
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_connection, call_gnunet_chat_handle_connection)

int
on_gnunet_chat_handle_update_msg(void *cls,
                                 struct GNUNET_CHAT_Context *context,
                                 const struct GNUNET_CHAT_Message *message)
{
  static unsigned int update_stage = 0;

  struct GNUNET_CHAT_Handle *handle = *(
    (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_eq(context, NULL);
  ck_assert_ptr_ne(message, NULL);

  const struct GNUNET_CHAT_Account *account;
  account = GNUNET_CHAT_message_get_account(message);

  const char *key;
  char *dup;

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_REFRESH:
      break;
    case GNUNET_CHAT_KIND_LOGIN:
      account = GNUNET_CHAT_get_connected(handle);

      ck_assert_ptr_ne(account, NULL);
      ck_assert_str_eq(
        GNUNET_CHAT_account_get_name(account),
        TEST_UPDATE_ID
      );

      key = GNUNET_CHAT_get_key(handle);
      ck_assert_ptr_ne(key, NULL);

      dup = (char*) GNUNET_CHAT_get_user_pointer(handle);

      ck_assert_int_eq(update_stage, 1);

      dup = GNUNET_strdup(key);

      ck_assert_ptr_ne(dup, NULL);
      ck_assert_str_eq(key, dup);

      GNUNET_CHAT_set_user_pointer(handle, (void*) dup);
      GNUNET_CHAT_update(handle);

      update_stage = 2;
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      account = GNUNET_CHAT_get_connected(handle);

      ck_assert_int_ge(update_stage, 2);
      ck_assert_int_le(update_stage, 3);

      ck_assert_ptr_ne(account, NULL);
      ck_assert_str_eq(
        GNUNET_CHAT_account_get_name(account),
        TEST_UPDATE_ID
      );

      if (update_stage == 3)
      {
        ck_assert_int_eq(GNUNET_CHAT_account_delete(
          handle,
          TEST_UPDATE_ID
        ), GNUNET_OK);

        update_stage = 4;
      }

      break;
    case GNUNET_CHAT_KIND_CREATED_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);

      if ((0 != strcmp(GNUNET_CHAT_account_get_name(account),
                       TEST_UPDATE_ID)))
        break;

      ck_assert_int_eq(update_stage, 0);

      update_stage = 1;
      GNUNET_CHAT_connect(handle, account);
      break;
    case GNUNET_CHAT_KIND_DELETED_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);

      if ((0 != strcmp(GNUNET_CHAT_account_get_name(account),
                       TEST_UPDATE_ID)))
        break;
      
      ck_assert_int_eq(update_stage, 4);

      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_UPDATE_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);

      if ((0 != strcmp(GNUNET_CHAT_account_get_name(account),
                       TEST_UPDATE_ID)))
        break;
      
      key = GNUNET_CHAT_get_key(handle);
      ck_assert_ptr_ne(key, NULL);

      dup = (char*) GNUNET_CHAT_get_user_pointer(handle);

      ck_assert_int_eq(update_stage, 2);
      ck_assert_ptr_ne(dup, NULL);
      ck_assert_str_ne(key, dup);

      GNUNET_free(dup);

      GNUNET_CHAT_disconnect(handle);

      update_stage = 3;
      break;
    default:
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_handle_update(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_update_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
    handle,
    TEST_UPDATE_ID
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_update, call_gnunet_chat_handle_update)

int
on_gnunet_chat_handle_rename_msg(void *cls,
                                 struct GNUNET_CHAT_Context *context,
                                 const struct GNUNET_CHAT_Message *message)
{
  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_eq(context, NULL);
  ck_assert_ptr_ne(message, NULL);

  const struct GNUNET_CHAT_Account *account;
  account = GNUNET_CHAT_message_get_account(message);

  const char *name = GNUNET_CHAT_get_name(handle);
  char *dup = (char*) GNUNET_CHAT_get_user_pointer(handle);

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_REFRESH:
      break;
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_ne(account, NULL);
      ck_assert_ptr_ne(name, NULL);
      ck_assert_ptr_eq(dup, NULL);
      ck_assert_str_eq(name, TEST_RENAME_ID_A);

      dup = GNUNET_strdup(name);

      ck_assert_ptr_ne(dup, NULL);
      ck_assert_str_eq(name, dup);

      GNUNET_CHAT_set_user_pointer(handle, (void*) dup);

      ck_assert_int_eq(GNUNET_CHAT_set_name(
        handle,
        TEST_RENAME_ID_B
      ), GNUNET_YES);
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_ptr_ne(account, NULL);
      ck_assert_int_eq(GNUNET_CHAT_account_delete(
        handle,
        TEST_RENAME_ID_B
      ), GNUNET_OK);
      break;
    case GNUNET_CHAT_KIND_CREATED_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);

      GNUNET_CHAT_connect(handle, account);
      break;
    case GNUNET_CHAT_KIND_DELETED_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);

      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_UPDATE_ACCOUNT:
      ck_assert_ptr_ne(account, NULL);
      ck_assert_ptr_ne(name, NULL);
      ck_assert_ptr_ne(dup, NULL);
      ck_assert_str_ne(name, dup);
      ck_assert_str_eq(name, TEST_RENAME_ID_B);
      ck_assert_str_eq(dup, TEST_RENAME_ID_A);

      GNUNET_free(dup);

      GNUNET_CHAT_disconnect(handle);
      break;
    default:
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_handle_rename(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_handle_rename_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
    handle,
    TEST_RENAME_ID_A
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_rename, call_gnunet_chat_handle_rename)

START_SUITE(handle_suite, "Handle")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_init, "Start/Stop")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_accounts, "Accounts")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_connection, "Connect/Disconnect")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_update, "Update")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_rename, "Rename")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
