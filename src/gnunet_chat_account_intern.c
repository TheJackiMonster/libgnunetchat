/*
   This file is part of GNUnet.
   Copyright (C) 2026 GNUnet e.V.

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
 * @file gnunet_chat_account_intern.c
 */

#include <gnunet/gnunet_util_lib.h>
#include <libsecret/secret.h>

const SecretSchema * account_secret_schema (void) G_GNUC_CONST;

#define CHAT_ACCOUNT_SECRET_SCHEMA account_secret_schema()
#define CHAT_ACCOUNT_DESC(name) account_secret_description(name)
#define CHAT_ACCOUNT_APP_ID "org.gnunet.Messenger"

const SecretSchema *
account_secret_schema(void)
{
    static const SecretSchema the_schema = {
        "org.gnunet.chat.AccountSecret", SECRET_SCHEMA_NONE,
        {
            {  "name", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  "app_id", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  "NULL", 0 },
        }
    };
    return &the_schema;
}

char*
account_secret_description(const char *name)
{
  char *desc;

  GNUNET_asprintf(
    &desc,
    "GNUnet Messenger account secret for identity %s",
    name
  );

  return desc;
}

static void
handle_account_secret_error(struct GNUNET_CHAT_Account *account,
                            GError *error)
{
  GNUNET_assert(account);

  if (!error)
    return;

  if (error->message)
    handle_send_internal_message(
      account->handle,
      account,
      NULL,
      GNUNET_CHAT_KIND_WARNING,
      error->message,
      GNUNET_YES
    );

  g_error_free(error);
}

static void
on_account_secret_stored (GNUNET_UNUSED GObject *source,
                          GAsyncResult *result,
                          gpointer cls)
{
  GNUNET_assert((result) && (cls));

  struct GNUNET_CHAT_Account *account = cls;
  GError *error = NULL;

  secret_password_store_finish(result, &error);

  if (account->cancellable)
  {
    g_object_unref(account->cancellable);
    account->cancellable = NULL;
  }

  handle_account_secret_error(account, error);
}

static void
on_account_secret_lookup (GNUNET_UNUSED GObject *source,
                          GAsyncResult *result,
                          gpointer cls)
{
  GNUNET_assert((result) && (cls));

  struct GNUNET_CHAT_Account *account = cls;
  GError *error = NULL;

  gchar *password = secret_password_lookup_finish(result, &error);

  if (account->cancellable)
  {
    g_object_unref(account->cancellable);
    account->cancellable = NULL;
  }

  if (error)
    handle_account_secret_error(account, error);
  else if (password)
  {
    GNUNET_CRYPTO_hash(
      password,
      g_utf8_strlen(password, -1),
      &(account->secret)
    );

    secret_password_free(password);
  }
  else
  {
    struct GNUNET_HashCode raw_secret;

    GNUNET_CRYPTO_random_block(
      GNUNET_CRYPTO_QUALITY_STRONG,
      &raw_secret, sizeof(raw_secret)
    );

    const char *secret_value = GNUNET_h2s_full(&raw_secret);
    const char *name = account_get_name(account);

    GNUNET_CRYPTO_hash_from_string(
      secret_value,
      &(account->secret)
    );

    char *desc = CHAT_ACCOUNT_DESC(name);

    secret_password_store(
      CHAT_ACCOUNT_SECRET_SCHEMA,
      SECRET_COLLECTION_DEFAULT,
      desc,
      secret_value,
      account->cancellable,
      on_account_secret_stored,
      account,
      "name", name,
      "app_id", CHAT_ACCOUNT_APP_ID,
      NULL
    );

    if (desc)
      GNUNET_free(desc);
  }
}

static void
drop_account_secret(struct GNUNET_CHAT_Account *account)
{
  GNUNET_assert(account);

  GError *error = NULL;
  const char *name = account_get_name(account);

  secret_password_clear_sync(
    CHAT_ACCOUNT_SECRET_SCHEMA,
    account->cancellable,
    &error,
    "name", name,
    "app_id", CHAT_ACCOUNT_APP_ID,
    NULL
  );

  handle_account_secret_error(account, error);
}
