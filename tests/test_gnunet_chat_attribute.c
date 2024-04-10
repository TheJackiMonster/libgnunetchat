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
 * @file test_gnunet_chat_attribute.c
 */

#include "test_gnunet_chat.h"
#include <check.h>
#include <gnunet/gnunet_chat_lib.h>
#include <gnunet/gnunet_time_lib.h>
#include <stdio.h>

enum GNUNET_GenericReturnValue
on_gnunet_chat_attribute_check_it(void *cls,
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

  if (0 == strcmp(name, "gnunet_chat_attribute_check"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

enum GNUNET_GenericReturnValue
on_gnunet_chat_attribute_check_attr(void *cls,
                                    struct GNUNET_CHAT_Handle *handle,
                                    const char *name,
                                    const char *value)
{
  ck_assert_ptr_ne(name, NULL);
  if (0 == strcmp(name, "test_attribute_name"))
  {
    ck_assert_ptr_ne(value, NULL);
    ck_assert_int_eq(strcmp(value, "test_attribute_value"), 0);
    GNUNET_CHAT_delete_attribute(handle, "test_attribute_name");
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

enum GNUNET_GenericReturnValue
on_gnunet_chat_attribute_check_msg(void *cls,
                                struct GNUNET_CHAT_Context *context,
                                const struct GNUNET_CHAT_Message *message)
{
  struct GNUNET_CHAT_Handle *handle = *(
    (struct GNUNET_CHAT_Handle**) cls
  );

  const char *text = NULL;

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_ne(message, NULL);

  if (GNUNET_CHAT_get_connected(handle))
    goto skip_search_account;

  GNUNET_CHAT_iterate_accounts(
    handle,
    on_gnunet_chat_attribute_check_it,
    handle
  );

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

skip_search_account:
  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_eq(context, NULL);

      GNUNET_CHAT_set_attribute(
        handle,
        "test_attribute_name",
        "test_attribute_value",
        GNUNET_TIME_relative_get_forever_()
      );
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_int_eq(GNUNET_CHAT_account_delete(
	      handle,
	      "gnunet_chat_attribute_check"
      ), GNUNET_OK);

      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_ATTRIBUTES:
      ck_assert_ptr_eq(context, NULL);

      text = GNUNET_CHAT_message_get_text(message);

      if (text)
      {
        ck_assert_int_eq(strcmp(text, "test_attribute_name"), 0);
        GNUNET_CHAT_get_attributes(
          handle,
          on_gnunet_chat_attribute_check_attr,
          NULL
        );
      }
      else
        GNUNET_CHAT_disconnect(handle);

      break;
    default:
      break;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_attribute_check(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_attribute_check_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
    handle,
    "gnunet_chat_attribute_check"
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_attribute_check, call_gnunet_chat_attribute_check)

START_SUITE(handle_suite, "Attribute")
ADD_TEST_TO_SUITE(test_gnunet_chat_attribute_check, "Check")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
