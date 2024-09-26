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
 * @file test_gnunet_chat_lobby_join.c
 */

#include "test_gnunet_chat.h"

#define TEST_JOIN_ID "gnunet_chat_lobby_join"

void
on_gnunet_chat_lobby_join_open(void *cls,
                               const struct GNUNET_CHAT_Uri *uri)
{
  struct GNUNET_CHAT_Handle *chat = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_nonnull(chat);
  ck_assert_ptr_nonnull(uri);

  GNUNET_CHAT_lobby_join(chat, uri);
}

enum GNUNET_GenericReturnValue
on_gnunet_chat_lobby_join_msg(void *cls,
                              struct GNUNET_CHAT_Context *context,
                              struct GNUNET_CHAT_Message *message)
{
  static struct GNUNET_CHAT_Lobby *lobby = NULL;

  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_nonnull(handle);
  ck_assert_ptr_nonnull(message);

  struct GNUNET_CHAT_Account *account;
  account = GNUNET_CHAT_message_get_account(message);

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_WARNING:
      ck_abort_msg("%s\n", GNUNET_CHAT_message_get_text(message));
      break;
    case GNUNET_CHAT_KIND_REFRESH:
      ck_assert_ptr_null(context);
      ck_assert_ptr_null(account);

      account = GNUNET_CHAT_find_account(handle, TEST_JOIN_ID);

      ck_assert_ptr_nonnull(account);

      GNUNET_CHAT_connect(handle, account);
      break;
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_ptr_null(lobby);

      lobby = GNUNET_CHAT_lobby_open(
        handle,
        1,
        on_gnunet_chat_lobby_join_open,
        handle
      );

      ck_assert_ptr_nonnull(lobby);
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_ptr_null(lobby);

      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_UPDATE_ACCOUNT:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      break;
    case GNUNET_CHAT_KIND_UPDATE_CONTEXT:
      ck_assert_ptr_nonnull(context);
      break;
    case GNUNET_CHAT_KIND_JOIN:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_ptr_nonnull(lobby);

      GNUNET_CHAT_lobby_close(lobby);
      lobby = NULL;

      GNUNET_CHAT_disconnect(handle);
      break;
    default:
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

REQUIRE_GNUNET_CHAT_ACCOUNT(gnunet_chat_lobby_join, TEST_JOIN_ID)

void
call_gnunet_chat_lobby_join(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_lobby_join_msg, &handle);

  ck_assert_ptr_nonnull(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_lobby_join, gnunet_chat_lobby_join)

START_SUITE(lobby_suite, "Lobby")
ADD_TEST_TO_SUITE(test_gnunet_chat_lobby_join, "Join")
END_SUITE

MAIN_SUITE(lobby_suite, CK_NORMAL)
