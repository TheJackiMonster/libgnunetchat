/*
   This file is part of GNUnet.
   Copyright (C) 2022--2023 GNUnet e.V.

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
 * @file test_gnunet_chat_file.c
 */

#include "gnunet/gnunet_chat_lib.h"
#include "test_gnunet_chat.h"
#include <check.h>
#include <gnunet/gnunet_common.h>

int
on_gnunet_chat_file_send_it(void *cls,
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

  if (0 == strcmp(name, "gnunet_chat_file_send"))
  {
    GNUNET_CHAT_connect(chat, account);
    return GNUNET_NO;
  }

  return GNUNET_YES;
}

void
on_gnunet_chat_file_send_upload(void *cls,
                                const struct GNUNET_CHAT_File *file,
                                uint64_t completed,
                                uint64_t size)
{
  struct GNUNET_CHAT_Handle *handle = (struct GNUNET_CHAT_Handle*) cls;

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_ne(file, NULL);
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

  ck_assert_ptr_ne(context, NULL);
  ck_assert_ptr_ne(file, NULL);
  ck_assert_uint_le(completed, size);

  if (completed > size)
    return;
  
  ck_assert_uint_eq(completed, size);

  GNUNET_CHAT_context_send_text(context, "gnunet_chat_file_deleted");
}

int
on_gnunet_chat_file_send_msg(void *cls,
                             struct GNUNET_CHAT_Context *context,
                             const struct GNUNET_CHAT_Message *message)
{
  struct GNUNET_CHAT_Handle *handle = *(
      (struct GNUNET_CHAT_Handle**) cls
  );

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_ptr_ne(message, NULL);

  enum GNUNET_CHAT_MessageKind kind = GNUNET_CHAT_message_get_kind(message);
  struct GNUNET_CHAT_File *file;

  if (kind == GNUNET_CHAT_KIND_TEXT)
    goto exit_file_test;

  if ((kind != GNUNET_CHAT_KIND_REFRESH) ||
      (GNUNET_CHAT_get_connected(handle)))
    goto skip_search_account;

  ck_assert(kind == GNUNET_CHAT_KIND_REFRESH);
  ck_assert_ptr_eq(context, NULL);

  ck_assert_int_ne(GNUNET_CHAT_iterate_accounts(
      handle,
      on_gnunet_chat_file_send_it,
      handle
  ), GNUNET_SYSERR);

  if (!GNUNET_CHAT_get_connected(handle))
    return GNUNET_YES;

skip_search_account:
  if ((GNUNET_CHAT_KIND_LOGIN != kind) ||
      (context))
    goto skip_file_upload;

  ck_assert(kind == GNUNET_CHAT_KIND_LOGIN);
  ck_assert_ptr_eq(context, NULL);

  struct GNUNET_CHAT_Group *group = GNUNET_CHAT_group_create(
      handle,
      "gnunet_chat_file_send_group"
  );

  ck_assert_ptr_ne(group, NULL);

  struct GNUNET_CHAT_Context *group_context = GNUNET_CHAT_group_get_context(
      group
  );

  ck_assert_ptr_ne(group_context, NULL);

  char *filename = GNUNET_DISK_mktemp("gnunet_chat_file_send_name");

  ck_assert_ptr_ne(filename, NULL);

  file = GNUNET_CHAT_context_send_file(
      group_context,
      filename,
      on_gnunet_chat_file_send_upload,
      handle
  );

  ck_assert_ptr_ne(file, NULL);

skip_file_upload:
  if (GNUNET_CHAT_KIND_FILE != kind)
    return GNUNET_YES;

  ck_assert(kind == GNUNET_CHAT_KIND_FILE);
  ck_assert_ptr_ne(context, NULL);

  file = GNUNET_CHAT_message_get_file(message);

  ck_assert_ptr_ne(file, NULL);

  ck_assert_int_eq(GNUNET_CHAT_file_unindex(
      file, 
      on_gnunet_chat_file_send_unindex, 
      context
  ), GNUNET_OK);

  return GNUNET_YES;

exit_file_test:
  ck_assert(kind == GNUNET_CHAT_KIND_TEXT);
  ck_assert_ptr_ne(context, NULL);

  const char* text = GNUNET_CHAT_message_get_text(message);

  ck_assert_str_eq(text, "gnunet_chat_file_deleted");
  ck_assert_int_eq(GNUNET_CHAT_account_delete(
      handle,
      "gnunet_chat_file_send"
  ), GNUNET_OK);

  GNUNET_CHAT_stop(handle);
  return GNUNET_YES;
}

void
call_gnunet_chat_file_send(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  static struct GNUNET_CHAT_Handle *handle = NULL;
  handle = GNUNET_CHAT_start(cfg, on_gnunet_chat_file_send_msg, &handle);

  ck_assert_ptr_ne(handle, NULL);
  ck_assert_int_eq(GNUNET_CHAT_account_create(
      handle,
      "gnunet_chat_file_send"
  ), GNUNET_OK);
}

CREATE_GNUNET_TEST(test_gnunet_chat_file_send, call_gnunet_chat_file_send)

START_SUITE(handle_suite, "File")
ADD_TEST_TO_SUITE(test_gnunet_chat_file_send, "Send")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
