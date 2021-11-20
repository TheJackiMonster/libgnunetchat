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
 * @file gnunet_chat_handle.h
 */

#ifndef GNUNET_CHAT_HANDLE_H_
#define GNUNET_CHAT_HANDLE_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_config.h>
#include <gnunet/gnunet_container_lib.h>
#include <gnunet/gnunet_arm_service.h>
#include <gnunet/gnunet_fs_service.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

#include "gnunet_chat_lib.h"

struct GNUNET_CHAT_Handle
{
  const struct GNUNET_CONFIGURATION_Handle* cfg;

  char *directory;

  GNUNET_CHAT_ContextMessageCallback msg_cb;
  void *msg_cls;

  struct GNUNET_CONTAINER_MultiHashMap *files;
  struct GNUNET_CONTAINER_MultiHashMap *contexts;
  struct GNUNET_CONTAINER_MultiShortmap *contacts;
  struct GNUNET_CONTAINER_MultiHashMap *groups;

  struct GNUNET_ARM_Handle *arm;
  struct GNUNET_FS_Handle *fs;
  struct GNUNET_MESSENGER_Handle *messenger;

  char *public_key;
  void *user_pointer;
};

struct GNUNET_CHAT_Handle*
handle_create_from_config (const struct GNUNET_CONFIGURATION_Handle* cfg,
			   const char *directory,
			   const char *name,
			   GNUNET_CHAT_ContextMessageCallback msg_cb,
			   void *msg_cls);

void
handle_update_key (struct GNUNET_CHAT_Handle *handle);

void
handle_destroy (struct GNUNET_CHAT_Handle* handle);

#endif /* GNUNET_CHAT_HANDLE_H_ */
