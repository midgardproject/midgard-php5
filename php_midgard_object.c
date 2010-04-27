/*
 * Copyright (C) 2007, 2008 Piotr Pokora <piotrek.pokora@gmail.com>
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
#include "php_midgard_object.h"

#include "php_midgard__helpers.h"

#include <Zend/zend_exceptions.h>

zend_class_entry *php_midgard_dbobject_class = NULL;
zend_class_entry *php_midgard_object_class = NULL;

#define __THROW_EXCEPTION \
	if (EG(exception)) { \
		return_value = zend_throw_exception(Z_OBJCE_P(EG(exception)), "", 0 TSRMLS_CC); \
		return; \
	}

#define _GET_MIDGARD_OBJECT \
	zval *zval_object = getThis(); \
	MidgardObject *object = MIDGARD_OBJECT(__php_gobject_ptr(zval_object)); \
	if (!object) \
		php_error(E_ERROR, "Can not find underlying midgard object instance");

static zend_bool init_php_midgard_object_from_id(zval *instance, const char *php_classname, zval *objid TSRMLS_DC)
{
	MidgardObject *gobject = NULL;
	MidgardConnection *mgd = mgd_handle();
	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);

	if (objid == NULL) {
		/* Initialize empty MidgardObject */
		gobject = midgard_object_new(mgd, (const gchar *) g_classname, NULL);
	} else {
		/* There is one parameter , so we get object by id or guid */

		if (Z_TYPE_P(objid) == IS_LONG) {
			// ok
		} else if (Z_TYPE_P(objid) == IS_STRING) {
			// if it is short and numeric, then it is id!
			if (Z_STRLEN_P(objid) < 10 && is_numeric_string(Z_STRVAL_P(objid), Z_STRLEN_P(objid), NULL, NULL, 0)) {
				convert_to_long(objid);
			}
		} else {
			php_error(E_WARNING, "Wrong id-type for '%s' constructor", php_classname);
			php_midgard_error_exception_throw(mgd);
			return FALSE;
		}

		GValue *value = php_midgard_zval2gvalue(objid);
		gobject = midgard_object_new(mgd, (const gchar *) g_classname, value);
		g_value_unset(value);
		g_free(value);
	}

	if (!gobject) {
		php_midgard_error_exception_throw(mgd);
		return FALSE;
	}

	MGD_PHP_SET_GOBJECT(instance, gobject);

	return TRUE;
}

/* Object constructor */
PHP_FUNCTION(_midgard_php_object_constructor)
{
	CHECK_MGD;
	RETVAL_FALSE;
	zval *zval_object = getThis();
	zend_class_entry *base_class = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(zval_object));
	const gchar *zend_classname = base_class->name;
	GObject *gobject;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] %s::__construct() \n", zval_object, zend_classname);
	}

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		// this is a normal constructing process (we don't have assigned gobject yet)
		zval *objid = NULL;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z!", &objid) == FAILURE)
			return;

		if (init_php_midgard_object_from_id(zval_object, zend_classname, objid TSRMLS_CC) == FALSE)
			return;

		RETVAL_TRUE;

		gobject = __php_gobject_ptr(zval_object);
	} else {
		// we already have gobject injected
	}

	php_midgard_init_properties_objects(zval_object);
	php_midgard_object_connect_class_closures(G_OBJECT(gobject), zval_object);
	g_signal_emit(gobject, MIDGARD_OBJECT_GET_CLASS(gobject)->signal_action_loaded_hook, 0);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= %s::__construct() \n", zval_object, zend_classname);
	}
}

PHP_FUNCTION(_midgard_php_object_create)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;

	if (object != NULL) {
		g_signal_emit(object, MIDGARD_OBJECT_GET_CLASS(object)->signal_action_create_hook, 0);
		__THROW_EXCEPTION

		if (midgard_object_create(object)) 
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_midgard_php_object_update)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;

	if (object) {

		g_signal_emit(object, MIDGARD_OBJECT_GET_CLASS(object)->signal_action_update_hook, 0);
		__THROW_EXCEPTION

		if (midgard_object_update(object)) 
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_get_type_by_id)
{
	RETVAL_FALSE;
	CHECK_MGD;
	guint id;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE)
		return;

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		midgard_object_get_by_id(mobj, id);
		if (!php_midgard_error_exception_throw(mgd_handle())) {
			g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
			RETVAL_TRUE;
		}
	}
}

