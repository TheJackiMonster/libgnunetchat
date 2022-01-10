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
   The development of this code was supported by the NLnet foundation as part
   of the NGI Assure initative to have a more free and secure internet.
 */
/*
 * @author Tobias Frisch
 * @file gnunet_chat_lib.h
 */

#ifndef GNUNET_CHAT_LIB_H_
#define GNUNET_CHAT_LIB_H_

/**
 * @defgroup gnunet_chat GNUnet Chat library
 */
/**@{*/

#include <gnunet/platform.h>
#include <gnunet/gnunet_common.h>
#include <gnunet/gnunet_time_lib.h>
#include <gnunet/gnunet_util_lib.h>

/**
 * Enum for the different kinds of messages.
 */
enum GNUNET_CHAT_MessageKind
{
  /**
   * The kind to inform that something went wrong.
   */
  GNUNET_CHAT_KIND_WARNING = 1,    /**< GNUNET_CHAT_KIND_WARNING */

  /**
   * The kind to inform that the application can be used.
   */
  GNUNET_CHAT_KIND_LOGIN = 2,      /**< GNUNET_CHAT_KIND_LOGIN */

  /**
   * The kind to inform that a context was updated.
   */
  GNUNET_CHAT_KIND_UPDATE = 3,      /**< GNUNET_CHAT_KIND_UPDATE */

  /**
   * The kind to inform that a contact has joined a chat.
   */
  GNUNET_CHAT_KIND_JOIN = 4,       /**< GNUNET_CHAT_KIND_JOIN */

  /**
   * The kind to inform that a contact has left a chat.
   */
  GNUNET_CHAT_KIND_LEAVE = 5,      /**< GNUNET_CHAT_KIND_LEAVE */

  /**
   * The kind to inform that a contact has changed.
   */
  GNUNET_CHAT_KIND_CONTACT = 6,    /**< GNUNET_CHAT_KIND_CONTACT */

  /**
   * The kind to describe an invitation to a different chat.
   */
  GNUNET_CHAT_KIND_INVITATION = 7, /**< GNUNET_CHAT_KIND_INVITATION */

  /**
   * The kind to describe a text message.
   */
  GNUNET_CHAT_KIND_TEXT = 8,       /**< GNUNET_CHAT_KIND_TEXT */

  /**
   * The kind to describe a shared file.
   */
  GNUNET_CHAT_KIND_FILE = 9,       /**< GNUNET_CHAT_KIND_FILE */

  /**
   * The kind to inform about a deletion of a previous message.
   */
  GNUNET_CHAT_KIND_DELETION = 10,  /**< GNUNET_CHAT_KIND_DELETION */

  /**
   * An unknown kind of message.
   */
  GNUNET_CHAT_KIND_UNKNOWN = 0     /**< GNUNET_CHAT_KIND_UNKNOWN */
};

/**
 * Struct of a chat handle.
 */
struct GNUNET_CHAT_Handle;

/**
 * Struct of a chat contact.
 */
struct GNUNET_CHAT_Contact;

/**
 * Struct of a chat group.
 */
struct GNUNET_CHAT_Group;

/**
 * Struct of a chat context.
 */
struct GNUNET_CHAT_Context;

/**
 * Struct of a chat message.
 */
struct GNUNET_CHAT_Message;

/**
 * Struct of a chat file.
 */
struct GNUNET_CHAT_File;

/**
 * Struct of a chat invitation.
 */
struct GNUNET_CHAT_Invitation;

/**
 * Iterator over chat contacts of a specific chat handle.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_iterate_contacts
 * @param[in,out] handle Chat handle
 * @param[in,out] contact Chat contact
 * @return #GNUNET_YES if we should continue to iterate, #GNUNET_NO otherwise.
 */
typedef int
(*GNUNET_CHAT_ContactCallback) (void *cls,
                                struct GNUNET_CHAT_Handle *handle,
				struct GNUNET_CHAT_Contact *contact);

