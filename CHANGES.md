## Version 0.1.0
* This release requires the GNUnet Messenger Service 0.1!
* It allows account management (creation, listing and deletion).
* Clients are able to switch between accounts during runtime.
* The client can rename an account or update its key.
* Contact exchange is possible via lobbies in form of URIs to be shared as text form or potentially QR encoded.
* Each resource allows handling a user pointer for the client application.
* Contacts and groups can be managed individually and given a custom nick name.
* It is possible to request and open a direct chat with any contact.
* Groups allow listing their members with custom user pointers related to the group memberships.
* Chats can be left explicitly.
* Each chat will be represented as context resource abstracting the variant of chat.
* It is possible to send text messages, send files, share files and send read receipts explicitly.
* Received messages allow checking for a read receipt status.
* Messages can be deleted with a custom delay.
* Files in a chat can be fully managed (they can be uploaded, downloaded, unindexed and provide a decrypted temporary preview if necessary) while being encrypted individually.
* The status of each operation (upload, download, unindex) regarding files can be tracked.
* Received invitations to new chats can be accepted.