PHP_FUNCTION(_midgard_php_object_get_by_guid)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	gchar *guid;
	int glen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &glen)  == FAILURE) {
		return;
	}

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		midgard_object_get_by_guid(mobj, guid);

		if (!php_midgard_error_exception_throw(mgd_handle())) {
			g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
			RETVAL_TRUE;
		}
	}
}

PHP_FUNCTION(_midgard_php_object_is_in_parent_tree)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long rootid, id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rootid, &id) == FAILURE)
		return;

	/* Return TRUE if id or rootid is 0. */
	/* FIXME: Comment says "or", but code says "and". Who's right? */
	if (rootid == 0 && id == 0)
		RETURN_TRUE;

	MidgardConnection *mgd = mgd_handle();
	zval *zval_object = getThis();
	zend_class_entry *base_class = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(zval_object));
	const char *php_classname = base_class->name;

	MidgardObject *leaf_obj = NULL, *root_obj = NULL;

	{
		const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
		GValue tmp_gval;
		g_value_set_int(&tmp_gval, id);

		leaf_obj = midgard_object_new(mgd, g_classname, &tmp_gval);

		if (!leaf_obj) {
			php_error(E_NOTICE, "Did not find object with id=%ld", id);
			return;
		}
	}

	{
		const gchar *g_parent_classname = midgard_schema_object_tree_get_parent_name(leaf_obj);
		GValue tmp_gval;
		g_value_set_int(&tmp_gval, rootid);

		root_obj = midgard_object_new(mgd, g_parent_classname, &tmp_gval);

		if (!root_obj) {
			php_error(E_NOTICE, "Did not find object with id=%ld", rootid);
			g_object_unref(leaf_obj);
			return;
		}
	}

	if (midgard_schema_object_tree_is_in_tree(leaf_obj, root_obj)) {
		RETVAL_TRUE;
	}

	g_object_unref(leaf_obj);
	g_object_unref(root_obj);
}

PHP_FUNCTION(_midgard_php_object_is_in_tree)
{
	RETVAL_FALSE;
	CHECK_MGD;
	long rootid, id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rootid, &id) == FAILURE)
		return;

	MidgardConnection *mgd = mgd_handle();
	zval *zval_object = getThis();
	zend_class_entry *base_class = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(zval_object));
	const char *php_classname = base_class->name;
	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);

	MidgardObject *leaf_obj = NULL, *root_obj = NULL;

	{
		GValue tmp_gval;
		g_value_set_int(&tmp_gval, id);

		leaf_obj = midgard_object_new(mgd, g_classname, &tmp_gval);

		if (!leaf_obj) {
			php_error(E_NOTICE, "Did not find object with id=%ld", id);
			return;
		}
	}

	{
		GValue tmp_gval;
		g_value_set_int(&tmp_gval, rootid);

		root_obj = midgard_object_new(mgd, g_classname, &tmp_gval);

		if (!root_obj) {
			php_error(E_NOTICE, "Did not find object with id=%ld", rootid);
			g_object_unref(leaf_obj);
			return;
		}
	}

	if (midgard_schema_object_tree_is_in_tree(leaf_obj, root_obj)) {
		RETVAL_TRUE;
	}

	g_object_unref(leaf_obj);
	g_object_unref(root_obj);
}

PHP_FUNCTION(_midgard_php_object_delete)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_delete_hook, 0);
		__THROW_EXCEPTION

		if (midgard_object_delete(mobj))
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_midgard_php_object_get_parent)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	MidgardObject *pobj;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		pobj = midgard_schema_object_tree_get_parent_object(mobj);

		if (pobj) {
			char *type_name = (char *)G_OBJECT_TYPE_NAME((GObject*)pobj);
			zend_class_entry *pce = zend_fetch_class(type_name, strlen(type_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

			object_init_ex(return_value, pce);
			MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(pobj));
			zend_call_method_with_0_params(&return_value, pce, &pce->constructor, "__construct", NULL);
		}
	}
}