/**
 * Iterator over chat groups of a specific chat handle.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_iterate_groups
 * @param[in,out] handle Chat handle
 * @param[in,out] group Chat group
 * @return #GNUNET_YES if we should continue to iterate, #GNUNET_NO otherwise.
 */
typedef int
(*GNUNET_CHAT_GroupCallback) (void *cls,
                              struct GNUNET_CHAT_Handle *handle,
			      struct GNUNET_CHAT_Group *group);

/**
 * Iterator over chat contacts in a specific chat group.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_group_iterate_contacts
 * @param[in,out] group Chat group
 * @param[in,out] contact Chat contact
 * @return #GNUNET_YES if we should continue to iterate, #GNUNET_NO otherwise.
 */
typedef int
(*GNUNET_CHAT_GroupContactCallback) (void *cls,
                                     const struct GNUNET_CHAT_Group *group,
                                     struct GNUNET_CHAT_Contact *contact);

/**
 * Iterator over chat messages in a specific chat context.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_context_iterate_messages
 * @param[in,out] context Chat context or NULL
 * @param[in] message Chat message
 * @return #GNUNET_YES if we should continue to iterate, #GNUNET_NO otherwise.
 */
typedef int
(*GNUNET_CHAT_ContextMessageCallback) (void *cls,
                                       struct GNUNET_CHAT_Context *context,
				       const struct GNUNET_CHAT_Message *message);

/**
 * Iterator over chat files in a specific chat context.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_context_iterate_files
 * @param[in,out] context Chat context
 * @param[in,out] file Chat file
 * @return #GNUNET_YES if we should continue to iterate, #GNUNET_NO otherwise.
 */
typedef int
(*GNUNET_CHAT_ContextFileCallback) (void *cls,
                                    struct GNUNET_CHAT_Context *context,
				    struct GNUNET_CHAT_File *file);

/**
 * Iterator over chat contacts in a chat to check whether they received a
 * specific message or not.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_message_get_read_receipt
 * @param[in] message Chat message
 * @param[in] contact Chat contact
 * @param[in] read_receipt #GNUNET_YES if the message was received by the contact,
 * 			   #GNUNET_NO otherwise
 * @return #GNUNET_YES if we should continue to iterate, #GNUNET_NO otherwise.
 */
typedef int
(*GNUNET_CHAT_MessageReadReceiptCallback) (void *cls,
                                           const struct GNUNET_CHAT_Message *message,
					   const struct GNUNET_CHAT_Contact *contact,
					   int read_receipt);

/**
 * Method called during an upload of a specific file in a chat to share it.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_context_send_file
 * @param[in] file Chat file
 * @param[in] completed Amount of the file being uploaded (in bytes)
 * @param[in] size Full size of the uploading file (in bytes)
 */
typedef void
(*GNUNET_CHAT_FileUploadCallback) (void *cls,
                                   const struct GNUNET_CHAT_File *file,
				   uint64_t completed,
				   uint64_t size);

/**
 * Method called during a download of a specific file in a chat which was shared.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_file_start_download
 * @param[in] file Chat file
 * @param[in] completed Amount of the file being downloaded (in bytes)
 * @param[in] size Full size of the downloading file (in bytes)
 */
typedef void
(*GNUNET_CHAT_FileDownloadCallback) (void *cls,
                                     const struct GNUNET_CHAT_File *file,
				     uint64_t completed,
				     uint64_t size);

/**
 * Method called during an unindexing of a specific file in a chat which was
 * uploaded before.
 *
 * @param[in,out] cls Closure from #GNUNET_CHAT_file_unindex
 * @param[in,out] file Chat file
 * @param[in] completed Amount of the file being unindexed (in bytes)
 * @param[in] size Full size of the unindexing file (in bytes)
 */
typedef void
(*GNUNET_CHAT_FileUnindexCallback) (void *cls,
                                    struct GNUNET_CHAT_File *file,
				    uint64_t completed,
				    uint64_t size);

