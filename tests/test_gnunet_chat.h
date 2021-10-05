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
 * @file test_gnunet_chat.h
 */

#ifndef TEST_GNUNET_CHAT_H_
#define TEST_GNUNET_CHAT_H_

#include <stdlib.h>
#include <check.h>

#include <gnunet/gnunet_chat_lib.h>

#define CREATE_GNUNET_TEST(test_name, test_call)                 \
void                                                             \
task_##test_call (__attribute__ ((unused)) void *cls,            \
                  __attribute__ ((unused)) char *const *args,    \
		  __attribute__ ((unused)) const char *cfgfile,  \
		  const struct GNUNET_CONFIGURATION_Handle *cfg) \
{                                                                \
  test_call (cfg);                                               \
}                                                                \
                                                                 \
START_TEST(test_name)                                            \
{                                                                \
  enum GNUNET_GenericReturnValue result;                         \
  struct GNUNET_GETOPT_CommandLineOption options[] = {           \
    GNUNET_GETOPT_OPTION_END                                     \
  };                                                             \
                                                                 \
  char *binary = #test_call;                                     \
  char *const args [] = { binary };                              \
                                                                 \
  result = GNUNET_PROGRAM_run(                                   \
    1,                                                           \
    args,                                                        \
    binary,                                                      \
    "",                                                          \
    options,                                                     \
    task_##test_call,                                            \
    NULL                                                         \
  );                                                             \
                                                                 \
  ck_assert(result == GNUNET_OK);                                \
}                                                                \
END_TEST

#define START_SUITE(suite_name, suite_title) \
Suite* suite_name (void)                     \
{                                            \
  Suite *suite;                              \
  TCase *tcase;                              \
                                             \
  suite = suite_create(suite_title);

#define ADD_TEST_TO_SUITE(test_name, test_title) \
  tcase = tcase_create(test_title);              \
  tcase_add_test(tcase, test_name);              \
  suite_add_tcase(suite, tcase);

#define END_SUITE \
  return suite;   \
}

#define MAIN_SUITE(suite_name, suite_check)                \
int main (void)                                            \
{                                                          \
  int tests_failed;                                        \
  SRunner *runner;                                         \
                                                           \
  runner = srunner_create(suite_name ());                  \
  srunner_run_all(runner, suite_check);                    \
                                                           \
  tests_failed = srunner_ntests_failed(runner);            \
  srunner_free(runner);                                    \
                                                           \
  return (tests_failed == 0? EXIT_SUCCESS : EXIT_FAILURE); \
}

#endif /* TEST_GNUNET_CHAT_H_ */
