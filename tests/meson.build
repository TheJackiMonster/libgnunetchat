#
# This file is part of GNUnet.
# Copyright (C) 2023--2024 GNUnet e.V.
#
# GNUnet is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# GNUnet is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: AGPL3.0-or-later
#

test_deps = [
    check_dep,
    dependency('gnunetutil'),
]

test_gnunet_chat_handle = executable(
    'test_gnunet_chat_handle.test',
    'test_gnunet_chat_handle.c',
    dependencies: test_deps,
    link_with: gnunetchat_lib,
    include_directories: tests_include,
    extra_files: 'test_gnunet_chat.h',
)

test_gnunet_chat_lobby = executable(
    'test_gnunet_chat_lobby.test',
    'test_gnunet_chat_lobby.c',
    dependencies: test_deps,
    link_with: gnunetchat_lib,
    include_directories: tests_include,
    extra_files: 'test_gnunet_chat.h',
)

test_gnunet_chat_file = executable(
    'test_gnunet_chat_file.test',
    'test_gnunet_chat_file.c',
    dependencies: test_deps,
    link_with: gnunetchat_lib,
    include_directories: tests_include,
    extra_files: 'test_gnunet_chat.h',
)

test_gnunet_chat_message = executable(
    'test_gnunet_chat_message.test',
    'test_gnunet_chat_message.c',
    dependencies: test_deps,
    link_with: gnunetchat_lib,
    include_directories: tests_include,
    extra_files: 'test_gnunet_chat.h',
)

test_gnunet_chat_attribute = executable(
    'test_gnunet_chat_attribute.test',
    'test_gnunet_chat_attribute.c',
    dependencies: test_deps,
    link_with: gnunetchat_lib,
    include_directories: tests_include,
    extra_files: 'test_gnunet_chat.h',
)

test('test_gnunet_chat_handle', test_gnunet_chat_handle, depends: gnunetchat_lib)
test('test_gnunet_chat_lobby', test_gnunet_chat_lobby, depends: gnunetchat_lib)
test('test_gnunet_chat_file', test_gnunet_chat_file, depends: gnunetchat_lib)
test('test_gnunet_chat_message', test_gnunet_chat_message, depends: gnunetchat_lib)
test('test_gnunet_chat_attribute', test_gnunet_chat_attribute, depends: gnunetchat_lib)