PHP_FUNCTION(_midgard_php_object_list)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	array_init(return_value);

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		guint i, n_objects;
		MidgardObject **objects = midgard_schema_object_tree_list_objects(mobj, &n_objects);

		if (objects) {
			zend_class_entry *ce = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(zval_object));

			for (i = 0; i < n_objects; i++) {
				zval *zobject = NULL;

				MAKE_STD_ZVAL(zobject);
				object_init_ex(zobject, ce); /* Initialize new object for which QB has been created for */
				MGD_PHP_SET_GOBJECT(zobject, G_OBJECT(objects[i])); // inject our gobject
				zend_call_method_with_0_params(&zobject, ce, &ce->constructor, "__construct", NULL); /* Call class constructor on given instance */

				zend_hash_next_index_insert(HASH_OF(return_value), &zobject, sizeof(zval *), NULL);
			}
		}
	}
}

PHP_FUNCTION(_midgard_php_object_list_children)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	gchar *childcname;
	guint ccnl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &childcname, &ccnl)  == FAILURE) {
		return;
	}

	array_init(return_value);

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		guint i, n_objects;
		MidgardObject **objects = midgard_schema_object_tree_list_children_objects(mobj, childcname, &n_objects);

		if (objects != NULL) {
			zend_class_entry *ce = php_midgard_get_mgdschema_class_ptr_by_name(childcname);

			for (i = 0; i < n_objects; i++) {
				zval *zobject = NULL;

				MAKE_STD_ZVAL(zobject);
				object_init_ex(zobject, ce); /* Initialize new object for which QB has been created for */
				MGD_PHP_SET_GOBJECT(zobject, G_OBJECT(objects[i])); // inject our gobject
				zend_call_method_with_0_params(&zobject, ce, &ce->constructor, "__construct", NULL); /* Call class constructor on given instance */

				zend_hash_next_index_insert(HASH_OF(return_value), &zobject, sizeof(zval *), NULL);
			}
		}
	}
}

PHP_FUNCTION(php_midgard_object_has_dependents)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;

	if (object) {
		gboolean rv = midgard_object_has_dependents(object);
		RETURN_BOOL(rv);
	}
}

PHP_FUNCTION(_midgard_php_object_get_by_path)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	gchar *path;
	guint pathl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &pathl)  == FAILURE) {
		return;
	}

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		gboolean rv = midgard_object_get_by_path(mobj, path);
		RETURN_BOOL(rv);
	}
}

PHP_FUNCTION(_midgard_php_object_parent)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	const gchar *parent_class_name;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		parent_class_name = midgard_schema_object_tree_get_parent_name(mobj);
		if (parent_class_name)
			RETVAL_STRING((gchar *)parent_class_name, 1);
	}
}

PHP_FUNCTION(_php_midgard_object_purge)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (mobj) {
		g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_purge_hook, 0);
		__THROW_EXCEPTION

		if (midgard_object_purge(mobj))
			RETVAL_TRUE;
	}
}

PHP_FUNCTION(_php_midgard_object_undelete)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *guid;
	guint guid_length;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length)  == FAILURE) {
		return;
	}

	rv = midgard_schema_object_factory_object_undelete(mgd_handle(), (const gchar *)guid);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_connect)
{
	CHECK_MGD;
	php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

PHP_FUNCTION(_php_midgard_new_query_builder)
{
	CHECK_MGD;

	gchar *_cname_space = NULL;
	gchar *_class_name = get_active_class_name(&_cname_space TSRMLS_CC);

	MidgardQueryBuilder *builder =
		midgard_query_builder_new(mgd_handle(), _class_name);

	if (!builder)
		return;

	zend_class_entry *zval_object_ce = zend_fetch_class((char *)_class_name, strlen(_class_name), ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);
	zval *this_class_name;

	MAKE_STD_ZVAL(this_class_name);
	ZVAL_STRINGL(this_class_name, zval_object_ce->name, zval_object_ce->name_length, 1);

	object_init_ex(return_value, php_midgard_query_builder_class);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(builder));
	zend_call_method_with_1_params(
		&return_value,
		php_midgard_query_builder_class, &php_midgard_query_builder_class->constructor, "__construct",
		NULL,
		this_class_name
	);

	zval_ptr_dtor(&this_class_name);
}

