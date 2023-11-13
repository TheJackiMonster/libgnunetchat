/*
   This file is part of GNUnet.
   Copyright (C) 2021--2023 GNUnet e.V.

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

  if (0 == strcmp(name, "gnunet_chat_handle_accounts"))
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
	  "gnunet_chat_handle_accounts"
      ), GNUNET_OK);

    accounts_stage = 4;
  }
  else if (4 == accounts_stage)
    GNUNET_CHAT_stop(accounts_handle);
  else if (0 == accounts_stage)
  {
    ck_assert_int_eq(GNUNET_CHAT_account_create(
	accounts_handle,
	"gnunet_chat_handle_accounts"
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

int
on_gnunet_chat_handle_connection_it(void *cls,
				    const struct GNUNET_CHAT_Handle *handle,
				    struct GNUNET_CHAT_Account *account)
{
  struct GNUNET_CHAT_Handle *chat = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_ne(chat, NULL);
  ck_assert_ptr_eq(handle, chat);
  ck_assert_ptr_ne(account, NULL);

  const char *name = GNUNET_CHAT_account_get_name(account);

  ck_assert_ptr_ne(name, NULL);
  ck_assert_ptr_eq(GNUNET_CHAT_get_connected(handle), NULL);

  if (0 == strcmp(name, "gnunet_chat_handle_connection"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

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

  if (GNUNET_CHAT_KIND_LOGIN == GNUNET_CHAT_message_get_kind(message))
    goto skip_iteration;

  GNUNET_CHAT_iterate_accounts(
      handle,
      on_gnunet_chat_handle_connection_it,
      handle
  );

skip_iteration:
  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

  GNUNET_CHAT_disconnect(handle);

  ck_assert_int_eq(GNUNET_CHAT_account_delete(
      handle,
      "gnunet_chat_handle_connection"
  ), GNUNET_OK);

  GNUNET_CHAT_stop(handle);
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
      "gnunet_chat_handle_connection"
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_connection, call_gnunet_chat_handle_connection)

int
on_gnunet_chat_handle_update_it(void *cls,
				const struct GNUNET_CHAT_Handle *handle,
				struct GNUNET_CHAT_Account *account)
{
  struct GNUNET_CHAT_Handle *chat = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_ne(chat, NULL);
  ck_assert_ptr_eq(handle, chat);
  ck_assert_ptr_ne(account, NULL);

  const char *name = GNUNET_CHAT_account_get_name(account);

  ck_assert_ptr_ne(name, NULL);
  ck_assert_ptr_eq(GNUNET_CHAT_get_connected(handle), NULL);

  if (0 == strcmp(name, "gnunet_chat_handle_update"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

int
on_gnunet_chat_handle_update_msg(void *cls,
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

  if (GNUNET_CHAT_get_connected(handle))
    goto skip_search_account;

  GNUNET_CHAT_iterate_accounts(
      handle,
      on_gnunet_chat_handle_update_it,
      handle
  );

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

skip_search_account:
  if (GNUNET_CHAT_KIND_LOGIN != kind)
    return GNUNET_YES;

  const char *key = GNUNET_CHAT_get_key(handle);
  ck_assert_ptr_ne(key, NULL);

  char *dup = (char*) GNUNET_CHAT_get_user_pointer(handle);

  if (!dup)
  {
    dup = GNUNET_strdup(key);

    ck_assert_ptr_ne(dup, NULL);
    ck_assert_str_eq(key, dup);

    GNUNET_CHAT_set_user_pointer(handle, (void*) dup);
    GNUNET_CHAT_update(handle);
  }
  else
  {
    ck_assert_ptr_ne(dup, NULL);
    ck_assert_str_ne(key, dup);

    GNUNET_free(dup);

    GNUNET_CHAT_disconnect(handle);

    ck_assert_int_eq(GNUNET_CHAT_account_delete(
	handle,
	"gnunet_chat_handle_update"
    ), GNUNET_OK);

    GNUNET_CHAT_stop(handle);
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
      "gnunet_chat_handle_update"
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_update, call_gnunet_chat_handle_update)

int
on_gnunet_chat_handle_rename_it(void *cls,
				const struct GNUNET_CHAT_Handle *handle,
				struct GNUNET_CHAT_Account *account)
{
  struct GNUNET_CHAT_Handle *chat = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_ne(chat, NULL);
  ck_assert_ptr_eq(handle, chat);
  ck_assert_ptr_ne(account, NULL);

  const char *name = GNUNET_CHAT_account_get_name(account);

  ck_assert_ptr_ne(name, NULL);
  ck_assert_ptr_eq(GNUNET_CHAT_get_connected(handle), NULL);

  if (0 == strcmp(name, "gnunet_chat_handle_rename_a"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

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

  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  if (GNUNET_CHAT_get_connected(handle))
    goto skip_search_account;

  GNUNET_CHAT_iterate_accounts(
      handle,
      on_gnunet_chat_handle_rename_it,
      handle
  );

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

skip_search_account:
  if (GNUNET_CHAT_KIND_LOGIN != kind)
    return GNUNET_YES;

  const char *name = GNUNET_CHAT_get_name(handle);
  ck_assert_ptr_ne(name, NULL);

  char *dup = (char*) GNUNET_CHAT_get_user_pointer(handle);

  if (!dup)
  {
    dup = GNUNET_strdup(name);

    ck_assert_ptr_ne(dup, NULL);
    ck_assert_str_eq(name, dup);

    GNUNET_CHAT_set_user_pointer(handle, (void*) dup);

    ck_assert_int_eq(GNUNET_CHAT_set_name(
	handle,
	"gnunet_chat_handle_rename_b"
    ), GNUNET_YES);
  }
  else if (0 != strcmp(name, dup))
  {
    ck_assert_ptr_ne(dup, NULL);
    ck_assert_str_ne(name, dup);

    GNUNET_free(dup);

    GNUNET_CHAT_disconnect(handle);

    ck_assert_int_eq(GNUNET_CHAT_account_delete(
	handle,
	"gnunet_chat_handle_rename_b"
    ), GNUNET_OK);

    GNUNET_CHAT_stop(handle);
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
      "gnunet_chat_handle_rename_a"
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
