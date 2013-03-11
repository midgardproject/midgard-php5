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
#include <spl/spl_exceptions.h>

zend_class_entry *php_midgard_dbobject_class = NULL;
zend_class_entry *php_midgard_object_class = NULL;
zend_class_entry *php_midgard_base_abstract_class = NULL;

#define __THROW_EXCEPTION \
	if (EG(exception)) { \
		return_value = zend_throw_exception(Z_OBJCE_P(EG(exception)), "", 0 TSRMLS_CC); \
		return; \
	}

static zend_bool init_php_midgard_object_from_id(zval *instance, const char *php_classname, zval *objid TSRMLS_DC)
{
	MidgardObject *gobject = NULL;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
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
			} else if (!midgard_is_guid(Z_STRVAL_P(objid))) {
				zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0 TSRMLS_CC, "'%s' is not a valid guid", Z_STRVAL_P(objid));
				return FALSE;
			}
		} else {
			zend_throw_exception_ex(spl_ce_InvalidArgumentException, 0 TSRMLS_CC, "Wrong id-type for '%s' constructor. Expecting number or guid", php_classname);
			return FALSE;
		}

		GValue *value = php_midgard_zval2gvalue(objid TSRMLS_CC);
		gobject = midgard_object_new(mgd, (const gchar *) g_classname, value);
		g_value_unset(value);
		g_free(value);
	}

	if (!gobject) {
		php_midgard_error_exception_throw(mgd TSRMLS_CC);
		return FALSE;
	}

	MGD_PHP_SET_GOBJECT(instance, gobject);

	return TRUE;
}

/* Object constructor */
PHP_FUNCTION(_midgard_php_object_constructor)
{
	zval *zval_object = getThis();
	zend_class_entry *zce = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(zval_object));
	const char *zend_classname = zce->name;
	GObject *gobject;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] %s::__construct() \n", zval_object, zend_classname);
	}

	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		// this is a normal constructing process (we don't have assigned gobject yet)
		zval *objid = NULL;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z!", &objid) == FAILURE)
			return;

		if (init_php_midgard_object_from_id(zval_object, zend_classname, objid TSRMLS_CC) == FALSE)
			return;

		gobject = __php_gobject_ptr(zval_object);
	} else {
		// we already have gobject injected
	}

	php_midgard_object_connect_class_closures(gobject, zval_object TSRMLS_CC);
	g_signal_emit(gobject, MIDGARD_OBJECT_GET_CLASS(gobject)->signal_action_loaded_hook, 0);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= %s::__construct() \n", zval_object, zend_classname);
	}
}

PHP_FUNCTION(_midgard_php_object_create)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());

	g_signal_emit(object, MIDGARD_OBJECT_GET_CLASS(object)->signal_action_create_hook, 0);
	__THROW_EXCEPTION

	if (midgard_object_create(object)) 
		RETVAL_TRUE;
}

PHP_FUNCTION(_midgard_php_object_update)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);


	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());

	g_signal_emit(object, MIDGARD_OBJECT_GET_CLASS(object)->signal_action_update_hook, 0);
	__THROW_EXCEPTION

	if (midgard_object_update(object)) 
		RETVAL_TRUE;
}

PHP_FUNCTION(_midgard_php_object_get_by_id)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	long id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	if (!midgard_object_get_by_id(mobj, id)) {
		php_midgard_error_exception_throw(mgd TSRMLS_CC);
		return;
	}

	g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
	RETVAL_TRUE;
}

PHP_FUNCTION(_midgard_php_object_get_by_guid)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *guid;
	int glen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &glen)  == FAILURE) {
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	if (!midgard_object_get_by_guid(mobj, guid)) {
		php_midgard_error_exception_throw(mgd TSRMLS_CC);
		return;
	}

	g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_loaded_hook, 0);
	RETVAL_TRUE;
}

