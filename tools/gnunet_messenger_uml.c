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
 * @file gnunet_messenger_uml.c
 */

#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_identity_service.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_scheduler_lib.h>
#include <gnunet/gnunet_time_lib.h>
#include <gnunet/gnunet_util_lib.h>
#include <string.h>

struct GNUNET_MESSENGER_Link
{
  struct GNUNET_MESSENGER_Link *prev;
  struct GNUNET_MESSENGER_Link *next;

  struct GNUNET_HashCode hash;
  struct GNUNET_HashCode previous;

  bool dotted;
};

struct GNUNET_MESSENGER_Tool
{
  const struct GNUNET_CONFIGURATION_Handle *cfg;
  struct GNUNET_IDENTITY_EgoLookup *lookup;
  struct GNUNET_MESSENGER_Handle *handle;
  struct GNUNET_SCHEDULER_Task *task;

  struct GNUNET_CONTAINER_MultiHashMap *map;

  struct GNUNET_MESSENGER_Link *head;
  struct GNUNET_MESSENGER_Link *tail;

  char *ego_name;
  char *room_name;
  int ignore_targets;

  bool quit;
};

static void
idle (void *cls)
{
  struct GNUNET_MESSENGER_Tool *tool = cls;

  tool->task = NULL;
  tool->quit = true;

  if (tool->handle)
    GNUNET_MESSENGER_disconnect(tool->handle);

  if (tool->lookup)
    GNUNET_IDENTITY_ego_lookup_cancel(tool->lookup);

  while (tool->head)
  {
    struct GNUNET_MESSENGER_Link *link = tool->head;

    const struct GNUNET_HashCode *hash = &(link->hash);
    const struct GNUNET_HashCode *previous = &(link->previous);

    printf(
      "X%s %s> ",
      GNUNET_h2s(hash),
      link->dotted? ".." : "--"
    );
    
    printf(
      "X%s\n",
      GNUNET_h2s(previous)
    );

    GNUNET_CONTAINER_DLL_remove(tool->head, tool->tail, link);
    GNUNET_free(link);
  }
}

