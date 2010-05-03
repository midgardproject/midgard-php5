/* Copyright (C) 2006, 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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


zend_class_entry *php_midgard_collector_class;

#define _GET_COLLECTOR_OBJECT \
	zval *zval_object = getThis(); \
	MidgardCollector *collector = MIDGARD_COLLECTOR(__php_gobject_ptr(zval_object)); \
	if (!collector) \
		php_error(E_ERROR, "Can not find underlying collector instance");

/* Object constructor */
static PHP_METHOD(midgard_collector, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	GObject *gobject;

	char *classname, *propname;
	int classname_length, propname_length;
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz",
				&classname, &classname_length,
				&propname, &propname_length,
				&value) == FAILURE)
	{
		return;
	}

	zend_class_entry *ce = zend_fetch_class(classname, classname_length, ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	if (ce == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Didn't find %s class", classname);
		php_midgard_error_exception_force_throw(mgd_handle(TSRMLS_C), MGD_ERR_INVALID_OBJECT);
	}

	zend_class_entry *base_ce = php_midgard_get_baseclass_ptr(ce);
	GType classtype = g_type_from_name((const gchar *)base_ce->name);

	if (!g_type_is_a(classtype, MIDGARD_TYPE_DBOBJECT)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Expected %s derived class", g_type_name(MIDGARD_TYPE_DBOBJECT));
		php_midgard_error_exception_force_throw(mgd_handle(TSRMLS_C), MGD_ERR_INVALID_OBJECT);
		return;
	}

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		GValue *gvalue = php_midgard_zval2gvalue(value);

		MidgardConnection *mgd = mgd_handle(TSRMLS_C);
		MidgardCollector *object = midgard_collector_new(mgd, base_ce->name, propname, gvalue);

		if (!object)
			return;

		MGD_PHP_SET_GOBJECT(zval_object, object);
	} else {
		// we already have gobject injected
	}

	php_midgard_gobject *php_gobject = __php_objstore_object(zval_object);
	/* Set user defined class. We might need it when execute is invoked */
	php_gobject->user_ce = ce;
	php_gobject->user_class_name = ce->name;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector___construct, 0, 0, 3)
	ZEND_ARG_INFO(0, classname)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, set_key_property)
{
	RETVAL_FALSE;
	CHECK_MGD;
	char *propname;
	int propname_length;
	zval *zvalue;
	gboolean rv;
	GValue *gvalue = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &propname, &propname_length, &zvalue) == FAILURE) {
		return;
	}

	_GET_COLLECTOR_OBJECT;

	rv = midgard_collector_set_key_property(collector, propname, gvalue);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_set_key_property, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, add_value_property)
{
	RETVAL_FALSE;
	CHECK_MGD;
	char *propname;
	int propname_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &propname, &propname_length) == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_add_value_property(collector, propname));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_add_value_property, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, set)
{
	CHECK_MGD;
	RETVAL_TRUE;
	char *key, *subkey;
	int key_length, subkey_length;
	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz",
				&key, &key_length,
				&subkey, &subkey_length,
				&zvalue) == FAILURE)
	{
		return;
	}

	_GET_COLLECTOR_OBJECT;

	GValue *gvalue = php_midgard_zval2gvalue(zvalue);
	gboolean rv = midgard_collector_set(collector, key, subkey, gvalue);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_set, 0, 0, 3)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, subkey)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static void __colector_update_zend_hash(GQuark key_id, gpointer data, gpointer user_data)
{
	zval *zend_hash = (zval *) user_data;
	GValue *gvalue = (GValue *) data;

	if (gvalue == NULL)
		return;

	zval *zvalue;
	MAKE_STD_ZVAL(zvalue);
	/* FIXME, we need to get underlying object here */
	php_midgard_gvalue2zval(gvalue, zvalue);

	add_assoc_zval(zend_hash,
			(gchar *)g_quark_to_string(key_id),
			zvalue);
	return;
}

