/* Copyright (C) 2010 Alexey Zakhlestin <indeyets@gmail.com>
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
#include "php_midgard_query.h"
#include "php_midgard_gobject.h"

#include "php_midgard__helpers.h"

zend_class_entry *php_midgard_query_holder_class;
zend_class_entry *php_midgard_query_property_class;
zend_class_entry *php_midgard_query_value_class;

static PHP_METHOD(midgard_query_holder, get_value)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQueryHolder *holder = MIDGARD_QUERY_HOLDER(__php_gobject_ptr(getThis()));

	GValue val = {0, };
	midgard_query_holder_get_value(holder, &val);

	zend_bool result = php_midgard_gvalue2zval(&val, return_value TSRMLS_CC);

	if (!result) {
		RETURN_FALSE;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_holder_get_value, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_holder, set_value)
{
	zval *value = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &value) == FAILURE) {
		return;
	}

	MidgardQueryHolder *holder = MIDGARD_QUERY_HOLDER(__php_gobject_ptr(getThis()));
	GValue *tmp = php_midgard_zval2gvalue(value TSRMLS_CC);

	zend_bool result = (zend_bool) midgard_query_holder_set_value(holder, (const GValue *) tmp);
	g_value_unset(tmp);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_holder_set_value, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()


static PHP_METHOD(midgard_query_property, __construct)
{
	char *property_name;
	int property_name_length = 0;
	zval *z_storage = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|O", &property_name, &property_name_length, &z_storage,  php_midgard_query_storage_class) == FAILURE) {
		return;
	}

	MidgardQueryStorage *storage = NULL;

	if (z_storage)
		storage = MIDGARD_QUERY_STORAGE(__php_gobject_ptr(z_storage));

	MidgardQueryProperty *property = midgard_query_property_new(property_name, storage);

	if (!property) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Can not wrap this property in midgard_query_property");
		return;
	}

	MGD_PHP_SET_GOBJECT(getThis(), property);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_property___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, property_name)
	ZEND_ARG_OBJ_INFO(0, storage, midgard_query_storage, 1)
ZEND_END_ARG_INFO()


static PHP_METHOD(midgard_query_value, __construct)
{
	zval *zvalue = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &zvalue) == FAILURE) {
		return;
	}

	MidgardQueryValue *qvalue;

	if (zvalue) {
		GValue *tmp = php_midgard_zval2gvalue(zvalue TSRMLS_CC);
		qvalue = midgard_query_value_create_with_value((const GValue *)tmp);
		g_value_unset(tmp);
		g_free(tmp);
	} else {
		qvalue = midgard_query_value_new();
	}

	if (!qvalue) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Can not wrap this value in midgard_query_value");
		return;
	}

	MGD_PHP_SET_GOBJECT(getThis(), qvalue);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_value___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()


PHP_MINIT_FUNCTION(midgard2_query_holders)
{
	static function_entry midgard_query_holder_methods[] = {
		PHP_ME(midgard_query_holder, get_value, arginfo_midgard_query_holder_get_value, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_holder, set_value, arginfo_midgard_query_holder_set_value, ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_holder_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_holder_class_entry, "MidgardQueryHolder", midgard_query_holder_methods);

	php_midgard_query_holder_class = zend_register_internal_class(&php_midgard_query_holder_class_entry TSRMLS_CC);
	php_midgard_query_holder_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_query_holder_class->create_object = php_midgard_gobject_new;
	php_midgard_query_holder_class->doc_comment = strdup("Base class for holding data-atom in midgard_query");

	zend_register_class_alias("midgard_query_holder", php_midgard_query_holder_class);

	static function_entry midgard_query_property_methods[] = {
		PHP_ME(midgard_query_property, __construct, arginfo_midgard_query_property___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_property_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_property_class_entry, "MidgardQueryProperty", midgard_query_property_methods);

	php_midgard_query_property_class = zend_register_internal_class_ex(&php_midgard_query_property_class_entry, php_midgard_query_holder_class, "midgard_query_holder" TSRMLS_CC);
	php_midgard_query_property_class->create_object = php_midgard_gobject_new;
	php_midgard_query_property_class->doc_comment = strdup("Class for holding object-properties");

	zend_register_class_alias("midgard_query_property", php_midgard_query_property_class);

	static function_entry midgard_query_value_methods[] = {
		PHP_ME(midgard_query_value, __construct, arginfo_midgard_query_value___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_value_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_value_class_entry, "MidgardQueryValue", midgard_query_value_methods);

	php_midgard_query_value_class = zend_register_internal_class_ex(&php_midgard_query_value_class_entry, php_midgard_query_holder_class, "midgard_query_holder" TSRMLS_CC);
	php_midgard_query_value_class->create_object = php_midgard_gobject_new;
	php_midgard_query_value_class->doc_comment = strdup("Class for holding literal data");

	zend_register_class_alias("midgard_query_value", php_midgard_query_value_class);

	return SUCCESS;
}
