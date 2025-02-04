/*
   This file is part of GNUnet.
   Copyright (C) 2025 GNUnet e.V.

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
 * @file gnunet_messenger_ping.c
 */

#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_scheduler_lib.h>
#include <gnunet/gnunet_time_lib.h>
#include <gnunet/gnunet_util_lib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

struct GNUNET_MESSENGER_PingTool
{
  const struct GNUNET_CONFIGURATION_Handle *cfg;
  struct GNUNET_IDENTITY_EgoLookup *lookup;
  struct GNUNET_MESSENGER_Handle *handle;
  struct GNUNET_SCHEDULER_Task *task;

  struct GNUNET_CONTAINER_MultiHashMap *map;
  struct GNUNET_CONTAINER_MultiHashMap *ping_map;

  char *ego_name;
  char *room_name;
  uint count;
  uint timeout;
  int public_room;
  int auto_pong;
  int require_pong;

  bool quit;
};

struct GNUNET_MESSENGER_Ping
{
  struct GNUNET_TIME_Absolute ping_time;
  const struct GNUNET_MESSENGER_Contact *sender;

  struct GNUNET_CONTAINER_MultiShortmap *pong_map;

  size_t pong_missing;
  size_t traffic;
};

static const struct GNUNET_ShortHashCode*
hash_contact (const struct GNUNET_MESSENGER_Contact *contact)
{
  static struct GNUNET_ShortHashCode hash;
  memset(&hash, 0, sizeof (hash));

  size_t id = GNUNET_MESSENGER_contact_get_id(contact);
  GNUNET_memcpy(&hash, &id, sizeof (id));

  return &hash;
}

static void
idle (void *cls)
{
  struct GNUNET_MESSENGER_PingTool *tool = cls;

  if ((tool->auto_pong) && (!(tool->quit)))
  {
    tool->task = GNUNET_SCHEDULER_add_delayed_with_priority(
      GNUNET_TIME_relative_multiply(
        GNUNET_TIME_relative_get_second_(), tool->timeout),
      GNUNET_SCHEDULER_PRIORITY_IDLE,
      idle,
      tool
    );
    return;
  }

  tool->task = NULL;
  tool->quit = true;

  if (tool->handle)
    GNUNET_MESSENGER_disconnect(tool->handle);

  if (tool->lookup)
    GNUNET_IDENTITY_ego_lookup_cancel(tool->lookup);
}

static enum GNUNET_GenericReturnValue
member_callback (void *cls,
                 struct GNUNET_MESSENGER_Room *room,
                 const struct GNUNET_MESSENGER_Contact *contact)
{
  struct GNUNET_MESSENGER_Ping *ping = cls;

  if (contact == ping->sender)
    return GNUNET_YES;

  GNUNET_CONTAINER_multishortmap_put(ping->pong_map, hash_contact (contact), NULL,
                                     GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_ONLY);
  
  return GNUNET_YES;
}

static bool
is_hash_following (struct GNUNET_MESSENGER_PingTool *tool,
                   const struct GNUNET_HashCode *hash,
                   const struct GNUNET_HashCode *prev)
{
  if (!prev)
    return false;

  bool first = true;
  while (hash)
  {
    if (0 == GNUNET_CRYPTO_hash_cmp(hash, prev))
      return true;

    if (first)
      first = false;
    else if (0 == GNUNET_CRYPTO_hash_cmp(hash + 1, prev))
      return true;

    hash = GNUNET_CONTAINER_multihashmap_get(tool->map, hash);
  }

  return false;
}

static void
send_ping (struct GNUNET_MESSENGER_PingTool *tool,
           struct GNUNET_MESSENGER_Room *room)
{
  struct GNUNET_MESSENGER_Message message;
  message.header.kind = GNUNET_MESSENGER_KIND_TEXT;
  message.body.text.text = NULL;

  GNUNET_MESSENGER_send_message(room, &message, NULL);
  
