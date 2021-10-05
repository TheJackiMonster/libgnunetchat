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
call_gnunet_chat_handle(const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  struct GNUNET_CHAT_Handle *handle;

  handle = GNUNET_CHAT_start(cfg, "", "Test", NULL, NULL, NULL, NULL);
  ck_assert_ptr_ne(handle, NULL);

  GNUNET_CHAT_stop(handle);
}

CREATE_GNUNET_TEST(test_gnunet_chat_handle, call_gnunet_chat_handle)

START_SUITE(handle_suite, "Handle")
ADD_TEST_TO_SUITE(test_gnunet_chat_handle, "Start/Stop")
END_SUITE

MAIN_SUITE(handle_suite, CK_NORMAL)