PHP_FUNCTION(_php_midgard_new_collector)
{
	CHECK_MGD;

	zval *propname;
	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &propname, &zvalue) == FAILURE)
		return;

	gchar *_cname_space = NULL;
	gchar *_class_name = get_active_class_name(&_cname_space TSRMLS_CC);

	zend_class_entry *zval_object_ce = zend_fetch_class((char *)_class_name, strlen(_class_name), ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);
	zval *this_class_name;

	MAKE_STD_ZVAL(this_class_name);
	ZVAL_STRINGL(this_class_name, zval_object_ce->name, zval_object_ce->name_length, 1);

	object_init_ex(return_value, php_midgard_collector_class);
	zend_call_method_with_3_params(
		&return_value, // obj
		php_midgard_collector_class, &php_midgard_collector_class->constructor, "__construct", // ce, method, method_name
		NULL, // return value
		this_class_name, propname, zvalue // parameters
	);

	zval_ptr_dtor(&this_class_name);
}

PHP_FUNCTION(_php_midgard_new_reflection_property)
{
	CHECK_MGD;

	gchar *_cname_space = NULL;
	gchar *_class_name = get_active_class_name(&_cname_space TSRMLS_CC);

	MidgardObjectClass *klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)_class_name);

	MidgardReflectionProperty *mrp =
		midgard_reflection_property_new(MIDGARD_DBOBJECT_CLASS(klass));

	if (!mrp)
		return;

	object_init_ex(return_value, php_midgard_reflection_property_class);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(mrp));
	zend_call_method_with_0_params(
		&return_value,
		php_midgard_reflection_property_class, &php_midgard_reflection_property_class->constructor, "__construct",
		NULL
	);
}

PHP_FUNCTION(_php_midgard_object_set_guid)
{
	CHECK_MGD;

	gchar *guid = NULL;
	guint guid_length;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length) == FAILURE)
		return;

	MidgardObject *object = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	RETURN_BOOL(midgard_object_set_guid(object, (const gchar *)guid));
}

PHP_FUNCTION(_php_midgard_object_emit)
{
	CHECK_MGD;

	gchar *name = NULL;
	guint name_length;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_length) == FAILURE)
		return;

	MidgardObject *object = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	g_signal_emit_by_name(object, name);
}

PHP_FUNCTION(_php_midgard_object_approve)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_approve(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_is_approved)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_is_approved(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_unapprove)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_unapprove(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_lock)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_lock(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_is_locked)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_is_locked(object);
	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_unlock)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MIDGARD_OBJECT;
	gboolean rv = midgard_object_unlock(object);
	RETURN_BOOL(rv);
}

static struct
{
	char *fname;
	void (*handler)(INTERNAL_FUNCTION_PARAMETERS);
	zend_uint flags;
	zend_arg_info arg_info[8]; /* Keep it as reasonable high value. Or refactor so it doesn't have to be allocated statically */
	zend_uint num_args;
	char *doc_comment;
}