PHP_FUNCTION(_midgard_php_object_is_in_parent_tree)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	long rootid, id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rootid, &id) == FAILURE)
		return;

	/* Return TRUE if id or rootid is 0. */
	/* FIXME: Comment says "or", but code says "and". Who's right? */
	if (rootid == 0 && id == 0)
		RETURN_TRUE;

	zend_class_entry *zce = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(getThis()));
	const char *php_classname = zce->name;

	MidgardObject *leaf_obj = NULL, *root_obj = NULL;

	{
		const gchar *g_classname = php_class_name_to_g_class_name(php_classname);
		GValue tmp_gval = { 0 };
		g_value_init(&tmp_gval, G_TYPE_INT);
		g_value_set_int(&tmp_gval, id);

		leaf_obj = midgard_object_new(mgd, g_classname, &tmp_gval);

		if (!leaf_obj) {
			php_error(E_NOTICE, "Did not find object with id=%ld", id);
			return;
		}
	}

	{
		const gchar *g_parent_classname = midgard_schema_object_tree_get_parent_name(leaf_obj);
		GValue tmp_gval = { 0 };
		g_value_init(&tmp_gval, G_TYPE_INT);
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
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	long rootid, id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &rootid, &id) == FAILURE)
		return;

	zend_class_entry *zce = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(getThis()));

	const char *php_classname = zce->name;
	const gchar *g_classname = php_class_name_to_g_class_name(php_classname);

	MidgardObject *leaf_obj = NULL, *root_obj = NULL;

	{
		GValue tmp_gval = { 0 };
		g_value_init(&tmp_gval, G_TYPE_INT);
		g_value_set_int(&tmp_gval, id);

		leaf_obj = midgard_object_new(mgd, g_classname, &tmp_gval);

		if (!leaf_obj) {
			php_error(E_NOTICE, "Did not find object with id=%ld", id);
			return;
		}
	}

	{
		GValue tmp_gval = { 0 };
		g_value_init(&tmp_gval, G_TYPE_INT);
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
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zend_bool check_dependencies = TRUE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &check_dependencies) == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_delete_hook, 0);
	__THROW_EXCEPTION

	if (midgard_object_delete(mobj, check_dependencies))
		RETVAL_TRUE;
}

PHP_FUNCTION(_midgard_php_object_get_parent)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);


	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	GObject *pobj = G_OBJECT(midgard_schema_object_tree_get_parent_object(mobj));

	if (pobj) {
		gchar const *type_name = G_OBJECT_TYPE_NAME(pobj);
		zend_class_entry *pce = zend_fetch_class((char *)type_name, strlen(type_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

		php_midgard_gobject_new_with_gobject(return_value, pce, pobj, TRUE TSRMLS_CC);
	}
}

PHP_FUNCTION(_midgard_php_object_list)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);


	if (zend_parse_parameters_none() == FAILURE)
		return;

	array_init(return_value);

	zval *zval_object = getThis();
	MidgardObject *mobj = __midgard_object_get_ptr(zval_object);

	guint i, n_objects;
	MidgardObject **objects = midgard_schema_object_tree_list_objects(mobj, &n_objects);

	if (objects) {
		zend_class_entry *ce = php_midgard_get_mgdschema_class_ptr(Z_OBJCE_P(zval_object));

		for (i = 0; i < n_objects; i++) {
			zval *zobject = NULL;

			MAKE_STD_ZVAL(zobject);
			php_midgard_gobject_new_with_gobject(zobject, ce, G_OBJECT(objects[i]), TRUE TSRMLS_CC);

			zend_hash_next_index_insert(HASH_OF(return_value), &zobject, sizeof(zval *), NULL);
		}
	}
}

