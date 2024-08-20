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
 * @file test_gnunet_chat_file_send.c
 */

#include "test_gnunet_chat.h"

#define TEST_SEND_ID       "gnunet_chat_file_send"
#define TEST_SEND_TEXT     "gnunet_chat_file_deleted"
#define TEST_SEND_FILENAME "gnunet_chat_file_send_name"
#define TEST_SEND_GROUP    "gnunet_chat_file_send_group"

void
on_gnunet_chat_file_send_upload(void *cls,
                                struct GNUNET_CHAT_File *file,
                                uint64_t completed,
                                uint64_t size)
{
  struct GNUNET_CHAT_Handle *handle = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_nonnull(handle);
  ck_assert_ptr_nonnull(file);
  ck_assert_uint_le(completed, size);

  uint64_t check_size = GNUNET_CHAT_file_get_size(file);

  ck_assert_uint_eq(size, check_size);

  if (completed > size)
    return;
  
  ck_assert_uint_eq(completed, size);
}

void
on_gnunet_chat_file_send_unindex(void *cls,
                                 struct GNUNET_CHAT_File *file,
                                 uint64_t completed,
                                 uint64_t size)
{
  struct GNUNET_CHAT_Context *context = (struct GNUNET_CHAT_Context*) cls;

  ck_assert_ptr_nonnull(context);
  ck_assert_ptr_nonnull(file);
  ck_assert_uint_le(completed, size);

  if (completed > size)
    return;
  
  ck_assert_uint_eq(completed, size);

  GNUNET_CHAT_context_send_text(context, TEST_SEND_TEXT);
}

enum GNUNET_GenericReturnValue
on_gnunet_chat_file_send_msg(void *cls,
                             struct GNUNET_CHAT_Context *context,
                             const struct GNUNET_CHAT_Message *message)
{
  static char *filename = NULL;

  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_nonnull(handle);
  ck_assert_ptr_nonnull(message);

  const struct GNUNET_CHAT_Account *account;
  struct GNUNET_CHAT_Group *group;
  struct GNUNET_CHAT_File *file;

  const char *name;
  const char *text;

  switch (GNUNET_CHAT_message_get_kind(message))
  {
    case GNUNET_CHAT_KIND_WARNING:
      ck_abort_msg("%s\n", GNUNET_CHAT_message_get_text(message));
      break;
    case GNUNET_CHAT_KIND_REFRESH:
      ck_assert_ptr_null(context);
      break;
    case GNUNET_CHAT_KIND_LOGIN:
      ck_assert_ptr_null(context);

      group = GNUNET_CHAT_group_create(
          handle, TEST_SEND_GROUP
      );

      ck_assert_ptr_nonnull(group);

      context = GNUNET_CHAT_group_get_context(group);

      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_null(filename);

      filename = GNUNET_DISK_mktemp(TEST_SEND_FILENAME);

      ck_assert_ptr_nonnull(filename);

      file = GNUNET_CHAT_context_send_file(
          context,
          filename,
          on_gnunet_chat_file_send_upload,
          handle
      );

      ck_assert_ptr_nonnull(file);
      break;
    case GNUNET_CHAT_KIND_LOGOUT:
      ck_assert_ptr_null(context);
      ck_assert_ptr_null(filename);
      break;
    case GNUNET_CHAT_KIND_CREATED_ACCOUNT:
      ck_assert_ptr_null(context);

      account = GNUNET_CHAT_message_get_account(message);

      ck_assert_ptr_nonnull(account);

      name = GNUNET_CHAT_account_get_name(account);

      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_SEND_ID);

      GNUNET_CHAT_connect(handle, account);
      break;
    case GNUNET_CHAT_KIND_DELETED_ACCOUNT:
      ck_assert_ptr_null(context);

      account = GNUNET_CHAT_message_get_account(message);

      ck_assert_ptr_nonnull(account);

      name = GNUNET_CHAT_account_get_name(account);

      ck_assert_ptr_nonnull(name);
      ck_assert_str_eq(name, TEST_SEND_ID);

      GNUNET_CHAT_stop(handle);
      break;
    case GNUNET_CHAT_KIND_UPDATE_ACCOUNT:
      ck_assert_ptr_null(context);
      break;
    case GNUNET_CHAT_KIND_UPDATE_CONTEXT:
    case GNUNET_CHAT_KIND_JOIN:
    case GNUNET_CHAT_KIND_CONTACT:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(filename);
      break;
    case GNUNET_CHAT_KIND_TEXT:
      ck_assert_ptr_nonnull(context);
      ck_assert_ptr_nonnull(filename);

      remove(filename);
      GNUNET_free(filename);
      filename = NULL;

      text = GNUNET_CHAT_message_get_text(message);

      ck_assert_ptr_nonnull(text);
      ck_assert_str_eq(text, TEST_SEND_TEXT);
      ck_assert_int_eq(GNUNET_CHAT_account_delete(
          handle, TEST_SEND_ID
      ), GNUNET_OK);
      break;
    case GNUNET_CHAT_KIND_FILE:
      ck_assert_ptr_nonnull(context);

      file = GNUNET_CHAT_message_get_file(message);

      ck_assert_ptr_nonnull(file);
      ck_assert_int_eq(GNUNET_CHAT_file_unindex(
          file, 
          on_gnunet_chat_file_send_unindex, 
          context
      ), GNUNET_OK);
      break;
    default:
      ck_abort();
      break;
  }

  return GNUNET_YES;
}

void
call_gnunet_chat_file_send(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_file_send_msg, &handle);

  ck_assert_ptr_nonnull(handle);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
      handle,
      TEST_SEND_ID
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_file_send, call_gnunet_chat_file_send)

START_SUITE(handle_suite, "File")
ADD_TEST_TO_SUITE(test_gnunet_chat_file_send, "Send")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
