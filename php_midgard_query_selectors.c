/* Copyright (C) 2010 Alexey Zakhlestin <indeyets@gmail.com>
 * Copyright (C) 2012 Piotr Pokora <piotrek.pokora@gmail.com>
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

#include <midgard/midgard_executable.h>

/* midgard core interfaces */
zend_class_entry *php_midgard_query_column_class;
zend_class_entry *php_midgard_query_row_class;
zend_class_entry *php_midgard_query_selector_class;
zend_class_entry *php_midgard_query_result_class;

/* midgard core sql classes */
zend_class_entry *php_midgard_sql_query_column_class;
zend_class_entry *php_midgard_sql_query_row_class;
zend_class_entry *php_midgard_sql_query_constraint_class;
zend_class_entry *php_midgard_sql_query_result_class;
zend_class_entry *php_midgard_sql_query_select_data_class;

/*	QueryColumn	*/

static PHP_METHOD(midgard_query_column, get_name)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQueryColumn *column = MIDGARD_QUERY_COLUMN(__php_gobject_ptr(getThis()));
	const gchar *name = midgard_query_column_get_name(column, NULL);
	if (name == NULL)
		RETURN_NULL();
		
	RETURN_STRING(name, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_column_get_name, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_column, get_qualifier)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQueryColumn *column = MIDGARD_QUERY_COLUMN(__php_gobject_ptr(getThis()));
	const gchar *qualifier = midgard_query_column_get_qualifier(column, NULL);
	if (qualifier == NULL)
		RETURN_NULL();
		
	RETURN_STRING(qualifier, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_column_get_qualifier, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_column, get_query_property)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQueryColumn *column = MIDGARD_QUERY_COLUMN(__php_gobject_ptr(getThis()));
	MidgardQueryProperty *property = midgard_query_column_get_query_property(column, NULL);
	if (property == NULL)
		RETURN_NULL();
		
	php_midgard_gobject_new_with_gobject(return_value, php_midgard_query_property_class, G_OBJECT(property), TRUE TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_column_get_query_property, 0, 0, 0)
ZEND_END_ARG_INFO()


/*	QueryRow	*/