static PHP_METHOD(midgard_collector, get)
{
	RETVAL_FALSE;
	CHECK_MGD;
	char *key;
	int key_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_length) == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	array_init(return_value);

	GData *keyslist = midgard_collector_get(collector, (const gchar *)key);

	if (keyslist != NULL) {
		g_datalist_foreach(&keyslist, __colector_update_zend_hash, return_value);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_get, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, get_subkey)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const char *key, *subkey;
	int key_length, subkey_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &key_length, &subkey, &subkey_length) == FAILURE) {
		return;
	}

	_GET_COLLECTOR_OBJECT;

	GValue *gvalue = midgard_collector_get_subkey(collector, key, subkey);

	if (!gvalue)
		return;

	zval *_ret;
	MAKE_STD_ZVAL(_ret);
	php_midgard_gvalue2zval(gvalue, _ret);

	RETURN_ZVAL(_ret, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_get_subkey, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, subkey)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, remove_key)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const char *key;
	int key_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_length) == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_remove_key(collector, key));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_remove_key, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, merge)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zobject;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|b", &zobject, php_midgard_collector_class, &zbool) == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	MidgardCollector *mc = MIDGARD_COLLECTOR(__php_gobject_ptr(zobject));

	RETURN_BOOL(midgard_collector_merge(collector, mc, zbool));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_merge, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, key, midgard_collector, 0)
	ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, list_keys)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	array_init(return_value);

	gchar **keys =
		midgard_collector_list_keys(collector);

	if (!keys)
		return;

	size_t i = 0;
	while (keys[i] != NULL) {
		add_assoc_string(return_value, (gchar *)keys[i], "", 1);
		i++;
	}

	g_free(keys);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_collector_list_keys, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, add_constraint)
{
	RETVAL_FALSE;
	CHECK_MGD;
	char *name, *op;
	int name_length, op_length;
	zval *value;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz",
				&name, &name_length,
				&op, &op_length,
				&value) != SUCCESS)
	{
		return;
	}

	GValue *gvalue = php_midgard_zval2gvalue(value);

	_GET_COLLECTOR_OBJECT;

	rv = midgard_collector_add_constraint(collector, name, op, gvalue);

	g_value_unset(gvalue);
	g_free(gvalue);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_add_constraint, 0, 0, 3)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, operator)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, begin_group)
{
	CHECK_MGD;
	RETVAL_FALSE;
	char *type;
	int type_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &type, &type_length) != SUCCESS)
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_begin_group(collector, type));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_collector_begin_group, 0)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, end_group)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_end_group(collector));
}

static PHP_METHOD(midgard_collector, add_order)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const char *field, *order = "ASC";
	int field_length, order_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &field, &field_length, &order, &order_length) != SUCCESS)
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_add_order(collector, field, order));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_add_order, 0, 0, 1)
	ZEND_ARG_INFO(0, property)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, set_offset)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long offset;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) != SUCCESS)
		return;

	if (offset < 0) {

		php_error(E_WARNING, "Ignoring a negative query offset");
		return;

	}

	_GET_COLLECTOR_OBJECT;

	midgard_collector_set_offset(collector, offset);

	RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_set_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, set_limit)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long limit;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &limit) != SUCCESS)
		return;

	if (limit < 0) {

		php_error(E_WARNING, "Ignoring a negative query limit");
		return;
	}

	_GET_COLLECTOR_OBJECT;

	midgard_collector_set_limit(collector, limit);

	RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_collector_set_limit, 0, 0, 1)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_collector, execute)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_COLLECTOR_OBJECT;

	RETURN_BOOL(midgard_collector_execute(collector));
}

PHP_MINIT_FUNCTION(midgard2_collector)
{
	static function_entry collector_methods[] = {
		PHP_ME(midgard_collector, __construct,        arginfo_midgard_collector___construct,        ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_collector, set_key_property,   arginfo_midgard_collector_set_key_property,   ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, add_value_property, arginfo_midgard_collector_add_value_property, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, set,                arginfo_midgard_collector_set,                ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, get,                arginfo_midgard_collector_get,                ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, get_subkey,         arginfo_midgard_collector_get_subkey,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, merge,              arginfo_midgard_collector_merge,              ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, list_keys,          arginfo_midgard_collector_list_keys,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, remove_key,         arginfo_midgard_collector_remove_key,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, add_constraint,     arginfo_midgard_collector_add_constraint,     ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, begin_group,        arginfo_midgard_collector_begin_group,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, end_group,          NULL,                                         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, add_order,          arginfo_midgard_collector_add_order,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, set_offset,         arginfo_midgard_collector_set_offset,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, set_limit,          arginfo_midgard_collector_set_limit,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_collector, execute,            NULL,                                         ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_collector_class_entry;
	INIT_CLASS_ENTRY(php_midgard_collector_class_entry, "midgard_collector", collector_methods);

	/* FIXME, this inheritance should be automatic once we switch to namespaces */
	php_midgard_collector_class =  zend_register_internal_class_ex (&php_midgard_collector_class_entry, NULL, "midgard_query_builder" TSRMLS_CC);
	php_midgard_collector_class->create_object = php_midgard_gobject_new;
	php_midgard_collector_class->doc_comment = strdup("Optimized database query tool, that doesn't return objects");

	return SUCCESS;
}
