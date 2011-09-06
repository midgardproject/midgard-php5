/* Copyright (C) 2011 Piotr Pokora <piotrek.pokora@gmail.com>
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

zend_class_entry *php_midgard_reflector_object_class;

#define __CHECK_GTYPE_NAME(__name) \
	GType type_from_name = g_type_from_name(__name); \
	if (type_from_name == G_TYPE_INVALID) { \
		php_error(E_WARNING, "Type '%s' is not registered in Gtype system", __name); \
		return; \
	} 

static PHP_METHOD(midgard_reflector_object, get_property_primary)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	const gchar *property_primary = midgard_reflector_object_get_property_primary(g_classname);

	if (!property_primary)
		RETURN_NULL();

	RETURN_STRING((char *)property_primary, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_get_property_primary, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, get_property_up)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	const gchar *property_up = midgard_reflector_object_get_property_up(g_classname);

	if (!property_up)
		RETURN_NULL();

	RETURN_STRING((char *)property_up, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_get_property_up, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, get_property_parent)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	const gchar *property_parent = midgard_reflector_object_get_property_parent(g_classname);

	if (!property_parent)
		RETURN_NULL();

	RETURN_STRING((char *)property_parent, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_get_property_parent, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, get_property_unique)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	const gchar *property_unique = midgard_reflector_object_get_property_unique(g_classname);

	if (!property_unique)
		RETURN_NULL();

	RETURN_STRING((char *)property_unique, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_get_property_unique, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, list_children)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	guint n_children;
	gchar **children = midgard_reflector_object_list_children(g_classname, &n_children);

	array_init(return_value);
	
	if (children == NULL)
		return;

	guint i;
	for (i = 0; i < n_children; i++) {
		add_assoc_string(return_value, (gchar *)children[i], "", 1);
	}

	g_free(children);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_list_children, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, has_metadata_class)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	RETURN_BOOL(midgard_reflector_object_has_metadata_class(g_classname));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_has_metadata_class, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, get_metadata_class)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	const gchar *metadata_class = midgard_reflector_object_get_metadata_class(g_classname);

	if (!metadata_class)
		RETURN_NULL();

	RETURN_STRING((char *)metadata_class, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_get_metadata_class, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()


static PHP_METHOD(midgard_reflector_object, get_schema_value)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;
	char *name;
	int name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs", &zvalue, &name, &name_length) == FAILURE) {
		return;
	}

	const gchar *classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		classname = (const gchar *) Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		classname = (const gchar *) Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "first argument should be object or string");
		return;
	}	

	const gchar *schema_value = midgard_reflector_object_get_schema_value(classname, (const gchar *)name);

	if (!schema_value)
		RETURN_NULL();

	RETURN_STRING((char *)schema_value, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_get_schema_value, 0, 0, 2)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(0, node_name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, is_mixin)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	RETURN_BOOL(midgard_reflector_object_is_mixin(g_classname));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_is_mixin, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, is_interface)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	RETURN_BOOL(midgard_reflector_object_is_interface(g_classname));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_is_interface, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, is_abstract)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	RETURN_BOOL(midgard_reflector_object_is_abstract(g_classname));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_is_abstract, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_reflector_object, list_defined_properties)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zvalue) == FAILURE) {
		return;
	}

	const char *php_classname = NULL;

	if (Z_TYPE_P(zvalue) == IS_STRING) {
		php_classname = Z_STRVAL_P(zvalue);
	} else if (Z_TYPE_P(zvalue) == IS_OBJECT) {
		php_classname = Z_OBJCE_P(zvalue)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "argument should be object or string");
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
	__CHECK_GTYPE_NAME(g_classname);

	guint n_prop;
	gchar **properties = midgard_reflector_object_list_defined_properties(g_classname, &n_prop);

	array_init(return_value);
	
	if (properties == NULL)
		return;

	guint i;
	for (i = 0; i < n_prop; i++) {
		add_assoc_string(return_value, (gchar *)properties[i], "", 1);
	}

	g_free(properties);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_reflector_object_list_defined_properties, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_reflector_object)
{
	static function_entry object_class_methods[] = {
		PHP_ME(midgard_reflector_object, get_property_primary,     arginfo_midgard_reflector_object_get_property_primary,     ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, get_property_up,     arginfo_midgard_reflector_object_get_property_up,     ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, get_property_parent, arginfo_midgard_reflector_object_get_property_parent, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, get_property_unique,     arginfo_midgard_reflector_object_get_property_unique,     ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, list_children,     arginfo_midgard_reflector_object_list_children,     ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, has_metadata_class,        arginfo_midgard_reflector_object_has_metadata_class,        ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, get_metadata_class,        arginfo_midgard_reflector_object_get_metadata_class,        ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, get_schema_value,    arginfo_midgard_reflector_object_get_schema_value,    ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, is_mixin,    arginfo_midgard_reflector_object_is_mixin,    ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, is_interface,    arginfo_midgard_reflector_object_is_interface,    ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, is_abstract,    arginfo_midgard_reflector_object_is_abstract,    ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
		PHP_ME(midgard_reflector_object, list_defined_properties,    arginfo_midgard_reflector_object_list_defined_properties,    ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_reflector_object_class_entry;
	INIT_CLASS_ENTRY(php_midgard_reflector_object_class_entry, "MidgardReflectorObject", object_class_methods);

	php_midgard_reflector_object_class = zend_register_internal_class(&php_midgard_reflector_object_class_entry TSRMLS_CC);
	php_midgard_reflector_object_class->doc_comment = strdup("Collection of static methods which provide reflection for MgdSchema classes");

	return SUCCESS;
}