  if (tool->count)
    tool->count--;
}

static void
send_pong (struct GNUNET_MESSENGER_PingTool *tool,
           struct GNUNET_MESSENGER_Room *room,
           const struct GNUNET_HashCode *hash,
           const struct GNUNET_TIME_Absolute timestamp)
{
  struct GNUNET_MESSENGER_Message message;
  message.header.kind = GNUNET_MESSENGER_KIND_TAG;
  message.body.tag.tag = NULL;

  GNUNET_memcpy(&(message.body.tag.hash), hash, sizeof(*hash));

  const struct GNUNET_TIME_Relative difference = GNUNET_TIME_absolute_get_difference(
    timestamp, GNUNET_TIME_absolute_get());

  printf("%s as response to %s from: time=%.3f ms\n",
    GNUNET_MESSENGER_name_of_kind(message.header.kind),
    GNUNET_h2s(hash),
    ((float) difference.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us);

  GNUNET_MESSENGER_send_message(room, &message, NULL);

  if (tool->count)
    tool->count--;
  
  if (!(tool->count))
    tool->quit = true;
}

static void
finish_ping (struct GNUNET_MESSENGER_PingTool *tool,
             struct GNUNET_MESSENGER_Ping *ping,
             struct GNUNET_MESSENGER_Room *room,
             const struct GNUNET_HashCode *hash)
{
  const size_t recipients = GNUNET_CONTAINER_multishortmap_size(ping->pong_map);
  const size_t loss_rate = recipients? 100 * ping->pong_missing / recipients : 100;
  const struct GNUNET_TIME_Relative delta = GNUNET_TIME_absolute_get_difference(
    ping->ping_time, GNUNET_TIME_absolute_get());
  
  printf("--- %s ping statistics ---\n", GNUNET_h2s(hash));

  struct GNUNET_TIME_Relative min = GNUNET_TIME_relative_get_forever_();
  struct GNUNET_TIME_Relative avg = GNUNET_TIME_relative_get_zero_();
  struct GNUNET_TIME_Relative max = GNUNET_TIME_relative_get_zero_();
  struct GNUNET_TIME_Relative mdev = GNUNET_TIME_relative_get_zero_();

  struct GNUNET_CONTAINER_MultiShortmapIterator *iter;
  const void *value;

  iter = GNUNET_CONTAINER_multishortmap_iterator_create(ping->pong_map);

  while (GNUNET_NO != GNUNET_CONTAINER_multishortmap_iterator_next(iter, NULL, &value))
  {
    if (!value)
      continue;

    const struct GNUNET_TIME_Absolute *time = value;
    struct GNUNET_TIME_Relative difference = GNUNET_TIME_absolute_get_difference(
      ping->ping_time, *time);
    
    if (GNUNET_TIME_relative_cmp(difference, <, min))
      min = difference;
    if (GNUNET_TIME_relative_cmp(difference, >, max))
      max = difference;

    avg = GNUNET_TIME_relative_add(avg, difference);
  }

  GNUNET_CONTAINER_multishortmap_iterator_destroy(iter);

  if (recipients > ping->pong_missing)
    avg = GNUNET_TIME_relative_divide(avg, recipients - ping->pong_missing);

  iter = GNUNET_CONTAINER_multishortmap_iterator_create(ping->pong_map);

  while (GNUNET_NO != GNUNET_CONTAINER_multishortmap_iterator_next(iter, NULL, &value))
  {
    if (!value)
      continue;

    const struct GNUNET_TIME_Absolute *time = value;
    struct GNUNET_TIME_Relative difference = GNUNET_TIME_absolute_get_difference(
      ping->ping_time, *time);
    
    difference = GNUNET_TIME_relative_subtract(difference, avg);
    difference = GNUNET_TIME_relative_saturating_multiply(difference,
      difference.rel_value_us);
    
    mdev = GNUNET_TIME_relative_add(mdev, difference);
  }

  GNUNET_CONTAINER_multishortmap_iterator_destroy(iter);

  if (recipients > ping->pong_missing)
    mdev = GNUNET_TIME_relative_divide(mdev, recipients - ping->pong_missing);

  mdev.rel_value_us = (uint64_t) sqrt(mdev.rel_value_us);
  
  printf("%lu messages exchanged, %lu recipients, %lu%% message loss, time %.3fms\n",
    ping->traffic, recipients, loss_rate, ((float) delta.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us);
  
  if (recipients > 0)
    printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
      ((float) min.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us,
      ((float) avg.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us,
      ((float) max.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us,
      ((float) mdev.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us);
  
  if (!(tool->quit))
  {
    if (tool->count)
    {
      printf("\n");

      send_ping(tool, room);
    }
    else
      tool->quit = true;
  }
}

static void
message_callback (void *cls,
                  struct GNUNET_MESSENGER_Room *room,
                  const struct GNUNET_MESSENGER_Contact *sender,
                  const struct GNUNET_MESSENGER_Contact *recipient,
                  const struct GNUNET_MESSENGER_Message *message,
                  const struct GNUNET_HashCode *hash,
                  enum GNUNET_MESSENGER_MessageFlags flags)
{
  struct GNUNET_MESSENGER_PingTool *tool = cls;

  if (tool->auto_pong)
  {
    if ((!(GNUNET_MESSENGER_FLAG_SENT & flags)) &&
        (GNUNET_MESSENGER_KIND_TEXT == message->header.kind))
      send_pong(tool, room, hash, GNUNET_TIME_absolute_ntoh(message->header.timestamp));

    goto skip_ping;
  }

  if (GNUNET_YES != GNUNET_CONTAINER_multihashmap_contains(tool->map, hash))
  {
    struct GNUNET_HashCode *copy = GNUNET_malloc(sizeof(struct GNUNET_HashCode) * 2);
    GNUNET_memcpy(copy, &(message->header.previous), sizeof (*copy));

    if (GNUNET_MESSENGER_KIND_MERGE == message->header.kind)
      GNUNET_memcpy(copy + 1, &(message->body.merge.previous), sizeof (*copy));
    else
      GNUNET_memcpy(copy + 1, &(message->header.previous), sizeof (*copy));

    GNUNET_CONTAINER_multihashmap_put(tool->map, hash, copy,
                                      GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST);
  }

  if (GNUNET_MESSENGER_FLAG_SENT & flags)
  {
    switch (message->header.kind)
    {
      case GNUNET_MESSENGER_KIND_JOIN:
      {
        send_ping(tool, room);
        break;
      }
      case GNUNET_MESSENGER_KIND_TEXT:
      {
        struct GNUNET_MESSENGER_Ping *ping = GNUNET_new(struct GNUNET_MESSENGER_Ping);

        ping->ping_time = GNUNET_TIME_absolute_ntoh(message->header.timestamp);
        ping->sender = sender;

        ping->pong_map = GNUNET_CONTAINER_multishortmap_create(8, GNUNET_NO);

        GNUNET_MESSENGER_iterate_members(room, member_callback, ping);

        ping->pong_missing = GNUNET_CONTAINER_multishortmap_size(ping->pong_map);
        ping->traffic = 1;

        GNUNET_CONTAINER_multihashmap_put(tool->ping_map, hash, ping,
                                          GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_ONLY);
        
        if (0 >= ping->pong_missing)
          finish_ping (tool, ping, room, hash);

        break;
      }
      default:
        break;
    }
  }
  else
  {
    struct GNUNET_CONTAINER_MultiHashMapIterator *iter =
      GNUNET_CONTAINER_multihashmap_iterator_create(tool->ping_map);
    
    struct GNUNET_HashCode key;
    const void *value;

    while (GNUNET_NO != GNUNET_CONTAINER_multihashmap_iterator_next(iter, &key, &value))
    {
      struct GNUNET_MESSENGER_Ping *ping = (struct GNUNET_MESSENGER_Ping*) value;

      if (0 >= ping->pong_missing)
        continue;

      ping->traffic++;

      if ((tool->require_pong) && 
          ((GNUNET_MESSENGER_KIND_TAG != message->header.kind) || 
           (0 != GNUNET_CRYPTO_hash_cmp(&(message->body.tag.hash), &key))))
        continue;

      if (NULL != GNUNET_CONTAINER_multishortmap_get(ping->pong_map, hash_contact (sender)))
        continue;

      if (!is_hash_following (tool, hash, &key))
        continue;

      struct GNUNET_TIME_Absolute *time = GNUNET_new(struct GNUNET_TIME_Absolute);
      *time = GNUNET_TIME_absolute_ntoh(message->header.timestamp);

      {
        struct GNUNET_TIME_Relative difference = GNUNET_TIME_absolute_get_difference(
          ping->ping_time, *time);

        printf("%s as response to %s from: sender=%lu time=%.3f ms\n",
          GNUNET_MESSENGER_name_of_kind(message->header.kind),
          GNUNET_h2s(&key),
          GNUNET_MESSENGER_contact_get_id(sender),
          ((float) difference.rel_value_us) / GNUNET_TIME_relative_get_millisecond_().rel_value_us);
      }

      GNUNET_CONTAINER_multishortmap_put(ping->pong_map, hash_contact (sender), time,
                                     GNUNET_CONTAINER_MULTIHASHMAPOPTION_REPLACE);
      
      ping->pong_missing--;
      if (0 < ping->pong_missing)
        continue;

      finish_ping (tool, ping, room, &key);
    }
    
    GNUNET_CONTAINER_multihashmap_iterator_destroy(iter);
  }

skip_ping:
  if (!(tool->quit))
  {
    if (tool->task)
      GNUNET_SCHEDULER_cancel(tool->task);

    tool->task = GNUNET_SCHEDULER_add_delayed_with_priority(
      GNUNET_TIME_relative_multiply(
        GNUNET_TIME_relative_get_second_(), tool->timeout),
      GNUNET_SCHEDULER_PRIORITY_IDLE,
      idle,
      tool
    );
  }
  else if (!(tool->task))
    tool->task = GNUNET_SCHEDULER_add_with_priority(
      GNUNET_SCHEDULER_PRIORITY_IDLE,
      idle,
      tool
    );
  
  if (tool->quit)
    GNUNET_MESSENGER_close_room(room);
}

static void
ego_lookup (void *cls,
            struct GNUNET_IDENTITY_Ego *ego)
{
  struct GNUNET_MESSENGER_PingTool *tool = cls;

  tool->lookup = NULL;

  const struct GNUNET_CRYPTO_PrivateKey *key;
  key = ego? GNUNET_IDENTITY_ego_get_private_key(ego) : NULL;

  tool->handle = GNUNET_MESSENGER_connect(
    tool->cfg,
    tool->ego_name,
    key,
    message_callback,
    tool
  );

  struct GNUNET_PeerIdentity peer;
  GNUNET_CRYPTO_get_peer_identity(
    tool->cfg,
    &peer
  );

  if (tool->auto_pong)
    printf("PONG ");
  else
    printf("PING ");

  printf("%s", GNUNET_i2s(&peer));

  struct GNUNET_HashCode hash;
  if (tool->room_name)
  {
    printf(":%s", tool->room_name);

    GNUNET_CRYPTO_hash(
      tool->room_name,
      strlen(tool->room_name),
      &hash
    );
  }
  else
    memset(&hash, 0, sizeof(hash));

  printf(" (%s): %u times\n",
    GNUNET_h2s(&hash), tool->count);
  
  struct GNUNET_MESSENGER_Room *room;
  room = GNUNET_MESSENGER_enter_room(
    tool->handle,
    &peer,
    &hash
  );

  if (room)
    GNUNET_MESSENGER_use_room_keys(
      room, tool->public_room? GNUNET_NO : GNUNET_YES);
}

static void
run (void *cls,
     char* const* args,
     const char *cfgfile,
     const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  struct GNUNET_MESSENGER_PingTool *tool = cls;

  tool->cfg = cfg;

  if (!(tool->ego_name))
  {
    ego_lookup(tool, NULL);
    return;
  }

  tool->lookup = GNUNET_IDENTITY_ego_lookup(
    cfg,
    tool->ego_name,
    &ego_lookup,
    tool
  );
}

enum GNUNET_GenericReturnValue
free_map_time (void *cls,
               const struct GNUNET_ShortHashCode *key,
               void *value)
{
  struct GNUNET_TIME_Absolute *time = value;

  if (time)
    GNUNET_free(time);

  return GNUNET_YES;
}

enum GNUNET_GenericReturnValue
free_map_ping (void *cls,
               const struct GNUNET_HashCode *key,
               void *value)
{
  struct GNUNET_MESSENGER_Ping *ping = value;

  GNUNET_CONTAINER_multishortmap_iterate(ping->pong_map, free_map_time, NULL);
  GNUNET_CONTAINER_multishortmap_destroy(ping->pong_map);

  GNUNET_free(ping);
  return GNUNET_YES;
}

enum GNUNET_GenericReturnValue
free_map_hashes (void *cls,
                 const struct GNUNET_HashCode *key,
                 void *value)
{
  struct GNUNET_HashCode *hashes = value;
  GNUNET_free(hashes);
  return GNUNET_YES;
}

int
main (int argc,
      char* const* argv)
{
  struct GNUNET_MESSENGER_PingTool tool;
  memset(&tool, 0, sizeof(tool));

  struct GNUNET_GETOPT_CommandLineOption options[] = {
    GNUNET_GETOPT_option_string(
      'e',
      "ego",
      "IDENTITY_NAME",
      "name of identity to send/receive messages with",
      &(tool.ego_name)
    ),
    GNUNET_GETOPT_option_string(
      'r',
      "room",
      "ROOM_NAME",
      "name of room to read messages from",
      &(tool.room_name)
    ),
    GNUNET_GETOPT_option_uint(
      'c',
      "count",
      "<count>",
      "stop after a count of iterations",
      &(tool.count)
    ),
    GNUNET_GETOPT_option_uint(
      't',
      "timeout",
      "<timeout>",
      "stop after a timeout in seconds",
      &(tool.timeout)
    ),
    GNUNET_GETOPT_option_flag(
      'p',
      "public",
      "disable forward secrecy for public rooms",
      &(tool.public_room)
    ),
    GNUNET_GETOPT_option_flag(
      'P',
      "pong",
      "only send back pong messages after a ping",
      &(tool.auto_pong)
    ),
    GNUNET_GETOPT_option_flag(
      'R',
      "require-pong",
      "only react to pong messages after a ping",
      &(tool.require_pong)
    ),
    GNUNET_GETOPT_OPTION_END
  };

  tool.map = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  tool.ping_map = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);

  enum GNUNET_GenericReturnValue result = GNUNET_PROGRAM_run(
    argc,
    argv,
    "gnunet_messenger_ping",
    gettext_noop("A tool to measure latency in the Messenger service of GNUnet."),
    options,
    &run,
    &tool
  );

  GNUNET_CONTAINER_multihashmap_iterate(tool.ping_map, free_map_ping, NULL);
  GNUNET_CONTAINER_multihashmap_iterate(tool.map, free_map_hashes, NULL);

  GNUNET_CONTAINER_multihashmap_destroy(tool.ping_map);
  GNUNET_CONTAINER_multihashmap_destroy(tool.map);

  return GNUNET_OK == result? 0 : 1;
}
