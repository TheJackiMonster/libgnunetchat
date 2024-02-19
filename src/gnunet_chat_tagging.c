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
 * @file gnunet_chat_tagging.c
 */

#include "gnunet_chat_tagging.h"
#include "gnunet_chat_message.h"

#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_messenger_service.h>
#include <string.h>

static const unsigned int initial_map_size_of_tagging = 4;

struct GNUNET_CHAT_Tagging*
tagging_create ()
{
  struct GNUNET_CHAT_Tagging* tagging = GNUNET_new(struct GNUNET_CHAT_Tagging);

  tagging->tags = GNUNET_CONTAINER_multihashmap_create(
    initial_map_size_of_tagging, GNUNET_NO);

  return tagging;
}

void
tagging_destroy (struct GNUNET_CHAT_Tagging *tagging)
{
  GNUNET_assert(
    (tagging) &&
    (tagging->tags)
  );

  GNUNET_CONTAINER_multihashmap_destroy(tagging->tags);

  GNUNET_free(tagging);
}

enum GNUNET_GenericReturnValue
tagging_add (struct GNUNET_CHAT_Tagging *tagging,
             struct GNUNET_CHAT_Message *message)
{
  GNUNET_assert((tagging) && (message));

  if ((GNUNET_YES != message_has_msg(message)) ||
      (GNUNET_MESSENGER_KIND_TAG != message->msg->header.kind))
    return GNUNET_SYSERR;
  
  const char *tag = message->msg->body.tag.tag;
  struct GNUNET_HashCode hash;
  
  if (tag)
    GNUNET_CRYPTO_hash_from_string(tag, &hash);
  else
    memset(&hash, 0, sizeof(hash));

  return GNUNET_CONTAINER_multihashmap_put(
    tagging->tags,
    &hash,
    message,
    GNUNET_CONTAINER_MULTIHASHMAPOPTION_MULTIPLE
  );
}

enum GNUNET_GenericReturnValue
tagging_remove (struct GNUNET_CHAT_Tagging *tagging,
                const struct GNUNET_CHAT_Message *message)
{
  GNUNET_assert((tagging) && (message));

  if ((GNUNET_YES != message_has_msg(message)) ||
      (GNUNET_MESSENGER_KIND_TAG != message->msg->header.kind))
    return GNUNET_SYSERR;
  
  const char *tag = message->msg->body.tag.tag;
  struct GNUNET_HashCode hash;
  
  if (tag)
    GNUNET_CRYPTO_hash_from_string(tag, &hash);
  else
    memset(&hash, 0, sizeof(hash));

  return GNUNET_CONTAINER_multihashmap_remove(
    tagging->tags,
    &hash,
    message
  );
}

struct GNUNET_CHAT_TaggingIterator
{
  GNUNET_CHAT_TaggingCallback cb;
  void *cls;
};

static enum GNUNET_GenericReturnValue
tagging_iterate_message (void *cls,
                         const struct GNUNET_HashCode *key,
                         void *value)
{
  struct GNUNET_CHAT_TaggingIterator *it = cls;
  const struct GNUNET_CHAT_Message *message = value;

  if (!(it->cb))
    return GNUNET_YES;
  
  return it->cb(it->cls, message);
}

int
tagging_iterate (const struct GNUNET_CHAT_Tagging *tagging,
                 enum GNUNET_GenericReturnValue ignore_tag,
                 const char *tag,
                 GNUNET_CHAT_TaggingCallback cb,
                 void *cls)
{
  GNUNET_assert(tagging);

  struct GNUNET_CHAT_TaggingIterator it;
  it.cb = cb;
  it.cls = cls;

  if (GNUNET_YES == ignore_tag)
    return GNUNET_CONTAINER_multihashmap_iterate(
      tagging->tags,
      tagging_iterate_message,
      &it
    );

  struct GNUNET_HashCode hash;
  
  if (tag)
    GNUNET_CRYPTO_hash_from_string(tag, &hash);
  else
    memset(&hash, 0, sizeof(hash));

  return GNUNET_CONTAINER_multihashmap_get_multiple(
    tagging->tags,
    &hash,
    tagging_iterate_message,
    &it
  );
}
