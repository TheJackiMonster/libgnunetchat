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
 * @file gnunet_chat_handle.c
 */

#include "gnunet_chat_handle.h"

#include "gnunet_chat_handle_intern.c"

struct GNUNET_CHAT_Handle*
handle_create_from_config (const struct GNUNET_CONFIGURATION_Handle* cfg,
			   const char *directory,
			   const char *name,
			   GNUNET_CHAT_ContextMessageCallback msg_cb,
			   void *msg_cls,
			   GNUNET_CHAT_WarningCallback warn_cb,
			   void *warn_cls)
{
  struct GNUNET_CHAT_Handle* handle = GNUNET_new(struct GNUNET_CHAT_Handle);

  handle->cfg = cfg;

  if ((directory) &&
      (GNUNET_YES == GNUNET_DISK_directory_test(directory, GNUNET_YES)))
    handle->directory = GNUNET_strdup(directory);
  else
    handle->directory = NULL;

  handle->msg_cb = msg_cb;
  handle->msg_cls = msg_cls;

  handle->warn_cb = warn_cb;
  handle->warn_cls = warn_cls;

  handle->files = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  handle->contexts = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);
  handle->contacts = GNUNET_CONTAINER_multishortmap_create(8, GNUNET_NO);
  handle->groups = GNUNET_CONTAINER_multihashmap_create(8, GNUNET_NO);

  handle->arm = GNUNET_ARM_connect(
      handle->cfg,
      on_handle_arm_connection, handle
  );

  if (handle->arm)
    on_handle_arm_connection(handle, GNUNET_NO);

  handle->fs = GNUNET_FS_start(
      handle->cfg, name, // TODO: raw name? (NULL?)
      notify_handle_fs_progress, handle,
      GNUNET_FS_FLAGS_NONE,
      GNUNET_FS_OPTIONS_END
  );

  handle->messenger = GNUNET_MESSENGER_connect(
      handle->cfg, name,
      on_handle_identity, handle,
      on_handle_message, handle
  );

  handle->user_pointer = NULL;

  return handle;
}

void
handle_destroy (struct GNUNET_CHAT_Handle* handle)
{
  if (handle->messenger)
    GNUNET_MESSENGER_disconnect(handle->messenger);

  if (handle->files)
    GNUNET_FS_stop(handle->fs);

  if (handle->arm)
    GNUNET_ARM_disconnect(handle->arm);

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->groups, it_destroy_handle_groups, NULL
  );

  GNUNET_CONTAINER_multishortmap_iterate(
      handle->contacts, it_destroy_handle_contacts, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->contexts, it_destroy_handle_contexts, NULL
  );

  GNUNET_CONTAINER_multihashmap_iterate(
      handle->files, it_destroy_handle_files, NULL
  );

  GNUNET_CONTAINER_multihashmap_destroy(handle->groups);
  GNUNET_CONTAINER_multishortmap_destroy(handle->contacts);
  GNUNET_CONTAINER_multihashmap_destroy(handle->contexts);
  GNUNET_CONTAINER_multihashmap_destroy(handle->files);

  if (handle->directory)
    GNUNET_free(handle->directory);

  GNUNET_free(handle);
}