PHP_FUNCTION(_midgard_php_object_list_children)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *childcname;
	int ccnl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &childcname, &ccnl)  == FAILURE) {
		return;
	}

	array_init(return_value);

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	guint i, n_objects;
	MidgardObject **objects = midgard_schema_object_tree_list_children_objects(mobj, childcname, &n_objects);

	if (objects != NULL) {
		zend_class_entry *ce = php_midgard_get_mgdschema_class_ptr_by_name(childcname TSRMLS_CC);

		for (i = 0; i < n_objects; i++) {
			zval *zobject = NULL;

			MAKE_STD_ZVAL(zobject);
			php_midgard_gobject_new_with_gobject(zobject, ce, G_OBJECT(objects[i]), TRUE TSRMLS_CC);

			zend_hash_next_index_insert(HASH_OF(return_value), &zobject, sizeof(zval *), NULL);
		}
	}
}

PHP_FUNCTION(php_midgard_object_has_dependents)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_has_dependents(object));
}

PHP_FUNCTION(_midgard_php_object_get_by_path)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *path;
	int pathl;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &pathl)  == FAILURE) {
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_get_by_path(mobj, path));
}

PHP_FUNCTION(_midgard_php_object_parent)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	const gchar *parent_class_name = midgard_schema_object_tree_get_parent_name(mobj);

	if (parent_class_name)
		RETVAL_STRING((char *)parent_class_name, 1);
}

PHP_FUNCTION(_php_midgard_object_purge)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zend_bool check_dependencies = TRUE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &check_dependencies) == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	g_signal_emit(mobj, MIDGARD_OBJECT_GET_CLASS(mobj)->signal_action_purge_hook, 0);
	__THROW_EXCEPTION

	if (midgard_object_purge(mobj, check_dependencies))
		RETVAL_TRUE;
}

PHP_FUNCTION(_php_midgard_object_undelete)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *guid;
	int guid_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length)  == FAILURE) {
		return;
	}

	RETURN_BOOL(midgard_schema_object_factory_object_undelete(mgd, (const gchar *)guid));
}

PHP_FUNCTION(_php_midgard_object_connect)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

// static
PHP_FUNCTION(_php_midgard_new_query_builder)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	const char *_class_name = get_active_class_name(NULL TSRMLS_CC);
#else
	char *_class_name = get_active_class_name(NULL TSRMLS_CC);
#endif

	MidgardQueryBuilder *builder = midgard_query_builder_new(mgd, (gchar *)_class_name);

	if (!builder)
		return;

	zval *this_class_name = NULL;
	MAKE_STD_ZVAL(this_class_name);
	ZVAL_STRING(this_class_name, _class_name, 1);

	object_init_ex(return_value, php_midgard_query_builder_class);
	MGD_PHP_SET_GOBJECT(return_value, builder);
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
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *propname;
	zval *zvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &propname, &zvalue) == FAILURE)
		return;

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	const char *_class_name = get_active_class_name(NULL TSRMLS_CC);
#else
	char *_class_name = get_active_class_name(NULL TSRMLS_CC);
#endif

	zval *this_class_name = NULL;
	MAKE_STD_ZVAL(this_class_name);
	ZVAL_STRING(this_class_name, _class_name, 1);

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
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	const char *_class_name = get_active_class_name(NULL TSRMLS_CC);
#else
	char *_class_name = get_active_class_name(NULL TSRMLS_CC);
#endif

	MidgardObjectClass *klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME((const gchar *)_class_name);
	MidgardReflectionProperty *mrp = midgard_reflection_property_new(MIDGARD_DBOBJECT_CLASS(klass));

	if (!mrp)
		return;

	php_midgard_gobject_new_with_gobject(return_value, php_midgard_reflection_property_class, G_OBJECT(mrp), TRUE TSRMLS_CC);
}

PHP_FUNCTION(_php_midgard_object_set_guid)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *guid = NULL;
	int guid_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length) == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());

	RETURN_BOOL(midgard_object_set_guid(object, (const gchar *)guid));
}

PHP_FUNCTION(_php_midgard_object_emit)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *signal_name = NULL;
	int signal_name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &signal_name, &signal_name_length) == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());

	g_signal_emit_by_name(object, signal_name);
}

PHP_FUNCTION(_php_midgard_object_approve)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_approve(object));
}

