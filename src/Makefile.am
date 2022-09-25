# This Makefile.am is in the public domain
AM_CFLAGS = -I$(top_srcdir)/include/gnunet

lib_LTLIBRARIES = libgnunetchat.la

libgnunetchat_la_SOURCES = \
  gnunet_chat_account.c gnunet_chat_account.h \
  gnunet_chat_contact.c gnunet_chat_contact.h \
  gnunet_chat_context.c gnunet_chat_context.h \
  gnunet_chat_file.c gnunet_chat_file.h \
  gnunet_chat_group.c gnunet_chat_group.h \
  gnunet_chat_handle.c gnunet_chat_handle.h \
  gnunet_chat_invitation.c gnunet_chat_invitation.h \
  gnunet_chat_lobby.c gnunet_chat_lobby.h \
  gnunet_chat_message.c gnunet_chat_message.h \
  gnunet_chat_uri.c gnunet_chat_uri.h \
  gnunet_chat_util.c gnunet_chat_util.h \
  gnunet_chat_lib.c

libgnunetchat_la_LIBADD = \
  -lgnunetarm \
  -lgnunetfs \
  -lgnunetidentity \
  -lgnunetgns \
  -lgnunetmessenger \
  -lgnunetnamestore \
  -lgnunetregex \
  -lgnunetutil

