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
 * @file gnunet_chat_discourse.c
 */

#include "gnunet_chat_discourse.h"
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_scheduler_lib.h>
#include <gnunet/gnunet_time_lib.h>

struct GNUNET_CHAT_Discourse*
discourse_create (struct GNUNET_CHAT_Context *context,
                  const struct GNUNET_ShortHashCode *id)
{
  GNUNET_assert((context) && (id));

  struct GNUNET_CHAT_Discourse *discourse = GNUNET_new(struct GNUNET_CHAT_Discourse);

  discourse->context = context;

  GNUNET_memcpy(&(discourse->id), id, sizeof (struct GNUNET_ShortHashCode));

  discourse->head = NULL;
  discourse->tail = NULL;

  return discourse;
}

static void
discourse_remove_subscription (void *cls)
{
  struct GNUNET_CHAT_DiscourseSubscription *sub = cls;
  struct GNUNET_CHAT_Discourse *discourse = sub->discourse;

  GNUNET_CONTAINER_DLL_remove(
    discourse->head,
    discourse->tail,
    sub
  );

  GNUNET_free(sub);
}

void
discourse_destroy (struct GNUNET_CHAT_Discourse *discourse)
{
  GNUNET_assert(discourse);

  while (discourse->head)
    discourse_remove_subscription (discourse->head);

  GNUNET_free(discourse);
}

void
discourse_subscribe (struct GNUNET_CHAT_Discourse *discourse,
                     struct GNUNET_CHAT_Contact *contact,
                     const struct GNUNET_TIME_Absolute timestamp,
                     const struct GNUNET_TIME_Relative time)
{
  GNUNET_assert((discourse) && (contact));

  const struct GNUNET_TIME_Absolute end = GNUNET_TIME_absolute_add(
    timestamp,
    time
  );

  if (GNUNET_TIME_absolute_cmp(end, <, GNUNET_TIME_absolute_get()))
    return;

  struct GNUNET_CHAT_DiscourseSubscription *sub;
  for (sub = discourse->head; sub; sub = sub->next)
    if (sub->contact == contact)
      break;

  if (!sub)
  {
    sub = GNUNET_new(struct GNUNET_CHAT_DiscourseSubscription);

    sub->prev = NULL;
    sub->next = NULL;

    sub->discourse = discourse;
    sub->contact = contact;

    GNUNET_CONTAINER_DLL_insert(
      discourse->head,
      discourse->tail,
      sub
    );
  }
  else if (sub->task)
    GNUNET_SCHEDULER_cancel(sub->task);

  sub->start = timestamp;
  sub->end = end;

  sub->task = GNUNET_SCHEDULER_add_at(
    end,
    discourse_remove_subscription,
    sub
  );
}

void
discourse_unsubscribe (struct GNUNET_CHAT_Discourse *discourse,
                       struct GNUNET_CHAT_Contact *contact,
                       const struct GNUNET_TIME_Absolute timestamp,
                       const struct GNUNET_TIME_Relative delay)
{
  GNUNET_assert((discourse) && (contact));

  struct GNUNET_CHAT_DiscourseSubscription *sub;
  for (sub = discourse->head; sub; sub = sub->next)
    if (sub->contact == contact)
      break;

  if ((!sub) || (GNUNET_TIME_absolute_cmp(sub->start, >, timestamp)))
    return;

  const struct GNUNET_TIME_Absolute exit = GNUNET_TIME_absolute_add(
    timestamp, delay
  );

  if (GNUNET_TIME_absolute_cmp(exit, <, sub->end))
    sub->end = exit;

  if (sub->task)
    GNUNET_SCHEDULER_cancel(sub->task);

  if (GNUNET_TIME_absolute_cmp(sub->end, <, GNUNET_TIME_absolute_get()))
    discourse_remove_subscription(sub);
  else
    sub->task = GNUNET_SCHEDULER_add_at(
      sub->end,
      discourse_remove_subscription,
      sub
    );
}
