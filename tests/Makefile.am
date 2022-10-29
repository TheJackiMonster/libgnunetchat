# This Makefile.am is in the public domain
AM_CFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = -L$(top_srcdir)

check_PROGRAMS = \
  test_gnunet_chat_handle.test \
  test_gnunet_chat_lobby.test \
  test_gnunet_chat_file.test

AM_TESTS_ENVIRONMENT = \
  export LD_LIBRARY_PATH=$(top_srcdir);
TESTS = \
  $(check_PROGRAMS)

test_gnunet_chat_handle_test_SOURCES = \
  test_gnunet_chat_handle.c
test_gnunet_chat_handle_test_LDADD = \
  -lcheck \
  $(top_builddir)/src/libgnunetchat.la

test_gnunet_chat_lobby_test_SOURCES = \
  test_gnunet_chat_lobby.c
test_gnunet_chat_lobby_test_LDADD = \
  -lcheck \
  $(top_builddir)/src/libgnunetchat.la

test_gnunet_chat_file_test_SOURCES = \
  test_gnunet_chat_file.c
test_gnunet_chat_file_test_LDADD = \
  -lcheck \
  $(top_builddir)/src/libgnunetchat.la
