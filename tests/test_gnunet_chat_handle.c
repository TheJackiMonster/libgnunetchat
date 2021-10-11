/*
   This file is part of GNUnet.
   Copyright (C) 2021 GNUnet e.V.

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

  handle = GNUNET_CHAT_start(cfg, "", "Init", NULL, NULL);
  ck_assert_ptr_ne(handle, NULL);

  GNUNET_CHAT_stop(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_init, call_gnunet_chat_handle_init)

struct GNUNET_CHAT_Handle *login_handle;

int
on_gnunet_chat_handle_login_msg(void *cls,
				struct GNUNET_CHAT_Context *context,
				const struct GNUNET_CHAT_Message *message)
{
  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);
  ck_assert_ptr_eq(cls, NULL);
  ck_assert_ptr_eq(context, NULL);

  GNUNET_CHAT_stop(login_handle);
  return GNUNET_YES;
}

void
call_gnunet_chat_handle_login(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  login_handle = GNUNET_CHAT_start(cfg, "", "Login", on_gnunet_chat_handle_login_msg, NULL);
  ck_assert_ptr_ne(login_handle, NULL);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_login, call_gnunet_chat_handle_login)

struct GNUNET_CHAT_Handle *access_handle;
int access_logins;

int
on_gnunet_chat_handle_access_msg(void *cls,
				struct GNUNET_CHAT_Context *context,
				const struct GNUNET_CHAT_Message *message)
{
  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  ck_assert_ptr_eq(cls, NULL);
  ck_assert_ptr_eq(context, NULL);

  const struct GNUNET_IDENTITY_PublicKey *key;
  const char *name;
  int result;

  if (access_logins == 0)
  {
    ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);

    key = GNUNET_CHAT_get_key(access_handle);
    ck_assert_ptr_eq(key, NULL);

    result = GNUNET_CHAT_update(access_handle);
    ck_assert_int_eq(result, GNUNET_OK);

    name = GNUNET_CHAT_get_name(access_handle);
    ck_assert_str_eq(name, "Access");
  }
  else if (access_logins == 1)
  {
    ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);

    key = GNUNET_CHAT_get_key(access_handle);
    ck_assert_ptr_ne(key, NULL);

    result = GNUNET_CHAT_set_name(access_handle, "Bccess");
    ck_assert_int_eq(result, GNUNET_YES);
  }
  else
  {
    ck_assert(kind == GNUNET_CHAT_KIND_CONTACT);

    ck_assert_int_eq(access_logins, 2);

    name = GNUNET_CHAT_get_name(access_handle);
    ck_assert_str_eq(name, "Bccess");

    GNUNET_CHAT_stop(access_handle);
  }

  access_logins++;
  return GNUNET_YES;
}

void
call_gnunet_chat_handle_access(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  access_logins = 0;

  access_handle = GNUNET_CHAT_start(cfg, "", "Access", on_gnunet_chat_handle_access_msg, NULL);
  ck_assert_ptr_ne(access_handle, NULL);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle_access, call_gnunet_chat_handle_access)


START_SUITE(handle_suite, "Handle")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_init, "Start/Stop")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_login, "Login")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle_access, "Get/Set")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