PHP_FUNCTION(_php_midgard_object_is_approved)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_is_approved(object));
}

PHP_FUNCTION(_php_midgard_object_unapprove)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_unapprove(object));
}

PHP_FUNCTION(_php_midgard_object_lock)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_lock(object));
}

PHP_FUNCTION(_php_midgard_object_is_locked)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_is_locked(object));
}

PHP_FUNCTION(_php_midgard_object_unlock)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	RETURN_BOOL(midgard_object_unlock(object));
}

PHP_FUNCTION(_php_midgard_object_get_workspace)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *object = __midgard_object_get_ptr(getThis());
	MidgardWorkspace *workspace = midgard_object_get_workspace(object);
	if (!workspace)
		return;

	php_midgard_gobject_new_with_gobject(return_value, php_midgard_workspace_class, G_OBJECT(object), TRUE TSRMLS_CC);
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
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "guid or id", sizeof("guid or id")-1, NULL, 0, 0, 1 /* Allows NULL */, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "guid or id", sizeof("guid or id")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif

		}, 
		1,
		"Creates new empty instance or fetch object's data from storage if argument is of integer or string type. In latter case valid guid is expected. Throws midgard_error_exception on failure"
	},

	{"get_by_id",
		ZEND_FN(_midgard_php_object_get_by_id),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 1 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		1,
		"Get object which is identified by given id"
	},

	{"get_by_guid",
		ZEND_FN(_midgard_php_object_get_by_guid),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 
		1, 
		"Load data which is identified by given guid into current object"
	},

	{"update",
		ZEND_FN(_midgard_php_object_update),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		0,
		"Store object-changes into database"
	},

	{"create",
		ZEND_FN(_midgard_php_object_create),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		0,
		"Create object's record in database"
	},

	{"is_in_parent_tree",
		ZEND_FN(_midgard_php_object_is_in_parent_tree),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		2,
		"Check if object identified by 'id' is descendant of object identified by 'root_id'. Traversing is done using 'parent' markers in schema"
	},

	{"is_in_tree",
		ZEND_FN(_midgard_php_object_is_in_tree),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "root_id", sizeof("root_id")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "id", sizeof("id")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		2,
		"Check if object identified by 'id' is descendant of object identified by 'root_id'. Traversing is done using 'up' markers in schema"
	},

	{"has_dependents",
		ZEND_FN(php_midgard_object_has_dependents),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"delete",
		ZEND_FN(_midgard_php_object_delete),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "check_dependencies", sizeof("check_dependencies")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "check_dependencies", sizeof("check_dependencies")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		1,
		"Mark object's record in database as \"deleted\""
	},

	{"get_parent",
		ZEND_FN(_midgard_php_object_get_parent),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		0,
		"returns parent-object, using 'parent' mark in schema"
	},

	{"list",
		ZEND_FN(_midgard_php_object_list),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"list_children",
		ZEND_FN(_midgard_php_object_list_children),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "node class name", sizeof("node class name")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "node class name", sizeof("node class name")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"get_by_path",
		ZEND_FN(_midgard_php_object_get_by_path),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "path", sizeof("path")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "path", sizeof("path")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"parent",
		ZEND_FN(_midgard_php_object_parent),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		0,
		"returns name of class, which is marked as 'parent' in schema"
	},

	{"has_parameters",
		ZEND_FN(php_midgard_object_has_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"list_parameters",
		ZEND_FN(_php_midgard_object_list_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"find_parameters",
		ZEND_FN(_php_midgard_object_find_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"delete_parameters",
		ZEND_FN(_php_midgard_object_delete_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"purge_parameters",
		ZEND_FN(_php_midgard_object_purge_parameters),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"get_parameter",
		ZEND_FN(_php_midgard_object_get_parameter),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 2
	},

	{"set_parameter",
		ZEND_FN(_php_midgard_object_set_parameter),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 3 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 3
	},

	{"parameter",
		ZEND_FN(_php_midgard_object_parameter),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0 },	
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "domain", sizeof("domain")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "value", sizeof("value")-1, NULL, 0, 0, 0, 0, 0, 0 },	
#endif
		}, 
	},

	{"has_attachments",
		ZEND_FN(php_midgard_object_has_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"list_attachments",
		ZEND_FN(_php_midgard_object_list_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"find_attachments",
		ZEND_FN(_php_midgard_object_find_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"delete_attachments",
		ZEND_FN(_php_midgard_object_delete_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
#endif
		}, 1
	},

	{"purge_attachments",
		ZEND_FN(_php_midgard_object_purge_attachments),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
			{ "delete blob", sizeof("delete blob")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "constraints", sizeof("constraints")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "delete blob", sizeof("delete blob")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 2
	},

	{"create_attachment",
		ZEND_FN(_php_midgard_object_create_attachment),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "name", sizeof("name")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
			{ "title", sizeof("title")-1, NULL, 0, 0, 1 /* Allows NULL */, 0 },
			{ "mimetype", sizeof("mimetype")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "name", sizeof("name")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "title", sizeof("title")-1, NULL, 0, 0, 1 /* Allows NULL */, 0, 0, 0 },
			{ "mimetype", sizeof("mimetype")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 3
	},

	{"serve_attachment",
		ZEND_FN(_php_midgard_object_serve_attachment),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"purge",
		ZEND_FN(_php_midgard_object_purge),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
			{ "check_dependencies", sizeof("check_dependencies")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
			{ "check_dependencies", sizeof("check_dependencies")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"undelete",
		ZEND_FN(_php_midgard_object_undelete),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"connect",
		ZEND_FN(_php_midgard_object_connect),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0 },
			{ "callback", sizeof("callback")-1, NULL, 0, 0, 0, 0 },
			{ "user_data", sizeof("user_data")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 2 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "callback", sizeof("callback")-1, NULL, 0, 0, 0, 0, 0, 0 },
			{ "user_data", sizeof("user_data")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 3
	},

	{"new_query_builder",
		ZEND_FN(_php_midgard_new_query_builder),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"new_collector",
		ZEND_FN(_php_midgard_new_collector),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"new_reflection_property",
		ZEND_FN(_php_midgard_new_reflection_property),
		ZEND_ACC_STATIC|ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"set_guid",
		ZEND_FN(_php_midgard_object_set_guid),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "guid", sizeof("guid")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"emit",
		ZEND_FN(_php_midgard_object_emit),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0 },
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 1 },
			{ "signal", sizeof("signal")-1, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 1
	},

	{"approve",
		ZEND_FN(_php_midgard_object_approve),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"is_approved",
		ZEND_FN(_php_midgard_object_is_approved),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"unapprove",
		ZEND_FN(_php_midgard_object_unapprove),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"lock",
		ZEND_FN(_php_midgard_object_lock),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"is_locked",
		ZEND_FN(_php_midgard_object_is_locked),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"unlock",
		ZEND_FN(_php_midgard_object_unlock),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		}, 0
	},

	{"get_workspace",
		ZEND_FN(_php_midgard_object_get_workspace),
		ZEND_ACC_PUBLIC,
		{
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
			{ NULL, 0, NULL, 0, 0, 0, 0},
#else
			{ NULL, 0, NULL, 0, 0, 0, 0, 0, 0 },
#endif
		},
		0,
		"Get workspace associated with object. Returned workspace is valid per method call." 
	},

	{ NULL, NULL }
};

int php_midgard_serialize_dbobject_hook(zval *zobject, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	php_midgard_gobject *php_gobject = __php_objstore_object(zobject);

	if (!php_gobject)
		return FAILURE;

	if (!php_gobject->gobject)
		return FAILURE;

	GObject *gobject = php_gobject->gobject;
	GType object_type = G_OBJECT_TYPE(gobject);
	GType parent_object_type = g_type_parent(object_type);

	if (parent_object_type != MIDGARD_TYPE_DBOBJECT && parent_object_type != MIDGARD_TYPE_OBJECT) {
		return FAILURE;
	}

	gchar *xml = midgard_replicator_serialize(gobject);

	if (!xml)
		return FAILURE;

	guint xml_length = strlen(xml)+1;
	*buffer = (unsigned char *)estrndup((const char*)xml, xml_length);
	*buf_len = xml_length;
	g_free(xml);

	return SUCCESS;
}

int php_midgard_unserialize_dbobject_hook(zval **zobject, zend_class_entry *ce, const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	if (buffer == NULL)
		return FAILURE;

	if (buf_len < 2)
		return FAILURE;

	GObject **objects =
		midgard_replicator_unserialize(mgd_handle(TSRMLS_C), (const gchar *)buffer, TRUE);

	if (!objects)
		return FAILURE;

	php_midgard_gobject_new_with_gobject(*zobject, ce, objects[0], TRUE TSRMLS_CC);

	g_free(objects);

	return SUCCESS;
}

static zend_class_entry *
__find_class_by_name (const gchar *name)
{
	TSRMLS_FETCH();

       	int iface_name_length = strlen(name);
	char *lower_iface_name = g_ascii_strdown(name, iface_name_length);
	zend_class_entry **ce;

	if (zend_hash_find(CG(class_table), (char *)lower_iface_name, iface_name_length + 1, (void **) &ce) != SUCCESS) {
		return NULL;
	}
	g_free(lower_iface_name);
	return *ce;
}

static void 
__add_method_comments(const char *class_name)
{
	/*
	guint j;
	FIXME, Rewrite for PHP 5.4
	for (j = 0; __midgard_php_type_functions[j].fname != NULL; j++) {
		php_midgard_docs_add_method_comment(class_name, __midgard_php_type_functions[j].fname, __midgard_php_type_functions[j].doc_comment);
	} */
}

static void 
__register_php_class(const gchar *class_name, zend_class_entry *parent TSRMLS_DC)
{
	zend_class_entry *mgdclass, *mgdclass_ptr;
	gint j;
	guint _am = 0;
	
	zend_class_entry *ce = __find_class_by_name(class_name);
	if (ce != NULL)
		return;

	for (j = 0; __midgard_php_type_functions[j].fname; j++) {
		_am++;
	}

	zend_function_entry __functions[_am+1];

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
	int class_name_length = strlen(class_name);
	mgdclass = g_new0(zend_class_entry, 1);
	mgdclass->name = g_strdup (class_name);
	mgdclass->name_length = class_name_length;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	mgdclass->info.internal.builtin_functions = __functions;
#else
	mgdclass->builtin_functions = __functions;
#endif

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
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	mgdclass->info.internal.module = NULL;
#else
	mgdclass->module = NULL;
#endif
	mgdclass->ce_flags = 0;

	// registering class-template as class
	mgdclass_ptr = zend_register_internal_class(mgdclass TSRMLS_CC);
	mgdclass_ptr->ce_flags = 0;
	mgdclass_ptr->serialize = php_midgard_serialize_dbobject_hook;
	mgdclass_ptr->unserialize = php_midgard_unserialize_dbobject_hook;
	mgdclass_ptr->create_object = php_midgard_gobject_new;

	/* Get class interfaces and add php ones */
	guint n_types;
	guint i;
	GType *iface_types = g_type_interfaces(g_type_from_name(class_name), &n_types);
	for (i = 0; i < n_types; i++) {
		zend_class_entry *iface_ce = __find_class_by_name(g_type_name(iface_types[i]));	
		zend_class_implements(mgdclass_ptr TSRMLS_CC, 1, iface_ce);
	}	
	g_free(iface_types);

	// freeing class-template (it is not needed anymore)
	g_free(mgdclass);

	/* Register all derived classes */
	GType *derived = g_type_children(g_type_from_name(class_name), &n_types);
	for (i = 0; i < n_types; i++) {
		const gchar *typename = g_type_name(derived[i]);
		__register_php_class(typename, mgdclass_ptr TSRMLS_CC);
		__add_method_comments(typename);
	}

}

PHP_MINIT_FUNCTION(midgard2_object)
{
	/* Register midgard_dbobject class */
	static zend_class_entry php_midgard_dbobject_ce;
	INIT_CLASS_ENTRY(php_midgard_dbobject_ce, "MidgardDBObject", NULL);

	php_midgard_dbobject_class = zend_register_internal_class(&php_midgard_dbobject_ce TSRMLS_CC);
	php_midgard_dbobject_class->ce_flags = ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	zend_register_class_alias("midgard_dbobject", php_midgard_dbobject_class);

	/* Register midgard_object class */
	static zend_class_entry php_midgard_object_ce;
	INIT_CLASS_ENTRY(php_midgard_object_ce, "MidgardObject", NULL);

	php_midgard_object_class = zend_register_internal_class_ex(&php_midgard_object_ce, php_midgard_dbobject_class, "MidgardDBObject" TSRMLS_CC);
	php_midgard_object_class->ce_flags = ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	zend_register_class_alias ("midgard_object", php_midgard_object_class);


	guint n_types, i;
	GType *all_types = g_type_children(MIDGARD_TYPE_OBJECT, &n_types);

	for (i = 0; i < n_types; i++) {
		const gchar *typename = g_type_name(all_types[i]);
		__register_php_class(typename, php_midgard_object_class TSRMLS_CC);
		__add_method_comments(typename);
	}

	g_free(all_types);

	return SUCCESS;
}

static void __register_abstract_php_classes(const gchar *class_name, zend_class_entry *parent TSRMLS_DC)
{
	zend_class_entry *mgdclass, *mgdclass_ptr;

	/* lcn is freed in zend_register_internal_class */
	gchar *lcn = g_ascii_strdown(class_name, strlen(class_name));

	// creating class-template
	mgdclass = g_new0(zend_class_entry, 1);
	mgdclass->name = lcn;
	mgdclass->name_length = strlen(class_name);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	mgdclass->info.internal.builtin_functions = NULL;
#else
	mgdclass->builtin_functions = NULL;
#endif

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
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	mgdclass->info.internal.module = NULL;
#else
	mgdclass->module = NULL;
#endif
	mgdclass->ce_flags = 0;

	/* registering class-template as class 
	 * From this class we need nothing but properties, so no need to define 
	 * object initialization routine or other ones */	
	mgdclass_ptr = zend_register_internal_class(mgdclass TSRMLS_CC);
	mgdclass_ptr->ce_flags = ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	/* Register default properties */
	guint n_prop;
	guint i;
	GObjectClass *klass = g_type_class_peek (g_type_from_name (class_name));
	GParamSpec **pspecs = g_object_class_list_properties (klass, &n_prop);
	for (i = 0; i < n_prop; i++) {
		/* By default, register string property */
		zend_declare_property_string (mgdclass_ptr, (char*) pspecs[i]->name, strlen (pspecs[i]->name), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	}
	g_free(pspecs);

	// freeing class-template (it is not needed anymore)
	g_free(mgdclass);
}

PHP_MINIT_FUNCTION(midgard2_base_abstract)
{
	/* Register MidgardBaseAbstract class */
	static zend_class_entry php_midgard_base_abstract_ce;
	INIT_CLASS_ENTRY(php_midgard_base_abstract_ce, "MidgardBaseAbstract", NULL);

	php_midgard_base_abstract_class = zend_register_internal_class(&php_midgard_base_abstract_ce TSRMLS_CC);

	guint n_types, i;
	GType *all_types = g_type_children(MIDGARD_TYPE_BASE_ABSTRACT, &n_types);

	for (i = 0; i < n_types; i++) {
		const gchar *typename = g_type_name(all_types[i]);
		__register_abstract_php_classes(typename, php_midgard_base_abstract_class TSRMLS_CC);
	}

	g_free(all_types);

	return SUCCESS;
}
