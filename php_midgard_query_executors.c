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


static void php_midgard_array_from_unknown_objects(MidgardDBObject **objects, zval *zarray TSRMLS_DC);


zend_class_entry *php_midgard_query_executor_class;
zend_class_entry *php_midgard_query_select_class;

// midgard_query_executor
static PHP_METHOD(midgard_query_executor, set_constraint)
{
	zval *z_constraint = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_constraint, php_midgard_query_constraint_simple_class) == FAILURE) {
		return;
	}

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));
	MidgardQueryConstraintSimple *constraint = MIDGARD_QUERY_CONSTRAINT_SIMPLE(__php_gobject_ptr(z_constraint));

	zend_bool result = midgard_query_executor_set_constraint(executor, constraint);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_set_constraint, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, property, midgard_query_constraint_simple, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_executor, set_offset)
{
	long offset = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) == FAILURE) {
		return;
	}

	if (offset < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "offset can't be negative");
		return;
	}

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));

	zend_bool result = midgard_query_executor_set_offset(executor, offset);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_set_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_executor, set_limit)
{
	long limit = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &limit) == FAILURE) {
		return;
	}

	if (limit < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "limit can't be negative");
		return;
	}

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));

	zend_bool result = midgard_query_executor_set_limit(executor, limit);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_set_limit, 0, 0, 1)
	ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_executor, add_order)
{
	zval *z_property = NULL;
	long direction = PHP_SORT_ASC;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l", &z_property, php_midgard_query_property_class, &direction) == FAILURE) {
		return;
	}

	if (direction != PHP_SORT_ASC && direction != PHP_SORT_DESC) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "direction should be either SORT_ASC or SORT_DESC");
		return;
	}

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));
	MidgardQueryProperty *property = MIDGARD_QUERY_PROPERTY(__php_gobject_ptr(z_property));
	const gchar *g_direction = (direction == PHP_SORT_ASC) ? "ASC" : "DESC";

	zend_bool result = midgard_query_executor_add_order(executor, property, g_direction);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_add_order, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, property, midgard_query_property, 0)
	ZEND_ARG_INFO(0, direction)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_executor, add_join)
{
	char *join_type = NULL;
	int join_type_len = 0;
	zval *z_property_left = NULL;
	zval *z_property_right = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sOO",
			&join_type, &join_type_len,
			&z_property_left, php_midgard_query_property_class,
			&z_property_right, php_midgard_query_property_class
		) == FAILURE
	) {
		return;
	}

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));
	MidgardQueryProperty *left_property = MIDGARD_QUERY_PROPERTY(__php_gobject_ptr(z_property_left));
	MidgardQueryProperty *right_property = MIDGARD_QUERY_PROPERTY(__php_gobject_ptr(z_property_right));

	zend_bool result = midgard_query_executor_add_join(executor, join_type, left_property, right_property);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_add_join, 0, 0, 3)
	ZEND_ARG_INFO(0, join_type)
	ZEND_ARG_OBJ_INFO(0, left_property, midgard_query_property, 0)
	ZEND_ARG_OBJ_INFO(0, right_property, midgard_query_property, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_executor, execute)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));

	zend_bool result = midgard_query_executor_execute(executor);

	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_execute, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_executor, get_results_count)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQueryExecutor *executor = MIDGARD_QUERY_EXECUTOR(__php_gobject_ptr(getThis()));

	guint count = midgard_query_executor_get_results_count(executor);

	RETURN_LONG(count);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_executor_get_results_count, 0, 0, 0)
ZEND_END_ARG_INFO()


