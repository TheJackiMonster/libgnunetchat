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
 * @file test_gnunet_chat_discourse_write.c
 */

#include "test_gnunet_chat.h"

#define TEST_WRITE_ID        "gnunet_chat_discourse_write"
#define TEST_WRITE_GROUP     "gnunet_chat_discourse_write_group"
#define TEST_WRITE_DISCOURSE "gnunet_chat_discourse_write_discourse"

enum GNUNET_GenericReturnValue
on_gnunet_chat_discourse_write_msg(void *cls,
                                  struct GNUNET_CHAT_Context *context,
                                  struct GNUNET_CHAT_Message *message)
{
  static unsigned int discourse_stage = 0;

  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_nonnull(handle);
  ck_assert_ptr_nonnull(message);

  struct GNUNET_CHAT_Account *account;
  account = GNUNET_CHAT_message_get_account(message);

  const char *name = GNUNET_CHAT_get_name(handle);
  struct GNUNET_CHAT_DiscourseId discourse_id;

  struct GNUNET_CHAT_Discourse *discourse;
  discourse = GNUNET_CHAT_message_get_discourse(message);

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_WARNING:
      ck_abort_msg("%s\n", GNUNET_CHAT_message_get_text(message));
      break;
    case GNUNET_CHAT_KIND_REFRESH:
      ck_assert_ptr_null(context);
      ck_assert_ptr_null(account);

      if (discourse_stage == 0)
      {
        account = GNUNET_CHAT_find_account(handle, TEST_WRITE_ID);

        ck_assert_ptr_nonnull(account);

        GNUNET_CHAT_connect(handle, account);
        discourse_stage = 1;
      }

      break;
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_WRITE_ID);
      ck_assert_uint_eq(discourse_stage, 1);

      GNUNET_CHAT_group_create(handle, TEST_WRITE_GROUP);
      discourse_stage = 2;
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_uint_eq(discourse_stage, 6);
      
      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_UPDATE_ACCOUNT:
      break;
    case GNUNET_CHAT_KIND_UPDATE_CONTEXT:
      break;
    case GNUNET_CHAT_KIND_JOIN:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_null(discourse);
      ck_assert_uint_eq(discourse_stage, 2);

      GNUNET_memcpy(
        &discourse_id,
        TEST_WRITE_DISCOURSE,
        sizeof(discourse_id)
      );

      discourse = GNUNET_CHAT_context_open_discourse(
        context,
        &discourse_id
      );

      ck_assert_ptr_nonnull(discourse);
      ck_assert_int_eq(GNUNET_CHAT_discourse_is_open(discourse), GNUNET_NO);

      discourse_stage = 3;
      break;
    case GNUNET_CHAT_KIND_CONTACT:
      break;
    case GNUNET_CHAT_KIND_DISCOURSE:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(discourse);

      GNUNET_memcpy(
        &discourse_id,
        TEST_WRITE_DISCOURSE,
        sizeof(discourse_id)
      );
      
      if (GNUNET_YES == GNUNET_CHAT_discourse_is_open(discourse))
      {
        ck_assert_uint_eq(discourse_stage, 3);

        ck_assert_int_eq(
          GNUNET_CHAT_discourse_write(
            discourse,
            (const char*) &discourse_id,
            sizeof(discourse_id)
          ),
          GNUNET_OK
        );

        discourse_stage = 4;
      }
      else
      {
        ck_assert_uint_eq(discourse_stage, 5);

        GNUNET_CHAT_disconnect(handle);

        discourse_stage = 6;
      }

      break;
    case GNUNET_CHAT_KIND_DATA:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(discourse);
      ck_assert_uint_eq(discourse_stage, 4);

      ck_assert_uint_eq(
        GNUNET_CHAT_message_available(message),
        sizeof(discourse_id)
      );

      ck_assert_int_eq(
        GNUNET_CHAT_message_read(
          message,
          (char*) &discourse_id,
          sizeof(discourse_id)
        ),
        GNUNET_OK
      );

      ck_assert_mem_eq(
        &discourse_id,
        TEST_WRITE_DISCOURSE,
        sizeof(discourse_id)
      );

      GNUNET_CHAT_discourse_close(discourse);
      discourse_stage = 5;
      break;
    default:
      ck_abort_msg("%d\n", GNUNET_CHAT_message_get_kind(message));
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

REQUIRE_GNUNET_CHAT_ACCOUNT(gnunet_chat_discourse_write, TEST_WRITE_ID)

void
call_gnunet_chat_discourse_write(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_discourse_write_msg, &handle);

  ck_assert_ptr_nonnull(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_discourse_write, gnunet_chat_discourse_write)

START_SUITE(handle_suite, "Handle")
ADD_TEST_TO_SUITE(test_gnunet_chat_discourse_write, "Talk")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
