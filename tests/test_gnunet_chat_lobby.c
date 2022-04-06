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
 * @file test_gnunet_chat_lobby.c
 */

#include "test_gnunet_chat.h"

int
on_gnunet_chat_lobby_base_it(void *cls,
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

  if (0 == strcmp(name, "gnunet_chat_lobby_base"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

int
on_gnunet_chat_lobby_base_msg(void *cls,
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

  if ((kind != GNUNET_CHAT_KIND_REFRESH) ||
      (GNUNET_CHAT_get_connected(handle)))
    goto skip_search_account;

  ck_assert(kind == GNUNET_CHAT_KIND_REFRESH);

  GNUNET_CHAT_iterate_accounts(
      handle,
      on_gnunet_chat_lobby_base_it,
      handle
  );

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

skip_search_account:
  if (GNUNET_CHAT_KIND_LOGIN != kind)
    return GNUNET_YES;

  ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);

  struct GNUNET_CHAT_Lobby *lobby = GNUNET_CHAT_lobby_open(
      handle,
      GNUNET_TIME_relative_get_second_(),
      NULL,
      NULL
  );

  ck_assert_ptr_ne(lobby, NULL);

  GNUNET_CHAT_lobby_close(lobby);
  GNUNET_CHAT_disconnect(handle);

  ck_assert_int_eq(GNUNET_CHAT_account_delete(
    handle,
    "gnunet_chat_lobby_base"
  ), GNUNET_OK);

  GNUNET_CHAT_stop(handle);

  return GNUNET_YES;
}

void
call_gnunet_chat_lobby_base(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_lobby_base_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
      handle,
      "gnunet_chat_lobby_base"
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_lobby_base, call_gnunet_chat_lobby_base)

int
on_gnunet_chat_lobby_join_it(void *cls,
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

  if (0 == strcmp(name, "gnunet_chat_lobby_join"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

void
on_gnunet_chat_lobby_join_task(void *cls)
{
  struct GNUNET_CHAT_Handle *chat = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_ne(chat, NULL);

  GNUNET_CHAT_disconnect(chat);

  ck_assert_int_eq(GNUNET_CHAT_account_delete(
      chat,
      "gnunet_chat_lobby_join"
  ), GNUNET_OK);

  GNUNET_CHAT_stop(chat);
}

void
on_gnunet_chat_lobby_join_open(void *cls,
			       const struct GNUNET_CHAT_Uri *uri)
{
  struct GNUNET_CHAT_Handle *chat = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_ne(chat, NULL);
  ck_assert_ptr_ne(uri, NULL);

  GNUNET_CHAT_lobby_join(chat, uri);

  GNUNET_SCHEDULER_add_at_with_priority(
      GNUNET_TIME_absolute_get(),
      GNUNET_SCHEDULER_PRIORITY_IDLE,
      on_gnunet_chat_lobby_join_task,
      chat
  );
}

int
on_gnunet_chat_lobby_join_msg(void *cls,
			      struct GNUNET_CHAT_Context *context,
			      const struct GNUNET_CHAT_Message *message)
{
  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_ne(message, NULL);

  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);

  if ((kind != GNUNET_CHAT_KIND_REFRESH) ||
      (GNUNET_CHAT_get_connected(handle)))
    goto skip_search_account;

  ck_assert(kind == GNUNET_CHAT_KIND_REFRESH);
  ck_assert_ptr_eq(context, NULL);

  GNUNET_CHAT_iterate_accounts(
      handle,
      on_gnunet_chat_lobby_join_it,
      handle
  );

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

skip_search_account:
  if (GNUNET_CHAT_KIND_LOGIN != kind)
    return GNUNET_YES;

  ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);
  ck_assert_ptr_eq(context, NULL);

  struct GNUNET_CHAT_Lobby *lobby = GNUNET_CHAT_lobby_open(
      handle,
      GNUNET_TIME_relative_get_second_(),
      on_gnunet_chat_lobby_join_open,
      handle
  );

  ck_assert_ptr_ne(lobby, NULL);
  return GNUNET_YES;
}

void
call_gnunet_chat_lobby_join(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_lobby_join_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
      handle,
      "gnunet_chat_lobby_join"
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_lobby_join, call_gnunet_chat_lobby_join)

START_SUITE(lobby_suite, "Lobby")
ADD_TEST_TO_SUITE(test_gnunet_chat_lobby_base, "Open/Close")
ADD_TEST_TO_SUITE(test_gnunet_chat_lobby_join, "Join")
END_SUITE

MAIN_SUITE(lobby_suite, CK_NORMAL)