/**
 * Start a chat handle with a certain configuration, an application <i>directory</i>
 * and a selected user <i>name</i>.
 *
 * A custom callback for warnings and message events can be provided optionally
 * together with their respective closures.
 *
 * @param[in] cfg Configuration
 * @param[in] directory Application directory path (optional)
 * @param[in] name User name (optional)
 * @param[in] msg_cb Callback for message events (optional)
 * @param[in,out] msg_cls Closure for message events (optional)
 * @return Chat handle
 */
struct GNUNET_CHAT_Handle*
GNUNET_CHAT_start (const struct GNUNET_CONFIGURATION_Handle *cfg,
		   const char *directory,
		   const char *name,
		   GNUNET_CHAT_ContextMessageCallback msg_cb, void *msg_cls);

/**
 * Stops a chat handle closing all its remaining resources and frees the
 * regarding memory.
 *
 * @param[in,out] handle Chat handle
 */
void
GNUNET_CHAT_stop (struct GNUNET_CHAT_Handle *handle);

/**
 * Updates a chat handle to renew the used ego to sign sent messages in active
 * chats.
 *
 * Updating the chat handle should only be used if necessary because the usage
 * can require renewed exchanging of GNS entries.
 *
 * @param[in,out] handle Chat handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_update (struct GNUNET_CHAT_Handle *handle);

/**
 * Updates the name of a chat handle for related communication.
 *
 * @param[in,out] handle Chat handle
 * @param[in] name New name or NULL
 * @return #GNUNET_YES on success, #GNUNET_NO on failure and #GNUNET_SYSERR if <i>handle</i> is NULL
 */
int
GNUNET_CHAT_set_name (struct GNUNET_CHAT_Handle *handle,
		      const char *name);

/**
 * Returns the name of a chat handle for related communication or NULL if no
 * name is set.
 *
 * @param[in] handle Chat handle
 * @return Name used by the handle or NULL
 */
const char*
GNUNET_CHAT_get_name (const struct GNUNET_CHAT_Handle *handle);

/**
 * Returns the public key of the used ego to verify signatures of sent messages.
 *
 * @param[in] handle Chat handle
 * @return Public key of the handles ego or NULL
 */
const char*
GNUNET_CHAT_get_key (const struct GNUNET_CHAT_Handle *handle);

/**
 * Iterates through the contacts of a given chat <i>handle</i> with a selected
 * callback and custom closure.
 *
 * @param[in,out] handle Chat handle
 * @param[in] callback Callback for contact iteration (optional)
 * @param[in,out] cls Closure for contact iteration (optional)
 * @return Amount of contacts iterated or #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_iterate_contacts (struct GNUNET_CHAT_Handle *handle,
			      GNUNET_CHAT_ContactCallback callback,
			      void *cls);

/**
 * Creates a new group chat to use with a given chat <i>handle</i> with an
 * optional public <i>topic</i>.
 *
 * If a specific <i>topic</i> is used, the created group will be publically
 * available to join for others searching for the used topic. Otherwise the
 * group will be private using a randomly generated key and others can only
 * join the chat via a private invitation.
 *
 * @param[in,out] handle Chat handle
 * @return New group chat
 */
struct GNUNET_CHAT_Group *
GNUNET_CHAT_group_create (struct GNUNET_CHAT_Handle *handle,
			  const char* topic);

/**
 * Iterates through the groups of a given chat <i>handle</i> with a selected
 * callback and custom closure.
 *
 * @param[in,out] handle Chat handle
 * @param[in] callback Callback for group iteration (optional)
 * @param[in,out] cls Closure for group iteration (optional)
 * @return Amount of groups iterated or #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_iterate_groups (struct GNUNET_CHAT_Handle *handle,
			    GNUNET_CHAT_GroupCallback callback,
			    void *cls);

/**
 * Leaves the private chat with a specific <i>contact</i> and frees the
 * regarding memory of the contact if there remains no common chat with it.
 *
 * @param[in,out] contact Cntact
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_contact_delete (struct GNUNET_CHAT_Contact *contact);

/**
 * Overrides the name of a given <i>contact</i> with a custom nick <i>name</i>
 * which will be only used locally.
 *
 * @param[in,out] contact Contact
 * @param[in] name Custom nick name
 */
