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
 * @file gnunet_chat_util.h
 */

#ifndef GNUNET_CHAT_UTIL_H_
#define GNUNET_CHAT_UTIL_H_

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_crypto_lib.h>
#include <gnunet/gnunet_disk_lib.h>
#include <gnunet/gnunet_messenger_service.h>
#include <gnunet/gnunet_util_lib.h>

void
util_shorthash_from_member (const struct GNUNET_MESSENGER_Contact *member,
			    struct GNUNET_ShortHashCode *shorthash);

void
util_set_name_field (const char *name, char **field);

int
util_hash_file (const char *filename, struct GNUNET_HashCode *hash);

int
util_encrypt_file (const char *filename,
		   const struct GNUNET_HashCode *hash,
		   const struct GNUNET_CRYPTO_SymmetricSessionKey *key);

int
util_decrypt_file (const char *filename,
		   const struct GNUNET_HashCode *hash,
		   const struct GNUNET_CRYPTO_SymmetricSessionKey *key);

int
util_get_dirname (const char *directory,
		  const char *subdir,
		  char **filename);

int
util_get_filename (const char *directory,
		   const char *subdir,
		   const struct GNUNET_HashCode *hash,
		   char **filename);

#endif /* GNUNET_CHAT_UTIL_H_ */
