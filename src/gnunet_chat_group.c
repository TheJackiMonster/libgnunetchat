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
 * @file gnunet_chat_group.c
 */

#include "gnunet_chat_group.h"
#include "gnunet_chat_util.h"

#include "gnunet_chat_group_intern.c"

struct GNUNET_CHAT_Group*
group_create_from_context (struct GNUNET_CHAT_Handle *handle,
			   struct GNUNET_CHAT_Context *context)
{
  GNUNET_assert((handle) && (context));

  struct GNUNET_CHAT_Group* group = GNUNET_new(struct GNUNET_CHAT_Group);

  group->handle = handle;
  group->context = context;

  group->topic = NULL;

  group->announcement = NULL;
  group->search = NULL;

  group->registry = GNUNET_CONTAINER_multipeermap_create(8, GNUNET_NO);

  group->user_pointer = NULL;

  return group;
}

void
group_destroy (struct GNUNET_CHAT_Group* group)
{
  GNUNET_assert(group);

  if (group->registry)
    GNUNET_CONTAINER_multipeermap_destroy(group->registry);

  if (group->search)
    GNUNET_REGEX_search_cancel(group->search);

  if (group->announcement)
    GNUNET_REGEX_announce_cancel(group->announcement);

  if (group->topic)
    GNUNET_free(group->topic);

  GNUNET_free(group);
}

void
group_publish (struct GNUNET_CHAT_Group* group)
{
  GNUNET_assert((group) &&
		(group->topic) &&
		(group->handle) &&
		(group->handle->cfg));

  char* topic = NULL;
  GNUNET_asprintf (
      &topic,
      "GNUNET_CHAT_%s",
      group->topic
  );

  group->announcement = GNUNET_REGEX_announce(
      group->handle->cfg, topic,
      GNUNET_TIME_relative_get_minute_(),
      6
  );

  group->search = GNUNET_REGEX_search(
      group->handle->cfg, topic,
      search_group_by_topic, group
  );

  GNUNET_free(topic);
}

void
group_load_config (struct GNUNET_CHAT_Group *group)
{
  GNUNET_assert((group) && (group->handle));

  const char *directory = group->handle->directory;

  if ((!directory) || (!(group->context)))
    return;

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(
      group->context->room
  );

  char* filename;
  util_get_filename(directory, "groups", key, &filename);

  if (GNUNET_YES != GNUNET_DISK_file_test(filename))
    goto free_filename;

  struct GNUNET_CONFIGURATION_Handle *config = GNUNET_CONFIGURATION_create();

  if (GNUNET_OK != GNUNET_CONFIGURATION_load(config, filename))
    goto destroy_config;

  char* name = NULL;

  if (GNUNET_OK == GNUNET_CONFIGURATION_get_value_string(
      config, "group", "topic", &name))
    util_set_name_field(name, &(group->topic));

  if (name)
    GNUNET_free(name);

destroy_config:
  GNUNET_CONFIGURATION_destroy(config);

free_filename:
  GNUNET_free(filename);
}

void
group_save_config (const struct GNUNET_CHAT_Group *group)
{
  GNUNET_assert((group) && (group->handle));

  const char *directory = group->handle->directory;

  if ((!directory) || (!(group->context)))
    return;

  const struct GNUNET_HashCode *key = GNUNET_MESSENGER_room_get_key(
      group->context->room
  );

  struct GNUNET_CONFIGURATION_Handle *config = GNUNET_CONFIGURATION_create();

  if (group->topic)
  {
    struct GNUNET_HashCode topic_hash;
    GNUNET_CRYPTO_hash(group->topic, strlen(group->topic), &topic_hash);

    if (0 == GNUNET_memcmp(key, &topic_hash))
      GNUNET_CONFIGURATION_set_value_string(
	config, "group", "topic", group->topic
      );
  }

  char* filename;
  util_get_filename(directory, "groups", key, &filename);

  if (GNUNET_OK == GNUNET_DISK_directory_create_for_file(filename))
    GNUNET_CONFIGURATION_write(config, filename);

  GNUNET_CONFIGURATION_destroy(config);

  GNUNET_free(filename);
}