void
GNUNET_CHAT_contact_set_name (struct GNUNET_CHAT_Contact *contact,
			      const char *name);

/**
 * Returns the provided name of a given <i>contact</i> or its custom nick name
 * if it was overriden.
 *
 * @param[in] contact Contact
 * @return Name or custom nick name
 */
const char*
GNUNET_CHAT_contact_get_name (const struct GNUNET_CHAT_Contact *contact);

/**
 * Returns the public key of the used ego by a specific <i>contact</i> to
 * verify signatures of sent messages.
 *
 * @param[in] contact Contact
 * @return Public key of the contacts ego or NULL
 */
const char*
GNUNET_CHAT_contact_get_key (const struct GNUNET_CHAT_Contact *contact);

/**
 * Returns the chat context for a private chat with a given <i>contact</i>.
 *
 * @param[in,out] contact Contact
 * @return Chat context
 */
struct GNUNET_CHAT_Context*
GNUNET_CHAT_contact_get_context (struct GNUNET_CHAT_Contact *contact);

/**
 * Sets a custom <i>user pointer</i> to a given <i>contact</i> so it can be
 * accessed in contact related callbacks.
 *
 * @param[in,out] contact Contact
 * @param[in] user_pointer Custom user pointer
 */
void
GNUNET_CHAT_contact_set_user_pointer (struct GNUNET_CHAT_Contact *contact,
				      void *user_pointer);

/**
 * Returns the custom user pointer of a given <i>contact</i> or NULL if it was
 * not set any.
 *
 * @param[in] contact Contact
 * @return Custom user pointer or NULL
 */
void*
GNUNET_CHAT_contact_get_user_pointer (const struct GNUNET_CHAT_Contact *contact);

/**
 * Returns if a given <i>contact</i> is owned as account and whether it has
 * sent messages with.
 *
 * @param[in] contact Contact
 * @return GNUNET_YES if the contact is owned, otherwise GNUNET_NO
 *         and GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_contact_is_owned (const struct GNUNET_CHAT_Contact *contact);

/**
 * Leaves a specific <i>group</i> chat and frees its memory if it is not shared
 * with other groups or contacts.
 *
 * @param[in,out] group Group
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_group_leave (struct GNUNET_CHAT_Group *group);

/**
 * Sets the name of a given <i>group</i> to a custom nick <i>name</i>
 * which will be only used locally.
 *
 * @param[in,out] group Group
 * @param[in] name Custom nick name
 */
void
GNUNET_CHAT_group_set_name (struct GNUNET_CHAT_Group *group,
			    const char *name);

/**
 * Returns the custom nick name of a given <i>group</i> if it was overriden.
 *
 * @param[in] group Group
 * @return Custom nick name or NULL
 */
const char*
GNUNET_CHAT_group_get_name (const struct GNUNET_CHAT_Group *group);

/**
 * Sets a custom <i>user pointer</i> to a given <i>group</i> so it can be
 * accessed in group related callbacks.
 *
 * @param[in,out] group Group
 * @param[in] user_pointer Custom user pointer
 */
void
GNUNET_CHAT_group_set_user_pointer (struct GNUNET_CHAT_Group *group,
				    void *user_pointer);

/**
 * Returns the custom user pointer of a given <i>group</i> or NULL if it was
 * not set any.
 *
 * @param[in] group Group
 * @return Custom user pointer or NULL
 */
void*
GNUNET_CHAT_group_get_user_pointer (const struct GNUNET_CHAT_Group *group);

/**
 * Invites a specific <i>contact</i> to a given <i>group</i> via a privately
 * sent invitation.
 *
 * @param[in,out] group Group
 * @param[in,out] contact Contact
 */
void
GNUNET_CHAT_group_invite_contact (const struct GNUNET_CHAT_Group *group,
				  struct GNUNET_CHAT_Contact *contact);

