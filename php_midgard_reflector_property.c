/* Copyright (C) 2006, 2007, 2011 Piotr Pokora <piotrek.pokora@gmail.com>
 *
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

zend_class_entry *php_midgard_reflector_property_class;

#define _GET_MRP_OBJECT \
	zval *zval_object = getThis(); \
	MidgardReflectorProperty *mrp = MIDGARD_REFLECTOR_PROPERTY(__php_gobject_ptr(zval_object)); \
	if (!mrp) \
		php_error(E_ERROR, "Can not find underlying reflector instance");

#define _NOSCHEMA_CLASS_ERR \
	{ php_error(E_WARNING,"%s method can not be called."\
			"midgard_reflector_property initialized with non schema class", \
			get_active_function_name(TSRMLS_C));\
	RETURN_FALSE; }

/* Object constructor */
static PHP_METHOD(midgard_reflector_property, __construct)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *php_classname = NULL;
	int php_classname_length;
	zend_class_entry *ce_base;
	zval *zval_object = getThis();
	GObject *gobject;

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &php_classname, &php_classname_length) == FAILURE)
			return;

		{
			gboolean __isderived = php_midgard_is_derived_from_class(php_classname, MIDGARD_TYPE_DBOBJECT, TRUE, &ce_base TSRMLS_CC);

			if (!__isderived) {
				php_error(E_WARNING, "Expected %s derived class", g_type_name(MIDGARD_TYPE_DBOBJECT));
				php_midgard_error_exception_force_throw(mgd, MGD_ERR_INVALID_OBJECT TSRMLS_CC);
				return;
			}
		}

		const gchar *g_classname = php_class_name_to_g_class_name(ce_base->name);
		MidgardObjectClass *klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME(g_classname);

		if (!klass) {
			php_error(E_WARNING, "%s is not registered Midgard schema class", g_classname);
			return;
		}

		MidgardReflectorProperty *mrp = midgard_reflector_property_new(g_classname);

		if (!mrp) {
			php_midgard_error_exception_throw(mgd TSRMLS_CC);
			return;
		}

		MGD_PHP_SET_GOBJECT(zval_object, mrp);
	} else {
		// we already have gobject injected
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, get_midgard_type)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_LONG(midgard_reflector_property_get_midgard_type(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_midgard_type, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, is_link)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflector_property_is_link(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_link, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, is_linked)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflector_property_is_linked(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_linked, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, get_link_name)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	const gchar *linkname = midgard_reflector_property_get_link_name(mrp, property_name);

	if (linkname)
		RETURN_STRING((gchar *)linkname, 1);

	RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_link_name, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, get_link_target)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *property_name;
	int property_name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	const gchar *linktarget =
		midgard_reflector_property_get_link_target(mrp, property_name);

	if (linktarget)
		RETURN_STRING((gchar *)linktarget, 1);

	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_link_target, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, description)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	const gchar *description =
		midgard_reflector_property_description(mrp, property_name);

	if (description)
		RETURN_STRING((char *)description, 1);

	RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_description, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, get_user_value)
{
	char *property_name, *name;
	int property_name_length, name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
				&property_name, &property_name_length, &name, &name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	const gchar *value = midgard_reflector_property_get_user_value(mrp, property_name, name);

	if (value)
		RETURN_STRING((gchar *)value, 1);

	RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_user_value, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, is_private)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflector_property_is_private(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_private, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, is_unique)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflector_property_is_unique(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_unique, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, is_primary)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflector_property_is_primary(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_is_primary, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, has_default_value)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	RETURN_BOOL(midgard_reflector_property_has_default_value(mrp, property_name));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_has_default_value, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_property, get_default_value)
{
	char *property_name;
	int property_name_length;
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &property_name, &property_name_length) == FAILURE)
		return;

	_GET_MRP_OBJECT;

	GValue val = {0, };
	gboolean has_value = midgard_reflector_property_get_default_value(mrp, property_name, &val);

	if (!has_value)
		return;

	php_midgard_gvalue2zval(&val, return_value TSRMLS_CC);
	g_value_unset (&val);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_mrp_get_default_value, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_reflector_property)
{
	static function_entry reflector_property_methods[] = {
		PHP_ME(midgard_reflector_property, __construct,      arginfo_mrp___construct,      ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_reflector_property, get_midgard_type, arginfo_mrp_get_midgard_type, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, is_link,          arginfo_mrp_is_link,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, is_linked,        arginfo_mrp_is_linked,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, get_link_name,    arginfo_mrp_get_link_name,    ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, get_link_target,  arginfo_mrp_get_link_target,  ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, description,      arginfo_mrp_description,      ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, get_user_value,   arginfo_mrp_get_user_value,   ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, is_private,       arginfo_mrp_is_private,       ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, is_unique,        arginfo_mrp_is_unique,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, is_primary,       arginfo_mrp_is_primary,       ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, has_default_value,arginfo_mrp_has_default_value,ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_property, get_default_value,arginfo_mrp_get_default_value,ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry reflector_property_class_entry;
	INIT_CLASS_ENTRY(reflector_property_class_entry, "MidgardReflectorProperty", reflector_property_methods);
	php_midgard_reflector_property_class = zend_register_internal_class(&reflector_property_class_entry TSRMLS_CC);
	php_midgard_reflector_property_class->create_object = php_midgard_gobject_new;

	zend_register_class_alias("midgard_reflector_property", php_midgard_reflector_property_class);

	return SUCCESS;
}
