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
 * @file gnunet_chat_context_intern.c
 */

#include "gnunet_chat_invitation.h"
#include "gnunet_chat_message.h"

#include <gnunet/gnunet_error_codes.h>

#define GNUNET_UNUSED __attribute__ ((unused))

int
it_destroy_context_timestamps (GNUNET_UNUSED void *cls,
			       GNUNET_UNUSED const struct GNUNET_ShortHashCode *key,
			       void *value)
{
  GNUNET_assert(value);

  struct GNUNET_TIME_Absolute *time = value;
  GNUNET_free(time);
  return GNUNET_YES;
}

int
it_destroy_context_messages (GNUNET_UNUSED void *cls,
			     GNUNET_UNUSED const struct GNUNET_HashCode *key,
			     void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_Message *message = value;
  message_destroy(message);
  return GNUNET_YES;
}

int
it_destroy_context_invites (GNUNET_UNUSED void *cls,
			    GNUNET_UNUSED const struct GNUNET_HashCode *key,
			    void *value)
{
  GNUNET_assert(value);

  struct GNUNET_CHAT_Invitation *invitation = value;
  invitation_destroy(invitation);
  return GNUNET_YES;
}

void
cont_context_write_records (void *cls,
			    enum GNUNET_ErrorCode ec)
{
  struct GNUNET_CHAT_Context *context = cls;

  GNUNET_assert(context);

  context->query = NULL;

  if (GNUNET_EC_NONE != ec)
    handle_send_internal_message(
	context->handle,
	context,
	GNUNET_CHAT_FLAG_WARNING,
	GNUNET_ErrorCode_get_hint(ec)
    );
}