/**
 * Iterates through the contacts of a given <i>group</i> with a selected
 * callback and custom closure.
 *
 * @param[in,out] group Group
 * @param[in] callback Callback for contact iteration (optional)
 * @param[in,out] cls Closure for contact iteration (optional)
 * @return Amount of contacts iterated or #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_group_iterate_contacts (const struct GNUNET_CHAT_Group *group,
				    GNUNET_CHAT_GroupContactCallback callback,
				    void *cls);

/**
 * Returns the chat context for a chat with a given <i>group</i>.
 *
 * @param[in,out] group Group
 * @return Chat context
 */
struct GNUNET_CHAT_Context*
GNUNET_CHAT_group_get_context (struct GNUNET_CHAT_Group *group);

/**
 * Returns the current status of a given <i>context</i> whether it is usable
 * or not to communicate with other contacts.
 *
 * @param[in] context Context
 * @return #GNUNET_OK if usable, #GNUNET_NO if the context has been requested,
 *         #GNUNET_SYSERR otherwise.
 */
int
GNUNET_CHAT_context_get_status (const struct GNUNET_CHAT_Context *context);

/**
 * Requests a <i>context</i> to get established between all required contacts.
 * The current status of this request can be tracked via
 * #GNUNET_CHAT_context_get_status() and will only change through the
 * receivement of new messages.
 *
 * @param[in,out] context Context
 */
void
GNUNET_CHAT_context_request (struct GNUNET_CHAT_Context *context);

/**
 * Returns the chat contact which uses a given <i>context</i>.
 *
 * @param[in] context Context
 * @return Chat contact
 */
const struct GNUNET_CHAT_Contact*
GNUNET_CHAT_context_get_contact (const struct GNUNET_CHAT_Context *context);

/**
 * Returns the chat group which uses a given <i>context</i>.
 *
 * @param[in] context Context
 * @return Chat group
 */
const struct GNUNET_CHAT_Group*
GNUNET_CHAT_context_get_group (const struct GNUNET_CHAT_Context *context);

/**
 * Sets a custom <i>user pointer</i> to a given chat <i>context</i> so it can
 * be accessed in chat context related callbacks.
 *
 * @param[in,out] context Chat context
 * @param[in] user_pointer Custom user pointer
 */
void
GNUNET_CHAT_context_set_user_pointer (struct GNUNET_CHAT_Context *context,
				      void *user_pointer);

/**
 * Returns the custom user pointer of a given chat <i>context</i> or NULL if it
 * was not set any.
 *
 * @param[in] context Chat context
 * @return Custom user pointer or NULL
 */
void*
GNUNET_CHAT_context_get_user_pointer (const struct GNUNET_CHAT_Context *context);

/**
 * Sends a selected <i>text</i> into a given chat <i>context</i>.
 *
 * @param[in,out] context Chat context
 * @param[in] text Text
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_context_send_text (struct GNUNET_CHAT_Context *context,
			       const char *text);

/**
 * Uploads a local file specified via its <i>path</i> using symmetric encryption
 * and shares the regarding information to download and decrypt it in a given
 * chat <i>context</i>.
 *
 * @param[in,out] context Chat context
 * @param[in] path Local file path
 * @param[in] callback Callback for file uploading (optional)
 * @param[in,out] cls Closure for file uploading (optional)
 * @return The file handle on success, NULL on failure
 */
struct GNUNET_CHAT_File*
GNUNET_CHAT_context_send_file (struct GNUNET_CHAT_Context *context,
			       const char *path,
			       GNUNET_CHAT_FileUploadCallback callback,
			       void *cls);

