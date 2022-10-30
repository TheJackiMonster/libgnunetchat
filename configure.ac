# This file is part of GNUnet.
# Copyright (C) 2022 GNUnet e.V.
#
# GNUnet is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# GNUnet is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: AGPL3.0-or-later
#
# Process this file with autoconf to produce a configure script.
#
#
AC_INIT([libgnunetchat],[m4_esyscmd_s(sh contrib/get_version.sh)],[bug-gnunet@gnu.org])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])

AC_CONFIG_HEADERS([
  include/gnunet/gnunet_chat_config.h
])

AC_CANONICAL_TARGET
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([tar-ustar])
AM_SILENT_RULES

# maybe use AC_USE_SYSTEM_EXTENSIONS?
AH_TOP([#define _GNU_SOURCE 1])

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
PKG_PROG_PKG_CONFIG([0.29.2])

LT_INIT([disable-static dlopen])

AS_IF([test "x$enable_shared" = "xno"],
  [AC_MSG_ERROR([GNUnet works only with shared libraries, sorry])])

# test for GNUnet core
gnunet=0
gnunet_conversation=0
lookin=${prefix}
GNUNET_CFLAGS=""
GNUNET_CPPFLAGS=""
GNUNET_LIBS=""
AC_MSG_CHECKING(for GNUnet core)
AC_ARG_WITH(gnunet,
   [  --with-gnunet=PFX       Base of GNUnet installation],
   [AC_MSG_RESULT([$with_gnunet])
    AS_CASE([$with_gnunet],
      [no],[lookin=""],
      [yes],[lookin="${prefix}"],
      [lookin="$with_gnunet"])
   ],
   [
     AC_MSG_RESULT([--with-gnunet not specified])
     PKG_CHECK_MODULES([GNUNET], [gnunetutil >= 0.18.0], gnunet=1)
     AC_CHECK_HEADERS([gnunet/gnunet_messenger_service.h],
     AC_CHECK_LIB([gnunetmessenger], [GNUNET_MESSENGER_connect],
      [
        EXT_LIB_PATH="-L${lookin}/lib $EXT_LIB_PATH"
        if test -d "${lookin}/lib64"; then
          EXT_LIB_PATH="-L${lookin}/lib64 $EXT_LIB_PATH"
        fi
      ]
      ),,[#include <gnunet/platform.h>]
     )
   ]
)

AS_IF([test "x$gnunet" = "x0" -a ! "x$lookin" = "x"],
[
  AC_MSG_CHECKING(for GNUnet util library in $lookin)
  backup_LIBS="$LIBS"
  backup_CFLAGS="$CFLAGS"
  backup_CPPFLAGS="$CPPFLAGS"
  GNUNET_LIBS="-L${lookin}/lib"
  if test -d "${lookin}/lib64"; then
    GNUNET_LIBS="-L${lookin}/lib64 $GNUNET_LIBS"
  fi
  GNUNET_CFLAGS="-I${lookin}/include"
  GNUNET_CPPFLAGS="-I${lookin}/include"
  LIBS="$GNUNET_LIBS $backup_LIBS"
  CFLAGS="$GNUNET_CFLAGS $backup_CFLAGS"
  CPPFLAGS="$GNUNET_CPPFLAGS $backup_CPPFLAGS"
  AC_CHECK_HEADERS([gnunet/gnunet_util_lib.h],
    AC_CHECK_LIB([gnunetutil], [GNUNET_xfree_],
      [
        gnunet=1
        EXT_LIB_PATH="-L${lookin}/lib $EXT_LIB_PATH"
        if test -d "${lookin}/lib64"; then
          EXT_LIB_PATH="-L${lookin}/lib64 $EXT_LIB_PATH"
        fi

      ]
    ),,[#include <gnunet/platform.h>]
  )
  AC_CHECK_HEADERS([gnunet/gnunet_messenger_service.h],
    AC_CHECK_LIB([gnunetmessenger], [GNUNET_MESSENGER_connect],
      [
        gnunet_messenger=1
      ]
    ),,[#include <gnunet/platform.h>]
  )
  LIBS="$backup_LIBS"
  CFLAGS="$backup_CFLAGS"
  CPPFLAGS="$backup_CPPFLAGS"
])

AS_IF([test "x$gnunet" = "x0" || test "x$gnunet_messenger" = "x0" ],
 [AC_MSG_ERROR([libgnunetchat requires GNUnet])])



AC_SUBST(GNUNET_CFLAGS)
AC_SUBST(GNUNET_LIBS)
CFLAGS="$GNUNET_CFLAGS $CFLAGS"
LIBS="$GNUNET_LIBS $LIBS"

AC_ARG_ENABLE([debug],
[  --enable-debug          turn on debugging],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  tests/Makefile
])

AC_OUTPUT
