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

#include <check.h>
#include <gnunet/gnunet_chat_lib.h>

START_TEST(test_gnunet_chat_handle)
{
  struct GNUNET_CHAT_Handle *handle;

  handle = GNUNET_CHAT_start(NULL, "", "Test", NULL, NULL, NULL, NULL);
  ck_assert_ptr_eq(handle, NULL);

  GNUNET_CHAT_stop(handle);
}
END_TEST

Suite* handle_suite(void)
{
  Suite *suite;
  TCase *test_case;

  suite = suite_create("Handle");

  test_case = tcase_create("Start/Stop");
  tcase_add_test(test_case, test_gnunet_chat_handle);
  suite_add_tcase(suite, test_case);

  return suite;
}

int main(void)
{
  int tests_failed;

  Suite *suite;
  SRunner *suite_runner;

  suite = handle_suite();
  suite_runner = srunner_create(suite);

  srunner_run_all(suite_runner, CK_NORMAL);
  tests_failed = srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (tests_failed == 0? EXIT_SUCCESS : EXIT_FAILURE);
}