/**
 * Shares the information to download and decrypt a specific <i>file</i> from
 * another chat in a given chat <i>context</i>.
 *
 * @param[in,out] context Chat context
 * @param[in] file File handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_context_share_file (struct GNUNET_CHAT_Context *context,
				const struct GNUNET_CHAT_File *file);

/**
 * Iterates through the contacts of a given chat <i>context</i> with a selected
 * callback and custom closure.
 *
 * @param[in,out] context Chat context
 * @param[in] callback Callback for contact iteration (optional)
 * @param[in,out] cls Closure for contact iteration (optional)
 * @return Amount of contacts iterated or #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_context_iterate_messages (struct GNUNET_CHAT_Context *context,
				      GNUNET_CHAT_ContextMessageCallback callback,
				      void *cls);

/**
 * Iterates through the files of a given chat <i>context</i> with a selected
 * callback and custom closure.
 *
 * @param[in,out] context Chat context
 * @param[in] callback Callback for file iteration (optional)
 * @param[in,out] cls Closure for file iteration (optional)
 * @return Amount of files iterated or #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_context_iterate_files (struct GNUNET_CHAT_Context *context,
				   GNUNET_CHAT_ContextFileCallback callback,
				   void *cls);

/**
 * Returns the kind of a given <i>message</i> to determine its content and
 * related usage.
 *
 * @param[in] message Message
 * @return The kind of message
 */
enum GNUNET_CHAT_MessageKind
GNUNET_CHAT_message_get_kind (const struct GNUNET_CHAT_Message *message);

/**
 * Returns the timestamp of a given <i>message</i> in absolute measure.
 *
 * @param[in] message Message
 * @return The timestamp of message
 */
struct GNUNET_TIME_Absolute
GNUNET_CHAT_message_get_timestamp (const struct GNUNET_CHAT_Message *message);

/**
 * Returns the contact of the sender from a given <i>message</i>.
 *
 * @param[in] message Message
 * @return Contact of the messages sender
 */
struct GNUNET_CHAT_Contact*
GNUNET_CHAT_message_get_sender (const struct GNUNET_CHAT_Message *message);

/**
 * Returns #GNUNET_YES if the message was sent by the related chat handle,
 * otherwise it returns #GNUNET_NO.
 *
 * @param[in] message Message
 * @return #GNUNET_YES if the message was sent, otherwise #GNUNET_NO
 */
int
GNUNET_CHAT_message_is_sent (const struct GNUNET_CHAT_Message *message);

/**
 * Returns #GNUNET_YES if the message was received privately encrypted by the
 * related chat handle, otherwise it returns #GNUNET_NO.
 *
 * @param[in] message Message
 * @return #GNUNET_YES if the message was privately received,
 * 	   otherwise #GNUNET_NO
 */
int
GNUNET_CHAT_message_is_private (const struct GNUNET_CHAT_Message *message);

/**
 * Iterates through the contacts of the context related to a given chat
 * <i>message</i> to check whether it was received by each of the contacts.
 *
 * @param[in] message Message
 * @param[in] callback Callback for contact iteration (optional)
 * @param[in,out] cls Closure for contact iteration (optional)
 * @return Amount of contacts iterated or #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_message_get_read_receipt (const struct GNUNET_CHAT_Message *message,
				      GNUNET_CHAT_MessageReadReceiptCallback callback,
				      void *cls);

/**
 * Returns the text of a given <i>message</i> if its kind is
 * #GNUNET_CHAT_KIND_TEXT or #GNUNET_CHAT_KIND_WARNING,
 * otherwise it returns NULL.
 *
 * @param[in] message Message
 * @return The text of message or NULL
 */
const char*
GNUNET_CHAT_message_get_text (const struct GNUNET_CHAT_Message *message);


/**
 * Returns the file handle of a given <i>message</i> if its kind is
 * #GNUNET_CHAT_KIND_FILE, otherwise it returns NULL.
 *
 * @param[in] message Message
 * @return The file handle of message or NULL
 */
struct GNUNET_CHAT_File*
GNUNET_CHAT_message_get_file (const struct GNUNET_CHAT_Message *message);

/**
 * Returns the invitation of a given <i>message</i> if its kind is
 * #GNUNET_CHAT_KIND_INVITATION, otherwise it returns NULL.
 *
 * @param[in] message Message
 * @return The invitation of message or NULL
 */