// midgard_query_select
static PHP_METHOD(midgard_query_select, __construct)
{
	zval *z_storage = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_storage, php_midgard_query_storage_class) == FAILURE) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to create Select executor");
		return;
	}

	MidgardConnection *mgd = mgd_handle(TSRMLS_C);

	if (!mgd) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "midgard_query_select object requires active midgard_connection");
		return;
	}

	MidgardQueryStorage *storage = MIDGARD_QUERY_STORAGE(__php_gobject_ptr(z_storage));

	MidgardQuerySelect *executor = midgard_query_select_new(mgd, storage);

	if (!executor) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Failed to create Select executor");
		return;
	}

	MGD_PHP_SET_GOBJECT(getThis(), executor);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_select___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, storage, midgard_query_storage, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_select, toggle_readonly)
{
	zend_bool readonly;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &readonly) == FAILURE) {
		return;
	}

	MidgardQuerySelect *select = MIDGARD_QUERY_SELECT(__php_gobject_ptr(getThis()));
	midgard_query_select_toggle_read_only(select, readonly);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_select_toggle_readonly, 0, 0, 1)
	ZEND_ARG_INFO(0, read_only)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_select, include_deleted)
{
	zend_bool include_deleted;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &include_deleted) == FAILURE) {
		return;
	}

	MidgardQuerySelect *select = MIDGARD_QUERY_SELECT(__php_gobject_ptr(getThis()));
	midgard_query_select_include_deleted(select, include_deleted);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_select_include_deleted, 0, 0, 1)
	ZEND_ARG_INFO(0, include_deleted)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_select, list_objects)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQuerySelect *select = MIDGARD_QUERY_SELECT(__php_gobject_ptr(getThis()));

	guint n_objects;
	MidgardDBObject **objects = midgard_query_select_list_objects(select, &n_objects);

	array_init(return_value);
	php_midgard_array_from_unknown_objects(objects, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_select_list_objects, 0, 0, 0)
ZEND_END_ARG_INFO()


PHP_MINIT_FUNCTION(midgard2_query_executors)
{
	static function_entry midgard_query_executor_methods[] = {
		PHP_ME(midgard_query_executor, set_constraint,    arginfo_midgard_query_executor_set_constraint,    ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_executor, set_offset,        arginfo_midgard_query_executor_set_offset,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_executor, set_limit,         arginfo_midgard_query_executor_set_limit,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_executor, add_order,         arginfo_midgard_query_executor_add_order,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_executor, add_join,          arginfo_midgard_query_executor_add_join,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_executor, execute,           arginfo_midgard_query_executor_execute,           ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_executor, get_results_count, arginfo_midgard_query_executor_get_results_count, ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_executor_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_executor_class_entry, "midgard_query_executor", midgard_query_executor_methods);

	php_midgard_query_executor_class = zend_register_internal_class(&php_midgard_query_executor_class_entry TSRMLS_CC);
	php_midgard_query_executor_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_query_executor_class->create_object = php_midgard_gobject_new;
	php_midgard_query_executor_class->doc_comment = strdup("Base class for midgard_query query executors");


	static function_entry midgard_query_select_methods[] = {
		PHP_ME(midgard_query_select, __construct,      arginfo_midgard_query_select___construct,      ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_query_select, toggle_readonly,  arginfo_midgard_query_select_toggle_readonly,  ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_select, include_deleted,  arginfo_midgard_query_select_include_deleted,  ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_select, list_objects,     arginfo_midgard_query_select_list_objects,  ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_select_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_select_class_entry, "midgard_query_select", midgard_query_select_methods);

	php_midgard_query_select_class = zend_register_internal_class_ex(&php_midgard_query_select_class_entry, php_midgard_query_executor_class, "midgard_query_executor" TSRMLS_CC);
	php_midgard_query_select_class->create_object = php_midgard_gobject_new;
	php_midgard_query_select_class->doc_comment = strdup("midgard_query Select query executor");


	return SUCCESS;
}



// ===========================
// = Helper-functions follow =
// ===========================

static void php_midgard_array_from_unknown_objects(MidgardDBObject **objects, zval *zarray TSRMLS_DC)
{
	if (!objects)
		return;

	size_t i = 0;

	while (objects[i] != NULL) {
		GObject *object = G_OBJECT(objects[i]);
		GType object_type = G_OBJECT_TYPE(object);
		const gchar *g_class_name = g_type_name(object_type);
		const char *php_class_name = g_class_name_to_php_class_name(g_class_name);

		zend_class_entry *ce = zend_fetch_class((char *)php_class_name, strlen(php_class_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);;

		zval *zobject;
		MAKE_STD_ZVAL(zobject);

		object_init_ex(zobject, ce);
		MGD_PHP_SET_GOBJECT(zobject, object);
		zend_call_method_with_0_params(&zobject, ce, &ce->constructor, "__construct", NULL);

		zend_hash_next_index_insert(HASH_OF(zarray), &zobject, sizeof(zval *), NULL);

		i++;
	}

	return;
}