static PHP_METHOD(midgard_query_row, get_object)
{
	char *name;
	int name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_length) == FAILURE)
		return;

	MidgardQueryRow *row = MIDGARD_QUERY_ROW(__php_gobject_ptr(getThis()));
	GObject *object = midgard_query_row_get_object(row, (const gchar *)name, NULL);
	MGD_PHP_SET_GOBJECT(return_value, g_object_ref(object));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_row_get_object, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_row, get_value)
{
	char *name;
	int name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_length) == FAILURE)
		return;

	MidgardQueryRow *row = MIDGARD_QUERY_ROW(__php_gobject_ptr(getThis()));
	const GValue *value = midgard_query_row_get_value(row, (const gchar *)name, NULL); 
	
	php_midgard_gvalue2zval(value, return_value TSRMLS_CC);	
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_row_get_value, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_row, get_values)
{
	if (zend_parse_parameters_none() == FAILURE)
		                return;

	MidgardQueryRow *row = MIDGARD_QUERY_ROW(__php_gobject_ptr(getThis()));
	GValueArray *varray = midgard_query_row_get_values(row, NULL); 
	array_init (return_value);
	if (varray == NULL)
		return;

	guint i;
	for (i = 0; i < varray->n_values; i++) {
		GValue *val = g_value_array_get_nth(varray, i);
		zval *z_val;
		MAKE_STD_ZVAL(z_val);
		php_midgard_gvalue2zval(val, z_val TSRMLS_CC);
		add_index_zval(return_value, i, z_val);
	}

	g_value_array_free (varray);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_row_get_values, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

/*	QuerySelector	*/

static PHP_METHOD(midgard_query_selector, get_connection)
{

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQuerySelector *selector = MIDGARD_QUERY_SELECTOR(__php_gobject_ptr(getThis()));
	MidgardConnection *mgd = midgard_query_selector_get_connection(selector);
	MGD_PHP_SET_GOBJECT(return_value, g_object_ref(G_OBJECT(mgd)));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_selector_get_connection, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_selector, get_query_result)
{

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQuerySelector *selector = MIDGARD_QUERY_SELECTOR(__php_gobject_ptr(getThis()));
	MidgardQueryResult *result = midgard_query_selector_get_query_result(selector, NULL);
	MGD_PHP_SET_GOBJECT(return_value, g_object_ref(G_OBJECT(result)));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_selector_get_query_result, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_selector, get_query_string)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardQuerySelector *selector = MIDGARD_QUERY_SELECTOR(__php_gobject_ptr(getThis()));
	const gchar *query_string = midgard_query_selector_get_query_string(selector);
	if (query_string == NULL)
		RETURN_NULL();
		
	RETURN_STRING(query_string, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_selector_get_query_string, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	QueryResult	*/

static PHP_METHOD(midgard_query_result, get_columns)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	guint n_objects;
	MidgardQueryResult *result = MIDGARD_QUERY_RESULT(__php_gobject_ptr(getThis()));
	MidgardQueryColumn **columns = midgard_query_result_get_columns(result, &n_objects, NULL);
	array_init(return_value);
	php_midgard_array_from_unknown_objects((GObject **)columns, n_objects, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_result_get_columns, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_result, get_objects)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	guint n_objects;
	MidgardQueryResult *result = MIDGARD_QUERY_RESULT(__php_gobject_ptr(getThis()));
	GObject **objects = midgard_query_result_get_objects(result, &n_objects, NULL);
	array_init(return_value);
	php_midgard_array_from_unknown_objects(objects, n_objects, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_result_get_objects, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_result, get_rows)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	guint n_objects;
	MidgardQueryResult *result = MIDGARD_QUERY_RESULT(__php_gobject_ptr(getThis()));
	MidgardQueryRow **rows = midgard_query_result_get_rows(result, &n_objects, NULL);
	array_init(return_value);
	php_midgard_array_from_unknown_objects((GObject **)rows, n_objects, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_result_get_rows, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_query_result, get_column_names)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	guint n_names;
	MidgardQueryResult *result = MIDGARD_QUERY_RESULT(__php_gobject_ptr(getThis()));
	gchar **names = midgard_query_result_get_column_names(result, &n_names, NULL);

	array_init(return_value);
	if (names == NULL)
		return;

	guint i;
	for (i = 0; i < n_names; i++) {
		add_assoc_string(return_value, (gchar *)names[i], "", 1);
	}
	
	g_free(names);	
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_result_get_column_names, 0, 0, 0)
ZEND_END_ARG_INFO()



PHP_MINIT_FUNCTION(midgard2_query_selectors)
{
	/*	QueryColumn	*/
	static zend_function_entry midgard_query_column_methods[] = {
		PHP_ME(midgard_query_column,	get_name,		arginfo_midgard_query_column_get_name,			ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_column,	get_qualifier,		arginfo_midgard_query_column_get_qualifier,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_column,	get_query_property,	arginfo_midgard_query_column_get_query_property,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_column_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_column_class_entry, "MidgardQueryColumn", midgard_query_column_methods);

	php_midgard_query_column_class = zend_register_internal_class(&php_midgard_query_column_class_entry TSRMLS_CC);
	php_midgard_query_column_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_query_column_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_query_column_class, strdup("Base, abstract class for column in query result"));

	zend_register_class_alias("midgard_query_column", php_midgard_query_column_class);

	/*	QueryRow	*/
	static zend_function_entry midgard_query_row_methods[] = {
		PHP_ME(midgard_query_row,	get_object,	arginfo_midgard_query_row_get_object,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_row,	get_value,	arginfo_midgard_query_row_get_value,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_row,	get_values,	arginfo_midgard_query_row_get_values,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_row_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_row_class_entry, "MidgardQueryRow", midgard_query_row_methods);

	php_midgard_query_row_class = zend_register_internal_class(&php_midgard_query_row_class_entry TSRMLS_CC);
	php_midgard_query_row_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_query_row_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_query_row_class, strdup("Base, abstract class for a row in query result"));

	zend_register_class_alias("midgard_query_row", php_midgard_query_row_class);

	/*	QuerySelector	*/
	static zend_function_entry midgard_query_selector_methods[] = {
		PHP_ME(midgard_query_selector,	get_connection,		arginfo_midgard_query_selector_get_connection,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_selector,	get_query_result,	arginfo_midgard_query_selector_get_query_result,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_selector,	get_query_string,	arginfo_midgard_query_selector_get_query_string,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_selector_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_selector_class_entry, "MidgardQuerySelector", midgard_query_selector_methods);

	php_midgard_query_selector_class = zend_register_internal_class(&php_midgard_query_selector_class_entry TSRMLS_CC);
	php_midgard_query_selector_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_query_selector_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_query_selector_class, strdup("Base, abstract class for a query selector"));

	zend_register_class_alias("midgard_query_selector", php_midgard_query_selector_class);

	/*	QueryResult	*/
	static zend_function_entry midgard_query_result_methods[] = {
		PHP_ME(midgard_query_result,	get_objects,		arginfo_midgard_query_result_get_objects,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_result,	get_columns,		arginfo_midgard_query_result_get_columns,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_result,	get_rows,		arginfo_midgard_query_result_get_rows,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_query_result,	get_column_names,	arginfo_midgard_query_result_get_column_names,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_result_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_result_class_entry, "MidgardQueryResult", midgard_query_result_methods);

	php_midgard_query_result_class = zend_register_internal_class(&php_midgard_query_result_class_entry TSRMLS_CC);
	php_midgard_query_result_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_query_result_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_query_result_class, strdup("Base, abstract class for a query result"));

	zend_register_class_alias("midgard_query_result", php_midgard_query_result_class);

	return SUCCESS;
}
