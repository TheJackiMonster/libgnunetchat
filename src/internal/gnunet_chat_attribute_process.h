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
 * @file gnunet_chat_attribute_process.h
 */

#ifndef GNUNET_CHAT_INTERNAL_ATTRIBUTE_PROCESS_H_
#define GNUNET_CHAT_INTERNAL_ATTRIBUTE_PROCESS_H_

#include <gnunet/gnunet_reclaim_service.h>

#include "gnunet_chat_lib.h"

struct GNUNET_CHAT_Handle;
struct GNUNET_CHAT_Account;
struct GNUNET_CHAT_Contact;

struct GNUNET_CHAT_AttributeProcess
{
  struct GNUNET_CHAT_Handle *handle;

  const struct GNUNET_CHAT_Account *account;
  struct GNUNET_CHAT_Contact *contact;

  struct GNUNET_RECLAIM_Attribute *attribute;
  struct GNUNET_TIME_Relative expires;
  char *name;
  void *data;

  GNUNET_CHAT_AttributeCallback callback;
  GNUNET_CHAT_AccountAttributeCallback account_callback;

  void *closure;

  struct GNUNET_RECLAIM_AttributeIterator *iter;
  struct GNUNET_RECLAIM_Operation *op;

  struct GNUNET_CHAT_AttributeProcess *next;
  struct GNUNET_CHAT_AttributeProcess *prev;
};

struct GNUNET_CHAT_AttributeProcess*
internal_attributes_create(struct GNUNET_CHAT_Handle *handle,
                           const char *name);

struct GNUNET_CHAT_AttributeProcess*
internal_attributes_create_store(struct GNUNET_CHAT_Handle *handle,
                                 const char *name,
                                 struct GNUNET_TIME_Relative expires);

struct GNUNET_CHAT_AttributeProcess*
internal_attributes_create_share(struct GNUNET_CHAT_Handle *handle,
                                 struct GNUNET_CHAT_Contact *contact,
                                 const char *name);

struct GNUNET_CHAT_AttributeProcess*
internal_attributes_create_request(struct GNUNET_CHAT_Handle *handle,
                                   const struct GNUNET_CHAT_Account *account);

void
internal_attributes_destroy(struct GNUNET_CHAT_AttributeProcess *attributes);

void
internal_attributes_next_iter(struct GNUNET_CHAT_AttributeProcess *attributes);

void
internal_attributes_stop_iter(struct GNUNET_CHAT_AttributeProcess *attributes);

#endif /* GNUNET_CHAT_INTERNAL_ATTRIBUTE_PROCESS_H_ */