static void
add_link (struct GNUNET_MESSENGER_Tool *tool,
          const struct GNUNET_HashCode *hash,
          const struct GNUNET_HashCode *previous,
          bool dotted)
{
  size_t i;
  for (i = 0; i < sizeof(*previous); i++)
    if (((unsigned char*) previous)[i] != 0)
      break;
  
  if (i == sizeof(*previous))
    return;

  struct GNUNET_MESSENGER_Link *link = GNUNET_new(
    struct GNUNET_MESSENGER_Link
  );

  GNUNET_memcpy(&(link->hash), hash, sizeof(*hash));
  GNUNET_memcpy(&(link->previous), previous, sizeof(*previous));

  link->dotted = dotted;

  GNUNET_CONTAINER_DLL_insert(tool->head, tool->tail, link);
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
  struct GNUNET_MESSENGER_Tool *tool = cls;

  if (GNUNET_YES == GNUNET_CONTAINER_multihashmap_contains(tool->map, hash))
    return;

  if (tool->task)
  {
    GNUNET_SCHEDULER_cancel(tool->task);
    tool->task = NULL;
  }

  printf(
    "json X%s {",
    GNUNET_h2s(hash)
  );

  printf(
    "\n  \"kind\":\"%s\"",
    GNUNET_MESSENGER_name_of_kind(
      message->header.kind
    )
  );

  printf(
    ",\n  \"sender_id\":\"%s\"",
    GNUNET_sh2s(
      &(message->header.sender_id)
    )
  );

  const struct GNUNET_TIME_Absolute timestamp = GNUNET_TIME_absolute_ntoh(
    message->header.timestamp
  );

  printf(
    ",\n  \"timestamp\":\"%s\"",
    GNUNET_STRINGS_absolute_time_to_string(
      timestamp
    )
  );

  if (sender)
  {
    const unsigned long long sender_address = (unsigned long long) sender;
    const char *sender_name = GNUNET_MESSENGER_contact_get_name(
      sender
    );

    if (sender_name)
      printf(
        ",\n  \"sender\":[\"0x%llx\",\"%s\"]",
        sender_address,
        sender_name
      );
    else
      printf(
        ",\n  \"sender\":\"0x%llx\"",
        sender_address
      );
  }

  if (recipient)
  {
    const unsigned long long recipient_address = (unsigned long long) recipient;
    const char *recipient_name = GNUNET_MESSENGER_contact_get_name(
      recipient
    );

    if (recipient_name)
      printf(
        ",\n  \"recipient\":[\"0x%llx\",\"%s\"]",
        recipient_address,
        recipient_name
      );
    else
      printf(
        ",\n  \"recipient\":\"0x%llx\"",
        recipient_address
      );
  }

  switch (message->header.kind)
  {
    case GNUNET_MESSENGER_KIND_PEER:
      printf(
        ",\n  \"peer\":\"%s\"",
        GNUNET_i2s(&(message->body.peer.peer))
      );
      break;
    case GNUNET_MESSENGER_KIND_MISS:
      printf(
        ",\n  \"peer\":\"%s\"",
        GNUNET_i2s(&(message->body.miss.peer))
      );
      break;
    case GNUNET_MESSENGER_KIND_TEXT:
      printf(
        ",\n  \"text\":\"%s\"",
        message->body.text.text
      );
      break;
    case GNUNET_MESSENGER_KIND_FILE:
      printf(
        ",\n  \"file\":[\"%s\",\"%s\"]",
        message->body.file.name,
        message->body.file.uri
      );
      break;
    case GNUNET_MESSENGER_KIND_TAG:
      printf(
        ",\n  \"tag\":\"%s\"",
        message->body.tag.tag
      );
      break;
    default:
      break;
  }

  printf("\n}\n");

  if (GNUNET_MESSENGER_KIND_MERGE == message->header.kind)
  {
    add_link(tool, hash, &(message->body.merge.previous), false);

    GNUNET_MESSENGER_get_message(
      room,
      &(message->body.merge.previous)
    );
  }

  if (0 == tool->ignore_targets)
  {
    if (GNUNET_MESSENGER_KIND_REQUEST == message->header.kind)
      add_link(tool, hash, &(message->body.request.hash), true);

    if (GNUNET_MESSENGER_KIND_DELETION == message->header.kind)
      add_link(tool, hash, &(message->body.deletion.hash), true);

    if (GNUNET_MESSENGER_KIND_TAG == message->header.kind)
      add_link(tool, hash, &(message->body.tag.hash), true);

    if (GNUNET_MESSENGER_KIND_APPEAL == message->header.kind)
      add_link(tool, hash, &(message->body.appeal.event), true);

    if (GNUNET_MESSENGER_KIND_ACCESS == message->header.kind)
      add_link(tool, hash, &(message->body.access.event), true);

    if (GNUNET_MESSENGER_KIND_GROUP == message->header.kind)
    {
      add_link(tool, hash, &(message->body.group.initiator), true);
      add_link(tool, hash, &(message->body.group.partner), true);
    }

    if (GNUNET_MESSENGER_KIND_AUTHORIZATION == message->header.kind)
      add_link(tool, hash, &(message->body.authorization.event), true);
  }

  add_link(tool, hash, &(message->header.previous), false);

  GNUNET_MESSENGER_get_message(
    room,
    &(message->header.previous)
  );

  GNUNET_CONTAINER_multihashmap_put(
    tool->map,
    hash,
    NULL,
    GNUNET_CONTAINER_MULTIHASHMAPOPTION_UNIQUE_FAST
  );

  if ((!(tool->quit)) && (!(tool->task)))
    tool->task = GNUNET_SCHEDULER_add_delayed_with_priority(
      GNUNET_TIME_relative_get_second_(),
      GNUNET_SCHEDULER_PRIORITY_IDLE,
      idle,
      tool
    );
}

static void
ego_lookup (void *cls,
            struct GNUNET_IDENTITY_Ego *ego)
{
  struct GNUNET_MESSENGER_Tool *tool = cls;

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

  struct GNUNET_HashCode hash;
  
  if (tool->room_name)
    GNUNET_CRYPTO_hash(
      tool->room_name,
      strlen(tool->room_name),
      &hash
    );
  else
    memset(&hash, 0, sizeof(hash));
  
  GNUNET_MESSENGER_enter_room(
    tool->handle,
    &peer,
    &hash
  );
}

static void
run (void *cls,
     char* const* args,
     const char *cfgfile,
     const struct GNUNET_CONFIGURATION_Handle *cfg)
{
  struct GNUNET_MESSENGER_Tool *tool = cls;

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

int
main (int argc,
      char* const* argv)
{
  struct GNUNET_MESSENGER_Tool tool;
  memset(&tool, 0, sizeof(tool));

  struct GNUNET_GETOPT_CommandLineOption options[] = {
    GNUNET_GETOPT_option_string(
      'e',
      "ego",
      "IDENTITY_NAME",
      "name of identity to read messages with",
      &(tool.ego_name)
    ),
    GNUNET_GETOPT_option_string(
      'r',
      "room",
      "ROOM_NAME",
      "name of room to read messages from",
      &(tool.room_name)
    ),
    GNUNET_GETOPT_option_flag(
      'i',
      "ignore-targets",
      "ignore indirect connections between messages and their targets",
      &(tool.ignore_targets)
    ),
    GNUNET_GETOPT_OPTION_END
  };

  printf("@startuml\n");

  tool.map = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);

  enum GNUNET_GenericReturnValue result = GNUNET_PROGRAM_run(
    argc,
    argv,
    "gnunet_messenger_uml",
    gettext_noop("A tool to debug the Messenger service of GNUnet."),
    options,
    &run,
    &tool
  );

  GNUNET_CONTAINER_multihashmap_destroy(tool.map);

  printf("@enduml\n");

  return GNUNET_OK == result? 0 : 1;
}
