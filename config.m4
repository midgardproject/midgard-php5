dnl $Id$

dnl  Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
dnl 
dnl  This program is free software; you can redistribute it and/or modify it
dnl  under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl 
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

PATH=${PATH}:/usr/local/bin:/opt/bin

dnl Check if pkg-config exists
PHP_ARG_WITH(midgard, for pkg-config ,
	[ --with-pkg-config[=FILE]	pkg-config binary. FILE is the optional pathname
                          to pkg-config])

if test "$PHP_PKGCONFIG" != "no"; then
	if test "$PHP_PKGCONFIG" != "yes"; then
		MIDGARD_CONFIG=`$php_shtool path pkg-config`
	else
		MIDGARD_CONFIG=$PHP_PKGCONFIG   
	fi

	if test ! -x $MIDGARD_CONFIG || test ! -f $MIDGARD_CONFIG; then
		AC_MSG_ERROR([pkg-config ($MIDGARD_CONFIG) not executable. 
			Please specify the full path, including the scriptname])
	fi
fi

AC_MSG_RESULT(PKGC $MIDGARD_CONFIG)

dnl pkg-config --exists module returns nothing :(
dnl MIDGARD_EXISTS=1
dnl dnl Check if midgard module exists
dnl MIDGARD_EXISTS=`"$PKG_CONFIG" --exists midgard`
dnl if test $MIDGARD_EXISTS -ne 0 ; then
dnl 	AC_MSG_ERROR(midgard library not found, please install midgard-core)
dnl fi

dnl FIXME, it doesn't work on Mac OSX
dnl AC_CHECK_LIB(midgard2, midgard_connection_new, [], [
dnl 	AC_MSG_ERROR(midgard library not found please install midgard-core)
dnl 	])

MIDGARD2_VERSION=`"$MIDGARD_CONFIG" --modversion midgard2`
AC_MSG_RESULT(Midgard library found. Version $MIDGARD2_VERSION)

dnl LFLAGS="$LFLAGS -Pmgd -olex.yy.c"


if test "$php_always_shared" = "yes"; then
	AC_DEFINE(MGD_INCLUDE_PHP_CONFIG, 1, [ ])
fi

AC_MSG_CHECKING(whether to compile for php with system regex)
AC_ARG_WITH(php-regex, [  --with-php-regex	configure for regex used in PHP ], 
	PHP_MIDGARD_REGEX=[$]withval, PHP_MIDGARD_REGEX="php")
AC_MSG_RESULT($PHP_MIDGARD_REGEX)
if test "$PHP_MIDGARD_REGEX" != "php"; then
	AC_DEFINE(MIDGARD_PHP_REGEX, 1, [ ])
fi

MIDGARD2_INCLINE=`$MIDGARD_CONFIG --cflags midgard2`
MIDGARD2_LIBLINE=`$MIDGARD_CONFIG --libs midgard2`

PHP_EVAL_INCLINE($MIDGARD2_INCLINE)
PHP_EVAL_LIBLINE($MIDGARD2_LIBLINE, MIDGARD2_SHARED_LIBADD)

CFLAGS="$CFLAGS -Wall -fno-strict-aliasing"

AC_DEFINE(HAVE_MIDGARD, 1, [ ])
PHP_SUBST(MIDGARD2_SHARED_LIBADD)
PHP_NEW_EXTENSION(midgard2, midgard.c preparser.c query_builder.c php_midgard_reflection_property.c php_midgard_collector.c php_midgard_object_parameter.c php_midgard_object_attachment.c php_midgard_config.c php_midgard_gobject_generic.c php_midgard_user.c php_midgard_blob.c php_midgard_object_class.c php_midgard_compat.c php_midgard_connection.c php_midgard_object.c php_midgard_replicator.c php_midgard_dbus.c php_midgard_timestamp.c php_midgard_error.c php_midgard_handle.c php_midgard_transaction.c php_midgard__helpers.c php_midgard_view.c php_midgard_storage.c php_reflection_workaround.c php_midgard_key_config_context.c php_midgard_key_config_file_context.c php_midgard_key_config.c php_midgard_key_config_file.c, $ext_shared)
