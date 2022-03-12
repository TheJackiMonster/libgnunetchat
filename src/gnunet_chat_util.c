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
 * @file gnunet_chat_util.c
 */

#include "gnunet_chat_util.h"

void
util_shorthash_from_member (const struct GNUNET_MESSENGER_Contact *member,
			    struct GNUNET_ShortHashCode *shorthash)
{
  GNUNET_assert(shorthash);

  memset(shorthash, 0, sizeof(*shorthash));
  GNUNET_memcpy(shorthash, &member, sizeof(member));
}

void
util_set_name_field (const char *name, char **field)
{
  GNUNET_assert(field);

  if (*field)
    GNUNET_free(*field);

  if (name)
    *field = GNUNET_strdup(name);
  else
    *field = NULL;
}

int
util_hash_file (const char *filename, struct GNUNET_HashCode *hash)
{
  GNUNET_assert((filename) && (hash));

  uint64_t size;

  if (GNUNET_OK != GNUNET_DISK_file_size(filename, &size, GNUNET_NO, GNUNET_YES))
    return GNUNET_SYSERR;

  struct GNUNET_DISK_FileHandle *file = GNUNET_DISK_file_open(
      filename, GNUNET_DISK_OPEN_READ, GNUNET_DISK_PERM_USER_READ
  );

  if (!file)
    return GNUNET_SYSERR;

  struct GNUNET_DISK_MapHandle *mapping;
  const void* data = GNUNET_DISK_file_map(
      file, &mapping, GNUNET_DISK_MAP_TYPE_READ, size
  );

  if (!data)
  {
    GNUNET_DISK_file_close(file);
    return GNUNET_SYSERR;
  }

  GNUNET_CRYPTO_hash(data, size, hash);

  GNUNET_DISK_file_unmap(mapping);
  GNUNET_DISK_file_close(file);

  return GNUNET_OK;
}

int
util_encrypt_file (const char *filename,
		   const struct GNUNET_HashCode *hash,
		   const struct GNUNET_CRYPTO_SymmetricSessionKey *key)
{
  GNUNET_assert((filename) && (hash) && (key));

  uint64_t size;

  if (GNUNET_OK != GNUNET_DISK_file_size(filename, &size, GNUNET_NO, GNUNET_YES))
    return GNUNET_SYSERR;

  struct GNUNET_DISK_FileHandle *file = GNUNET_DISK_file_open(
      filename, GNUNET_DISK_OPEN_READWRITE,
      GNUNET_DISK_PERM_USER_READ | GNUNET_DISK_PERM_USER_WRITE
  );

  if (!file)
    return GNUNET_SYSERR;

  struct GNUNET_DISK_MapHandle *mapping = NULL;
  void* data = GNUNET_DISK_file_map(
      file, &mapping, GNUNET_DISK_MAP_TYPE_READWRITE, size
  );

  if ((!data) || (!mapping))
  {
    GNUNET_DISK_file_close(file);
    return GNUNET_SYSERR;
  }

  struct GNUNET_CRYPTO_SymmetricInitializationVector iv;
  const uint64_t block_size = 1024*1024;
  ssize_t result = -1;

  const uint64_t blocks = ((size + block_size - 1) / block_size);

  for (uint64_t i = 0; i < blocks; i++)
  {
    const uint64_t index = (blocks - i - 1);
    const uint64_t offset = block_size * index;

    const uint64_t remaining = (size - offset);
    void* location = ((uint8_t*) data) + offset;

    if (index > 0)
      memcpy(&iv, ((uint8_t*) data) + (block_size * (index - 1)), sizeof(iv));
    else
      memcpy(&iv, hash, sizeof(iv));

    result = GNUNET_CRYPTO_symmetric_encrypt(
    	location,
    	remaining >= block_size? block_size : remaining,
    	key,
    	&iv,
    	location
    );

    if (result < 0)
      break;
  }

  if (GNUNET_OK != GNUNET_DISK_file_unmap(mapping))
    result = -1;

  if (GNUNET_OK != GNUNET_DISK_file_sync(file))
    result = -1;

  if (GNUNET_OK != GNUNET_DISK_file_close(file))
    result = -1;

  if (result < 0)
    return GNUNET_SYSERR;

  return GNUNET_OK;
}