struct GNUNET_CHAT_Invitation*
GNUNET_CHAT_message_get_invitation (const struct GNUNET_CHAT_Message *message);

/**
 * Deletes a given <i>message</i> with a specific relative <i>delay</i>.
 *
 * @param[in] message Message
 * @param[in] delay Relative delay
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_message_delete (const struct GNUNET_CHAT_Message *message,
			    struct GNUNET_TIME_Relative delay);

/**
 * Returns the name of a given <i>file</i> handle.
 *
 * @param[in] file File handle
 * @return The file name of file
 */
const char*
GNUNET_CHAT_file_get_name (const struct GNUNET_CHAT_File *file);

/**
 * Returns the hash of a given <i>file</i> handle.
 *
 * @param[in] file File handle
 * @return The hash of file
 */
const struct GNUNET_HashCode*
GNUNET_CHAT_file_get_hash (const struct GNUNET_CHAT_File *file);

/**
 * Returns the file size of a given <i>file</i> handle.
 *
 * @param[in] file File handle
 * @return The file size of file
 */
uint64_t
GNUNET_CHAT_file_get_size (const struct GNUNET_CHAT_File *file);

/**
 * Checks whether a file locally exists of a given <i>file</i> handle and
 * returns #GNUNET_YES in that case, otherwise #GNUNET_NO.
 *
 * @param[in] file File handle
 * @return #GNUNET_YES if the file exists locally, otherwise #GNUNET_NO
 */
int
GNUNET_CHAT_file_is_local (const struct GNUNET_CHAT_File *file);

/**
 * Sets a custom <i>user pointer</i> to a given <i>file</i> handle so it can
 * be accessed in file related callbacks.
 *
 * @param[in,out] file File handle
 * @param[in] user_pointer Custom user pointer
 */
void
GNUNET_CHAT_file_set_user_pointer (struct GNUNET_CHAT_File *file,
				   void *user_pointer);

/**
 * Returns the custom user pointer of a given <i>file</i> handle or NULL if it
 * was not set any.
 *
 * @param[in] file File handle
 * @return Custom user pointer
 */
void*
GNUNET_CHAT_file_get_user_pointer (const struct GNUNET_CHAT_File *file);

/**
 * Starts downloading a file from a given <i>file</i> handle and sets up a
 * selected callback and custom closure for its progress.
 *
 * @param[in,out] file File handle
 * @param[in] callback Callback for file downloading (optional)
 * @param[in,out] cls Closure for file downloading (optional)
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_file_start_download (struct GNUNET_CHAT_File *file,
				 GNUNET_CHAT_FileDownloadCallback callback,
				 void *cls);

/**
 * Pause downloading a file from a given <i>file</i> handle.
 *
 * @param[in,out] file File handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_file_pause_download (struct GNUNET_CHAT_File *file);

/**
 * Resume downloading a file from a given <i>file</i> handle.
 *
 * @param[in,out] file File handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_file_resume_download (struct GNUNET_CHAT_File *file);

/**
 * Stops downloading a file from a given <i>file</i> handle.
 *
 * @param[in,out] file File handle
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_file_stop_download (struct GNUNET_CHAT_File *file);

/**
 * Unindexes an uploaded file from a given <i>file</i> handle with a selected
 * callback and a custom closure.
 *
 * @param[in,out] file File handle
 * @param[in] callback Callback for file unindexing (optional)
 * @param[in,out] cls Closure for file unindexing (optional)
 * @return #GNUNET_OK on success, #GNUNET_SYSERR on failure
 */
int
GNUNET_CHAT_file_unindex (struct GNUNET_CHAT_File *file,
			  GNUNET_CHAT_FileUnindexCallback callback,
			  void *cls);

/**
 * Accepts a given chat <i>invitation</i> to enter another chat.
 *
 * @param[in,out] invitation Chat invitation
 */
void
GNUNET_CHAT_invitation_accept (struct GNUNET_CHAT_Invitation *invitation);

/**@}*/

#endif /* GNUNET_CHAT_LIB_H_ */