__midgard_php_type_functions[] =
{
	{"__construct",
		ZEND_FN(_midgard_php_object_constructor),
		ZEND_ACC_PUBLIC | ZEND_ACC_CTOR,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "guid or id", sizeof("guid or id")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 
		1,
		"Creates new empty instance or fetch object's data from storage if argument is of integer or string type. In latter case valid guid is expected. Throws midgard_error_exception on failure"
	},

	{"get_by_id",
		ZEND_FN(_get_type_by_id),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"get_by_guid",
		ZEND_FN(_midgard_php_object_get_by_guid),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 
		1, 
		"Get object which is identified by given guid"
	},

	{"update",
		ZEND_FN(_midgard_php_object_update),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"create",
		ZEND_FN(_midgard_php_object_create),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"is_in_parent_tree",
		ZEND_FN(_midgard_php_object_is_in_parent_tree),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 2
	},

	{"is_in_tree",
		ZEND_FN(_midgard_php_object_is_in_tree),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 2
	},

	{"has_dependents",
		ZEND_FN(php_midgard_object_has_dependents),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"delete",
		ZEND_FN(_midgard_php_object_delete),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"get_parent",
		ZEND_FN(_midgard_php_object_get_parent),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"list",
		ZEND_FN(_midgard_php_object_list),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"list_children",
		ZEND_FN(_midgard_php_object_list_children),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "node class name", sizeof("node class name")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"get_by_path",
		ZEND_FN(_midgard_php_object_get_by_path),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "path", sizeof("path")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"parent",
		ZEND_FN(_midgard_php_object_parent),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"has_parameters",
		ZEND_FN(php_midgard_object_has_parameters),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"list_parameters",
		ZEND_FN(_php_midgard_object_list_parameters),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 1
	},

	{"find_parameters",
		ZEND_FN(_php_midgard_object_find_parameters),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 1
	},

	{"delete_parameters",
		ZEND_FN(_php_midgard_object_delete_parameters),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 1
	},

	{"purge_parameters",
		ZEND_FN(_php_midgard_object_purge_parameters),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 1
	},

	{"get_parameter",
		ZEND_FN(_php_midgard_object_get_parameter),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 2
	},

	{"set_parameter",
		ZEND_FN(_php_midgard_object_set_parameter),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 3 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 3
	},

	{"parameter",
		ZEND_FN(_php_midgard_object_parameter),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0, 0, 0 },	
		}, 
	},

	{"has_attachments",
		ZEND_FN(php_midgard_object_has_attachments),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"list_attachments",
		ZEND_FN(_php_midgard_object_list_attachments),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"find_attachments",
		ZEND_FN(_php_midgard_object_find_attachments),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 1
	},

	{"delete_attachments",
		ZEND_FN(_php_midgard_object_delete_attachments),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
		}, 1
	},

	{"purge_attachments",
		ZEND_FN(_php_midgard_object_purge_attachments),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "delete blob", sizeof("delete blob")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 2
	},

	{"create_attachment",
		ZEND_FN(_php_midgard_object_create_attachment),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "title", sizeof("title")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "mimetype", sizeof("mimetype")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 3
	},

	{"serve_attachment",
		ZEND_FN(_php_midgard_object_serve_attachment),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"purge",
		ZEND_FN(_php_midgard_object_purge),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"undelete",
		ZEND_FN(_php_midgard_object_undelete),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"connect",
		ZEND_FN(_php_midgard_object_connect),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "callback", sizeof("callback")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "user_data", sizeof("user_data")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 3
	},

	{"new_query_builder",
		ZEND_FN(_php_midgard_new_query_builder),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"new_collector",
		ZEND_FN(_php_midgard_new_collector),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"new_reflection_property",
		ZEND_FN(_php_midgard_new_reflection_property),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"set_guid",
		ZEND_FN(_php_midgard_object_set_guid),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"emit",
		ZEND_FN(_php_midgard_object_emit),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0, 0, 0 },
		}, 1
	},

	{"approve",
		ZEND_FN(_php_midgard_object_approve),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"is_approved",
		ZEND_FN(_php_midgard_object_is_approved),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"unapprove",
		ZEND_FN(_php_midgard_object_unapprove),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"lock",
		ZEND_FN(_php_midgard_object_lock),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"is_locked",
		ZEND_FN(_php_midgard_object_is_locked),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{"unlock",
		ZEND_FN(_php_midgard_object_unlock),
		ZEND_ACC_PUBLIC,
		{
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
		}, 0
	},

	{ NULL, NULL }
};

int __serialize_object_hook(zval *zobject,
		unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	php_midgard_gobject *php_gobject = __php_objstore_object(zobject);

	if (!php_gobject)
		return FAILURE;

	if (!php_gobject->gobject)
		return FAILURE;

	GObject *gobject = G_OBJECT(php_gobject->gobject);
	GType object_type = G_OBJECT_TYPE(gobject);
	GType parent_object_type = g_type_parent(object_type);

	if (parent_object_type != MIDGARD_TYPE_DBOBJECT) {
		/* Try MIDGARD_OBJECT */
		if (parent_object_type != MIDGARD_TYPE_OBJECT)
			return FAILURE;
	}

	gchar *xml = midgard_replicator_serialize(G_OBJECT(gobject));

	if (!xml)
		return FAILURE;

	guint xml_length = strlen(xml)+1;
	*buffer = (unsigned char *)estrndup((const char*)xml, xml_length);
	*buf_len = xml_length;
	g_free(xml);

	return SUCCESS;
}

int __unserialize_object_hook(zval **zobject, zend_class_entry *ce,
		const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	if (buffer == NULL)
		return FAILURE;

	if (buf_len < 2)
		return FAILURE;

	GObject **objects =
		midgard_replicator_unserialize(mgd_handle(), (const gchar *)buffer, TRUE);

	if (!objects)
		return FAILURE;

	object_init_ex(*zobject, ce);
	MGD_PHP_SET_GOBJECT(*zobject, objects[0]);
	zend_call_method_with_0_params(zobject, ce, &ce->constructor, "__construct", NULL);

	g_free(objects);

	return SUCCESS;
}


