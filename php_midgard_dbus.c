/* Copyright (C) 2008 Piotr Pokora <piotrek.pokora@gmail.com>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "php_midgard.h"
#include "php_midgard_gobject.h"

#include <zend_exceptions.h>

static zend_class_entry *php_midgard_dbus_class;

/* Object constructor */
static PHP_METHOD(midgard_dbus, __construct)
{
	char *path;
	int path_length;
	zend_bool use_session = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &path, &path_length, &use_session) == FAILURE)
		return;

	MidgardConnection *connection = mgd_handle();
	if (NULL == connection) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Failed to get connection");
		return;
	}

	MidgardDbus *mbus = midgard_dbus_new(connection, path, (gboolean)use_session);
	if (!mbus) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Failed to init dbus subsystem");
		return;
	}

	MGD_PHP_SET_GOBJECT(getThis(), mbus);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_dbus___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, use_session)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_dbus, send)
{
	RETVAL_FALSE;
	CHECK_MGD;
	char *path, *msg;
	int path_length, msg_length;
	zend_bool use_session = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &path, &path_length, &msg, &msg_length, &use_session) == FAILURE)
		return;

	midgard_dbus_send(mgd_handle(), path, msg, (gboolean)use_session);

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_dbus_send, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, use_session)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_dbus)
{

	static function_entry midgard_dbus_methods[] = {
		PHP_ME(midgard_dbus, __construct, arginfo_midgard_dbus___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_dbus, send,        arginfo_midgard_dbus_send,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_dbus_class_entry;

	INIT_CLASS_ENTRY(php_midgard_dbus_class_entry, "midgard_dbus", midgard_dbus_methods);

	php_midgard_dbus_class = zend_register_internal_class(&php_midgard_dbus_class_entry TSRMLS_CC);
	php_midgard_dbus_class->create_object = php_midgard_gobject_new;
	php_midgard_dbus_class->doc_comment = strdup("Sender of DBUS messages");

	return SUCCESS;
}
