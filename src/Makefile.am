# This Makefile.am is in the public domain
AM_CFLAGS = -I$(top_srcdir)/include/gnunet

EXTRA_DIST = \
  gnunet_chat_contact_intern.c \
  gnunet_chat_context_intern.c \
  gnunet_chat_group_intern.c \
  gnunet_chat_handle_intern.c \
  gnunet_chat_lib_intern.c \
  gnunet_chat_lobby_intern.c

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
  -lgnunetgns \
  -lgnunetidentity \
  -lgnunetmessenger \
  -lgnunetnamestore \
  -lgnunetregex \
  -lgnunetutil

libgnunetchat_la_CFLAGS = \
  -fPIC -pedantic -Wall -Wextra

if DEBUG
libgnunetchat_la_CFLAGS += \
  -O0 -D _DEBUG -ggdb3
else
libgnunetchat_la_CFLAGS += \
  -O2 -D NDEBUG
endif
