dnl  Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
dnl  Copyright (C) 2010 Alexey Zakhlestin <indeyets@gmail.com>
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

PHP_ARG_WITH(midgard2, for midgard2 support,
[  --with-midgard2         Enable midgard2 support], yes)

if test "$PHP_MIDGARD2" != "no"; then
    if test -z "$PKG_CONFIG"; then
        AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
    fi

    if test -f "$PKG_CONFIG"; then
        if $PKG_CONFIG --atleast-version=10.05.1 midgard2; then
            MIDGARD2_INCLINE=`$PKG_CONFIG --cflags midgard2`
            MIDGARD2_LIBLINE=`$PKG_CONFIG --libs midgard2`
        else
            AC_MSG_ERROR([Midgard2 version 10.05.1 or greater required.])
        fi

        PHP_EVAL_INCLINE($MIDGARD2_INCLINE)
        PHP_EVAL_LIBLINE($MIDGARD2_LIBLINE, MIDGARD2_SHARED_LIBADD)

        CFLAGS="$CFLAGS -Wall -fno-strict-aliasing"

        if test "$php_always_shared" = "yes"; then
            AC_DEFINE(MGD_INCLUDE_PHP_CONFIG, 1, [ ])
        fi

        PHP_SUBST(MIDGARD2_SHARED_LIBADD)
        AC_DEFINE(HAVE_MIDGARD, 1, [ ])

        PHP_NEW_EXTENSION(midgard2, \
            midgard.c query_builder.c \
            php_midgard_reflection_property.c php_midgard_collector.c php_midgard_object_parameter.c \
	    php_midgard_object_attachment.c php_midgard_config.c php_midgard_gobject_closures.c \
	    php_midgard_gobject_generic.c php_midgard_user.c php_midgard_blob.c php_midgard_object_class.c \
            php_midgard_functions.c php_midgard_connection.c php_midgard_object.c php_midgard_replicator.c \
	    php_midgard_dbus.c php_midgard_timestamp.c php_midgard_timestamp_api.c php_midgard_error.c \
	    php_midgard_handle.c php_midgard_transaction.c php_midgard__helpers.c php_midgard_view.c \
	    php_midgard_storage.c php_reflection_workaround.c php_midgard_key_config_context.c \
	    php_midgard_key_config_file_context.c php_midgard_key_config.c php_midgard_key_config_file.c \
            php_midgard_urlwrapper.c php_midgard_query.c php_midgard_query_storage.c php_midgard_query_holders.c \
	    php_midgard_query_constraints.c php_midgard_reflector_object.c php_midgard_reflector_property.c \ 
            php_midgard_query_executors.c php_midgard_g_mainloop.c php_midgard_workspace_storage.c php_midgard_base_interface.c \
            , $ext_shared)
        PHP_ADD_EXTENSION_DEP(midgard2, spl)
    else
        AC_MSG_ERROR(Ooops ! no pkg-config found .... )
    fi
fi
