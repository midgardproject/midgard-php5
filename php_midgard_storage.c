/* Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
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

static zend_class_entry *php_midgard_storage_class;

static PHP_METHOD(midgard_storage, create_base_storage)
{
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	zend_bool rv = (zend_bool) midgard_storage_create_base_storage(mgd_handle());
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_storage_create_base_storage, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_storage, create_class_storage)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *classname;
	gint classname_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &classname_length) == FAILURE) {
		return;
	}

	zend_bool rv = (zend_bool) midgard_storage_create(mgd_handle(), classname);
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_storage_create_class_storage, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_storage, update_class_storage)
{
	RETVAL_FALSE;
	gchar *classname;
	gint classname_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &classname_length) == FAILURE) {
		return;
	}

	zend_bool rv = (zend_bool) midgard_storage_update(mgd_handle(), classname);
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_storage_update_class_storage, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_storage, delete_class_storage)
{
	RETVAL_FALSE;
	gchar *classname;
	gint classname_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &classname_length) == FAILURE) {
		return;
	}

	zend_bool rv = (zend_bool) midgard_storage_delete(mgd_handle(), classname);
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_storage_delete_class_storage, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_storage, class_storage_exists)
{
	RETVAL_FALSE;
	gchar *classname;
	gint classname_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &classname_length) == FAILURE) {
		return;
	}

	zend_bool rv = (zend_bool) midgard_storage_exists(mgd_handle(), classname);
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_storage_class_storage_exists, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_storage)
{
	static function_entry midgard_storage_methods[] = {
		PHP_ME(midgard_storage, create_base_storage,  arginfo_midgard_storage_create_base_storage,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_storage, create_class_storage, arginfo_midgard_storage_create_class_storage, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_storage, update_class_storage, arginfo_midgard_storage_update_class_storage, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_storage, delete_class_storage, arginfo_midgard_storage_delete_class_storage, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_storage, class_storage_exists, arginfo_midgard_storage_class_storage_exists, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_storage_class_entry;
	INIT_CLASS_ENTRY(php_midgard_storage_class_entry, "midgard_storage", midgard_storage_methods);

	php_midgard_storage_class = zend_register_internal_class(&php_midgard_storage_class_entry TSRMLS_CC);
	php_midgard_storage_class->create_object = NULL;
	php_midgard_storage_class->doc_comment = strdup("Collection of static methods for managing underlying data storage");

	return SUCCESS;
}
