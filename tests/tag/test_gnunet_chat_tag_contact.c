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
 * @file test_gnunet_chat_tag_contact.c
 */

#include "test_gnunet_chat.h"

#define TEST_TAG_ID          "gnunet_chat_tag_contact"
#define TEST_TAG_GROUP       "gnunet_chat_tag_contact_group"
#define TEST_TAG_CONTACT_TAG "test_contact_tag_tagged"

enum GNUNET_GenericReturnValue
on_gnunet_chat_tag_contact_msg(void *cls,
                               struct GNUNET_CHAT_Context *context,
                               struct GNUNET_CHAT_Message *message)
{
  static unsigned int tag_stage = 0;

  struct GNUNET_CHAT_Handle *handle = *(
    (struct GNUNET_CHAT_Handle**) cls
  );

  struct GNUNET_CHAT_Account *account;
  struct GNUNET_CHAT_Group *group;
  struct GNUNET_CHAT_Contact *contact;
  const char *text;

  ck_assert_ptr_nonnull(handle);
  ck_assert_ptr_nonnull(message);

  account = GNUNET_CHAT_message_get_account(message);
  contact = GNUNET_CHAT_message_get_sender(message);

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_WARNING:
      ck_abort_msg("%s\n", GNUNET_CHAT_message_get_text(message));
      break;
    case GNUNET_CHAT_KIND_REFRESH:
      ck_assert_ptr_null(context);
      ck_assert_ptr_null(account);

      if (tag_stage == 0)
      {
        account = GNUNET_CHAT_find_account(handle, TEST_TAG_ID);

        ck_assert_ptr_nonnull(account);

        GNUNET_CHAT_connect(handle, account);
        tag_stage = 1;
      }

      break;
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_uint_eq(tag_stage, 1);

      group = GNUNET_CHAT_group_create(handle, TEST_TAG_GROUP);

      ck_assert_ptr_nonnull(group);

      tag_stage = 2;
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_ptr_null(context);
      ck_assert_ptr_nonnull(account);
      ck_assert_uint_eq(tag_stage, 6);

      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_UPDATE_ACCOUNT:
      ck_assert_ptr_nonnull(account);
      break;
    case GNUNET_CHAT_KIND_UPDATE_CONTEXT:
      ck_assert_ptr_nonnull(context);
      break;
    case GNUNET_CHAT_KIND_JOIN:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(contact);
      ck_assert_uint_eq(tag_stage, 2);

      ck_assert_int_eq(GNUNET_CHAT_contact_is_tagged(
        contact, TEST_TAG_CONTACT_TAG
      ), GNUNET_NO);

      GNUNET_CHAT_contact_tag(
        contact,
        TEST_TAG_CONTACT_TAG
      );

      tag_stage = 3;
      break;
    case GNUNET_CHAT_KIND_LEAVE:
      ck_assert_ptr_nonnull(context);
      ck_assert_uint_eq(tag_stage, 5);
      
      GNUNET_CHAT_disconnect(handle);

      tag_stage = 6;
      break;
    case GNUNET_CHAT_KIND_CONTACT:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(contact);
      break;
    case GNUNET_CHAT_KIND_TAG:
      ck_assert_ptr_nonnull(context);
      ck_assert_uint_ge(tag_stage, 3);

      if (GNUNET_YES == GNUNET_CHAT_message_is_deleted(message))
        break;

      ck_assert_uint_eq(tag_stage, 3);

      text = GNUNET_CHAT_message_get_text(message);

      ck_assert_str_eq(text, TEST_TAG_CONTACT_TAG);

      message = GNUNET_CHAT_message_get_target(message);

      ck_assert_ptr_nonnull(message);

      contact = GNUNET_CHAT_message_get_sender(message);

      ck_assert_ptr_nonnull(contact);

      ck_assert_int_eq(GNUNET_CHAT_contact_is_tagged(
        contact, TEST_TAG_CONTACT_TAG
      ), GNUNET_YES);
      
      GNUNET_CHAT_contact_untag(
        contact,
        TEST_TAG_CONTACT_TAG
      );

      tag_stage = 4;
      break;
    case GNUNET_CHAT_KIND_DELETION:
      ck_assert_ptr_nonnull(context);
      ck_assert_uint_eq(tag_stage, 4);

      group = GNUNET_CHAT_context_get_group(context);

      ck_assert_ptr_nonnull(group);
      
      ck_assert_int_eq(
        GNUNET_CHAT_group_leave(group),
        GNUNET_OK
      );

      tag_stage = 5;
      break;
    default:
      ck_abort_msg("%d\n", GNUNET_CHAT_message_get_kind(message));
      break;
  }

  return GNUNET_YES;
}

REQUIRE_GNUNET_CHAT_ACCOUNT(gnunet_chat_tag_contact, TEST_TAG_ID)

void
call_gnunet_chat_tag_contact(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_tag_contact_msg, &handle);

  ck_assert_ptr_nonnull(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_tag_contact, gnunet_chat_tag_contact)

START_SUITE(handle_suite, "Tag")
ADD_TEST_TO_SUITE(test_gnunet_chat_tag_contact, "Contact")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