int
util_decrypt_file (const char *filename,
		   const struct GNUNET_HashCode *hash,
		   const struct GNUNET_CRYPTO_SymmetricSessionKey *key)
{
  GNUNET_assert((filename) && (hash) && (key));

  uint64_t size;

  if (GNUNET_OK != GNUNET_DISK_file_size(filename, &size, GNUNET_NO, GNUNET_YES))
    return GNUNET_SYSERR;

  struct GNUNET_DISK_FileHandle *file = GNUNET_DISK_file_open(
      filename, GNUNET_DISK_OPEN_READWRITE,
      GNUNET_DISK_PERM_USER_READ | GNUNET_DISK_PERM_USER_WRITE
  );

  if (!file)
    return GNUNET_SYSERR;

  struct GNUNET_DISK_MapHandle *mapping = NULL;
  void* data = GNUNET_DISK_file_map(
      file, &mapping, GNUNET_DISK_MAP_TYPE_READWRITE, size
  );

  if ((!data) || (!mapping))
  {
    GNUNET_DISK_file_close(file);
    return GNUNET_SYSERR;
  }

  struct GNUNET_CRYPTO_SymmetricInitializationVector iv;
  const uint64_t block_size = 1024*1024;
  ssize_t result = -1;

  const uint64_t blocks = ((size + block_size - 1) / block_size);

  for (uint64_t index = 0; index < blocks; index++)
  {
    const uint64_t offset = block_size * index;

    const uint64_t remaining = (size - offset);
    void* location = ((uint8_t*) data) + offset;

    if (index > 0)
      memcpy(&iv, ((uint8_t*) data) + (block_size * (index - 1)), sizeof(iv));
    else
      memcpy(&iv, hash, sizeof(iv));

    result = GNUNET_CRYPTO_symmetric_decrypt(
	location,
	remaining >= block_size? block_size : remaining,
	key,
	&iv,
	location
    );

    if (result < 0)
      break;
  }

  struct GNUNET_HashCode check;
  GNUNET_CRYPTO_hash(data, size, &check);

  if (0 != GNUNET_CRYPTO_hash_cmp(hash, &check))
    result = -1;

  if (GNUNET_OK != GNUNET_DISK_file_unmap(mapping))
    result = -1;

  if (GNUNET_OK != GNUNET_DISK_file_sync(file))
    result = -1;

  if (GNUNET_OK != GNUNET_DISK_file_close(file))
    result = -1;

  if (result < 0)
    return GNUNET_SYSERR;

  return GNUNET_OK;
}

int
util_get_dirname (const char *directory,
		  const char *subdir,
		  char **filename)
{
  GNUNET_assert((filename) &&
  		(directory) &&
  		(subdir));

  return GNUNET_asprintf (
      filename,
      "%s%c%s",
      directory,
      DIR_SEPARATOR,
      subdir
  );
}

int
util_get_filename (const char *directory,
		   const char *subdir,
		   const struct GNUNET_HashCode *hash,
		   char **filename)
{
  GNUNET_assert((filename) &&
		(directory) &&
		(subdir) &&
		(hash));

  char* dirname;
  util_get_dirname(directory, subdir, &dirname);

  int result = GNUNET_asprintf (
      filename,
      "%s%c%s",
      dirname,
      DIR_SEPARATOR,
      GNUNET_h2s_full(hash)
  );

  GNUNET_free(dirname);
  return result;
}

int
util_get_context_label (enum GNUNET_CHAT_ContextType type,
		        const struct GNUNET_HashCode *hash,
		        char **label)
{
  const char *type_string = "chat";

  switch (type)
  {
    case GNUNET_CHAT_CONTEXT_TYPE_CONTACT:
      type_string = "contact";
      break;
    case GNUNET_CHAT_CONTEXT_TYPE_GROUP:
      type_string = "group";
      break;
    default:
      break;
  }

  return GNUNET_asprintf (
      label,
      "%s_%s",
      type_string,
      GNUNET_h2s(hash)
  );
}

int util_lobby_name (const struct GNUNET_HashCode *hash,
		     char **name)
{
  return GNUNET_asprintf (
      name,
      "chat_lobby_%s",
      GNUNET_h2s(hash)
  );
}
