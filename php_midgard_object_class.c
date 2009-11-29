/* Copyright (C) 2006 Piotr Pokora <piotrek.pokora@gmail.com>
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

static zend_class_entry *php_midgard_object_class_class;

static ZEND_METHOD(midgard_object_class, factory)
{
	RETVAL_FALSE;
	CHECK_MGD;

	char *class_name;
	int class_name_length;
	zval *zvalue = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &class_name, &class_name_length, &zvalue) == FAILURE)
		return;

	zend_class_entry *ce = zend_fetch_class(class_name, class_name_length, ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	if (ce == NULL) {
		php_error(E_WARNING, "Can not find %s class", class_name);
		return;
	}

	object_init_ex(return_value, ce); /* Initialize new object for which QB has been created for */

	if (zvalue == NULL) {
		zend_call_method_with_0_params(&return_value, ce, &ce->constructor, "__construct", NULL); /* Call class constructor on given instance */
	} else {
		zend_call_method_with_1_params(&return_value, ce, &ce->constructor, "__construct", NULL, zvalue); /* Call class constructor on given instance */
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_factory, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, get_object_by_guid)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *guid;
	guint guid_length;
	const gchar *type_name;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length) == FAILURE)
		return;

	MgdObject *object = midgard_object_class_get_object_by_guid(mgd_handle(), guid);

	if (!object) {
		php_midgard_error_exception_throw(mgd_handle());
		return;
	}

	type_name = G_OBJECT_TYPE_NAME(G_OBJECT(object));
	zend_class_entry *ce = zend_fetch_class((gchar *)type_name, strlen(type_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	if (ce == NULL) {
		php_error(E_WARNING, "Can not find %s class", type_name);
		return;
	}

	object_init_ex(return_value, ce); /* Initialize new object for which QB has been created for */
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(object)); // inject our gobject
	zend_call_method_with_0_params(&return_value, ce, &ce->constructor, "__construct", NULL); /* Call class constructor on given instance */
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_get_object_by_guid, 0, 0, 1)
	ZEND_ARG_INFO(0, guid)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, get_property_up)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zvalue;
	MidgardObjectClass *klass = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(zvalue) != IS_STRING) {
		if (Z_TYPE_P(zvalue) != IS_OBJECT) {
			php_error(E_WARNING,
					"%s() accepts object or string as first argument",
					get_active_function_name(TSRMLS_C));
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_STRVAL_P(zvalue));
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_OBJCE_P(zvalue)->name);
	}

	if (!klass) {
		php_error(E_WARNING, "MidgardObjectClass not found");
		RETURN_FALSE;
	}

	const gchar *property_up = midgard_object_class_get_property_up(klass);

	if (!property_up)
		RETURN_NULL();

	RETURN_STRING((gchar *)property_up, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_get_property_up, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, get_property_parent)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zvalue;
	MidgardObjectClass *klass = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(zvalue) != IS_STRING) {
		if (Z_TYPE_P(zvalue) != IS_OBJECT) {
			php_error(E_WARNING,
					"%s() accepts object or string as first argument",
					get_active_function_name(TSRMLS_C));
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_STRVAL_P(zvalue));
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_OBJCE_P(zvalue)->name);
	}

	if (!klass) {
		php_error(E_WARNING, "MidgardObjectClass not found");
		RETURN_FALSE;
	}

	const gchar *property_parent = midgard_object_class_get_property_parent(klass);

	if (!property_parent)
		RETURN_NULL();

	RETURN_STRING((gchar *)property_parent, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_get_property_parent, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, undelete)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *guid;
	guint guid_length;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length)  == FAILURE) {
		return;
	}

	rv = midgard_object_class_undelete(mgd_handle(),
			(const gchar *)guid);
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_undelete, 0, 0, 1)
	ZEND_ARG_INFO(0, guid)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, connect_default)
{
	CHECK_MGD;
	php_midgard_object_class_connect_default(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_connect_default, 0, 0, 3)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(0, signal)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, userdata)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, has_metadata)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zvalue;
	MidgardObjectClass *klass = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(zvalue) != IS_STRING) {
		if (Z_TYPE_P(zvalue) != IS_OBJECT) {
			php_error(E_WARNING,
					"%s() accepts object or string as first argument",
					get_active_function_name(TSRMLS_C));
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_STRVAL_P(zvalue));
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_OBJCE_P(zvalue)->name);
	}

	if (!klass) {
		php_error(E_WARNING, "MidgardObjectClass not found");
		RETURN_FALSE;
	}

	gboolean rv = midgard_object_class_has_metadata (klass);

	RETURN_BOOL (rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_has_metadata, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static ZEND_METHOD(midgard_object_class, get_schema_value)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zvalue;
	gchar *name;
	guint name_length;
	MidgardObjectClass *klass = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs", &zvalue, &name, &name_length) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(zvalue) != IS_STRING) {
		if (Z_TYPE_P(zvalue) != IS_OBJECT) {
			php_error(E_WARNING,
					"%s() accepts object or string as first argument",
					get_active_function_name(TSRMLS_C));
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_STRVAL_P(zvalue));
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)Z_OBJCE_P(zvalue)->name);
	}

	if (!klass) {

		php_error(E_WARNING, "MidgardObjectClass not found");
		RETURN_FALSE;
	}

	const gchar *schema_value = midgard_object_class_get_schema_value (klass, (const gchar *)name);

	if (!schema_value)
		RETURN_NULL();

	RETURN_STRING((gchar *)schema_value, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_class_get_schema_value, 0, 0, 2)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(0, node_name)
ZEND_END_ARG_INFO()


void php_midgard_object_class_init(int module_number)
{
	static function_entry object_class_methods[] = {
		ZEND_ME(midgard_object_class,	factory,
				arginfo_midgard_object_class_factory,		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,	undelete,
				arginfo_midgard_object_class_undelete,		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,	get_object_by_guid,
				arginfo_midgard_object_class_get_object_by_guid,ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,	get_property_up,
				arginfo_midgard_object_class_get_property_up,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,	get_property_parent,
				arginfo_midgard_object_class_get_property_parent,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,   connect_default,
				arginfo_midgard_object_class_connect_default,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,   has_metadata,
				arginfo_midgard_object_class_has_metadata,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		ZEND_ME(midgard_object_class,   get_schema_value,
				arginfo_midgard_object_class_get_schema_value,	ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_object_class_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			php_midgard_object_class_class_entry,
			"midgard_object_class", object_class_methods);

	php_midgard_object_class_class =
		zend_register_internal_class(&php_midgard_object_class_class_entry TSRMLS_CC);
}
