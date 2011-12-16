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

#include "php_midgard__helpers.h"

static zend_class_entry *php_midgard_error_class;

/* Object constructor */
static PHP_METHOD(midgard_error, __construct)
{
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_error___construct, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_error, error)
{
	char *msg;
	zend_bool tog = FALSE;
	int msg_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &msg, &msg_length, &tog) == FAILURE)
		return;

	g_error("%s", msg);

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_error_error, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_error, critical)
{
	char *msg;
	zend_bool tog = FALSE;
	int msg_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &msg, &msg_length, &tog) == FAILURE)
		return;

	php_midgard_log_enabled = tog;
	g_critical("%s", msg);
	php_midgard_log_enabled = TRUE;

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_error_critical, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_error, warning)
{
	char *msg;
	zend_bool tog = FALSE;
	int msg_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &msg, &msg_length, &tog) == FAILURE)
		return;

	php_midgard_log_enabled = tog;
	g_warning("%s", msg);
	php_midgard_log_enabled = TRUE;

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_error_warning, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_error, message)
{
	char *msg;
	zend_bool tog = FALSE;
	int msg_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &msg, &msg_length, &tog) == FAILURE)
		return;

	php_midgard_log_enabled = tog;
	g_message("%s", msg);
	php_midgard_log_enabled = TRUE;

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_error_message, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_error, info)
{
	char *msg;
	zend_bool tog = FALSE;
	int msg_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &msg, &msg_length, &tog) == FAILURE)
		return;

	php_midgard_log_enabled = tog;
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "%s", msg);
	php_midgard_log_enabled = TRUE;

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_error_info, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_error, debug)
{
	char *msg;
	zend_bool tog = FALSE;
	int msg_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &msg, &msg_length, &tog) == FAILURE)
		return;

	php_midgard_log_enabled = tog;
	g_debug("%s", msg);
	php_midgard_log_enabled = TRUE;

	return;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_error_debug, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_error)
{
	static zend_function_entry midgard_error_methods[] = {
		PHP_ME(midgard_error, __construct, arginfo_midgard_error___construct, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
		PHP_ME(midgard_error, error,       arginfo_midgard_error_error,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_error, critical,    arginfo_midgard_error_critical,    ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_error, warning,     arginfo_midgard_error_warning,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_error, message,     arginfo_midgard_error_message,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_error, info,        arginfo_midgard_error_info,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_error, debug,       arginfo_midgard_error_debug,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_error_class_entry;
	INIT_CLASS_ENTRY(php_midgard_error_class_entry, "midgard_error", midgard_error_methods);

	php_midgard_error_class = zend_register_internal_class(&php_midgard_error_class_entry TSRMLS_CC);
	php_midgard_error_class->doc_comment = strdup("Collection of static methods, which can be used to send core-level log-messages");

	return SUCCESS;
}
