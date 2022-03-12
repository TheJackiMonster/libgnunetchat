/*
   This file is part of GNUnet.
   Copyright (C) 2021--2022 GNUnet e.V.

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
#include <gnunet/gnunet_gns_service.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_namestore_service.h>
#include <gnunet/gnunet_scheduler_lib.h>
#include <gnunet/gnunet_util_lib.h>

#include "gnunet_chat_lib.h"
#include "gnunet_chat_account.h"
#include "gnunet_chat_lobby.h"
#include "gnunet_chat_message.h"
#include "gnunet_chat_uri.h"

struct GNUNET_CHAT_Handle;

struct GNUNET_CHAT_InternalMessages
{
  struct GNUNET_CHAT_Message *msg;
  struct GNUNET_CHAT_InternalMessages *next;
  struct GNUNET_CHAT_InternalMessages *prev;
};

struct GNUNET_CHAT_InternalAccounts
{
  struct GNUNET_CHAT_Account *account;
  struct GNUNET_CHAT_InternalAccounts *next;
  struct GNUNET_CHAT_InternalAccounts *prev;
};

struct GNUNET_CHAT_InternalLobbies
{
  struct GNUNET_CHAT_Lobby *lobby;
  struct GNUNET_CHAT_InternalLobbies *next;
  struct GNUNET_CHAT_InternalLobbies *prev;
};

struct GNUNET_CHAT_UriLookups
{
  struct GNUNET_CHAT_Handle *handle;

  struct GNUNET_GNS_LookupRequest *request;
  struct GNUNET_CHAT_Uri *uri;

  struct GNUNET_CHAT_UriLookups *next;
  struct GNUNET_CHAT_UriLookups *prev;
};

struct GNUNET_CHAT_Handle
{
  const struct GNUNET_CONFIGURATION_Handle* cfg;
  struct GNUNET_SCHEDULER_Task *shutdown_hook;

  struct GNUNET_CHAT_InternalMessages *internal_head;
  struct GNUNET_CHAT_InternalMessages *internal_tail;

  char *directory;

  GNUNET_CHAT_ContextMessageCallback msg_cb;
  void *msg_cls;

  struct GNUNET_CHAT_InternalAccounts *accounts_head;
  struct GNUNET_CHAT_InternalAccounts *accounts_tail;

  const struct GNUNET_CHAT_Account *current;
  struct GNUNET_IDENTITY_Operation *creation_op;
  struct GNUNET_NAMESTORE_ZoneMonitor *monitor;

  struct GNUNET_CHAT_InternalLobbies *lobbies_head;
  struct GNUNET_CHAT_InternalLobbies *lobbies_tail;

  struct GNUNET_CHAT_UriLookups *lookups_head;
  struct GNUNET_CHAT_UriLookups *lookups_tail;

  struct GNUNET_CONTAINER_MultiHashMap *files;
  struct GNUNET_CONTAINER_MultiHashMap *contexts;
  struct GNUNET_CONTAINER_MultiShortmap *contacts;
  struct GNUNET_CONTAINER_MultiHashMap *groups;

  struct GNUNET_ARM_Handle *arm;
  struct GNUNET_FS_Handle *fs;
  struct GNUNET_GNS_Handle *gns;
  struct GNUNET_IDENTITY_Handle *identity;
  struct GNUNET_MESSENGER_Handle *messenger;
  struct GNUNET_NAMESTORE_Handle *namestore;

  char *public_key;
  void *user_pointer;
};

struct GNUNET_CHAT_Handle*
handle_create_from_config (const struct GNUNET_CONFIGURATION_Handle* cfg,
			   const char *directory,
			   GNUNET_CHAT_ContextMessageCallback msg_cb,
			   void *msg_cls);

void
handle_update_key (struct GNUNET_CHAT_Handle *handle);

void
handle_destroy (struct GNUNET_CHAT_Handle *handle);

void
handle_connect (struct GNUNET_CHAT_Handle *handle,
		const struct GNUNET_CHAT_Account *account);

void
handle_disconnect (struct GNUNET_CHAT_Handle *handle);

const char*
handle_get_directory (const struct GNUNET_CHAT_Handle *handle);

const struct GNUNET_IDENTITY_PrivateKey*
handle_get_key (const struct GNUNET_CHAT_Handle *handle);

void
handle_send_internal_message (struct GNUNET_CHAT_Handle *handle,
			      struct GNUNET_CHAT_Context *context,
			      enum GNUNET_CHAT_MessageFlag flag,
			      const char *warning);

void
handle_send_room_name (struct GNUNET_CHAT_Handle *handle,
		       struct GNUNET_MESSENGER_Room *room);

int
handle_request_context_by_room (struct GNUNET_CHAT_Handle *handle,
				struct GNUNET_MESSENGER_Room *room);

struct GNUNET_CHAT_Contact*
handle_get_contact_from_messenger (const struct GNUNET_CHAT_Handle *handle,
				   const struct GNUNET_MESSENGER_Contact *contact);

struct GNUNET_CHAT_Group*
handle_get_group_from_messenger (const struct GNUNET_CHAT_Handle *handle,
				 const struct GNUNET_MESSENGER_Room *room);

struct GNUNET_CHAT_Context*
handle_process_records (struct GNUNET_CHAT_Handle *handle,
			const char *label,
			unsigned int count,
			const struct GNUNET_GNSRECORD_Data *data);

#endif /* GNUNET_CHAT_HANDLE_H_ */