static void __register_php_classes(const gchar *class_name, zend_class_entry *parent)
{
	zend_class_entry *mgdclass, *mgdclass_ptr;
	gint j;
	guint _am = 0;
	TSRMLS_FETCH();

	for (j = 0; __midgard_php_type_functions[j].fname; j++) {
		_am++;
	}

	zend_function_entry __functions[_am+1];

	/* lcn is freed in zend_register_internal_class */
	gchar *lcn = g_ascii_strdown(class_name, strlen(class_name));

	__functions[0].fname = "__construct";
	__functions[0].handler = ZEND_FN(_midgard_php_object_constructor);
	__functions[0].arg_info = __midgard_php_type_functions[0].arg_info;
	__functions[0].num_args = __midgard_php_type_functions[0].num_args;
	__functions[0].flags = ZEND_ACC_PUBLIC | ZEND_ACC_CTOR;

	for (j = 1; __midgard_php_type_functions[j].fname; j++) {
		__functions[j].fname = __midgard_php_type_functions[j].fname;
		__functions[j].handler = __midgard_php_type_functions[j].handler;
		__functions[j].arg_info = __midgard_php_type_functions[j].arg_info;
		__functions[j].num_args = __midgard_php_type_functions[j].num_args;
		__functions[j].flags = __midgard_php_type_functions[j].flags;
	}

	__functions[_am].fname = NULL;
	__functions[_am].handler = NULL;
	__functions[_am].arg_info = NULL;
	__functions[_am].num_args = 0;
	__functions[_am].flags = 0;

	// creating class-template
	mgdclass = g_new0(zend_class_entry, 1);
	mgdclass->name = lcn;
	mgdclass->name_length = strlen(class_name);
	mgdclass->builtin_functions = __functions;

	mgdclass->constructor = NULL;
	mgdclass->destructor = NULL;
	mgdclass->clone = NULL;
	mgdclass->create_object = NULL;
	mgdclass->interface_gets_implemented = NULL;
	mgdclass->__call = NULL;
	mgdclass->__get = NULL;
	mgdclass->__set = NULL;
	mgdclass->parent = parent;
	mgdclass->num_interfaces = 0;
	mgdclass->interfaces = NULL;
	mgdclass->get_iterator = NULL;
	mgdclass->iterator_funcs.funcs = NULL;
	mgdclass->module = NULL;
	mgdclass->ce_flags = 0;

	// registering class-template as class
	mgdclass_ptr = zend_register_internal_class(mgdclass TSRMLS_CC);
	mgdclass_ptr->ce_flags = 0;
	mgdclass_ptr->serialize = __serialize_object_hook;
	mgdclass_ptr->unserialize = __unserialize_object_hook;
	mgdclass_ptr->create_object = php_midgard_gobject_new;

	// freeing class-template (it is not needed anymore)
	g_free(mgdclass);
}

static void __add_method_comments(const char *class_name)
{
	guint j;

	for (j = 0; __midgard_php_type_functions[j].fname != NULL; j++) {
		php_midgard_docs_add_method_comment(class_name, __midgard_php_type_functions[j].fname, __midgard_php_type_functions[j].doc_comment);
	}
}

void php_midgard_object_init(int module_number)
{
	/* Register midgard_dbobject class */
	static zend_class_entry php_midgard_dbobject_ce;
	TSRMLS_FETCH();
	INIT_CLASS_ENTRY(php_midgard_dbobject_ce, "midgard_dbobject", NULL);
	php_midgard_dbobject_class = zend_register_internal_class(&php_midgard_dbobject_ce TSRMLS_CC);

	/* Register midgard_object class */
	static zend_class_entry php_midgard_object_ce;
	INIT_CLASS_ENTRY(php_midgard_object_ce, "midgard_object", NULL);
	php_midgard_object_class = zend_register_internal_class_ex(&php_midgard_object_ce, php_midgard_dbobject_class, "midgard_dbobject" TSRMLS_CC);

	guint n_types, i;
	const gchar *typename;
	GType *all_types = g_type_children(MIDGARD_TYPE_OBJECT, &n_types);

	for (i = 0; i < n_types; i++) {
		typename = g_class_name_to_php_class_name(g_type_name(all_types[i]));

		__register_php_classes(typename, php_midgard_object_class);
		__add_method_comments(typename);
	}

	g_free(all_types);
}
