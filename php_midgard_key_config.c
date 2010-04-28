/* 
 * Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
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

static const gchar *class_doc_comment = "Abstract class for key-value (ini like) configurations";

static zend_class_entry *php_midgard_key_config_class;

static PHP_METHOD(midgard_key_config, set_value)
{
	zval *zval_object = getThis();
	char *group = NULL, *key = NULL, *value = NULL;
	int group_length, key_length, value_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", 
				&group, &group_length, &key, &key_length, &value, &value_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	midgard_key_config_set_value (mkc, group, key, value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_set_value, 0, 0, 3)
	ZEND_ARG_INFO(0, group_name)
	ZEND_ARG_INFO(0, key_name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, get_value)
{
	zval *zval_object = getThis();
	char *group = NULL, *key = NULL;
	int group_length, key_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &group, &group_length, &key, &key_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	gchar *value = midgard_key_config_get_value (mkc, group, key);

	if (!value)
		RETURN_NULL();

	RETVAL_STRING(value, 1);

	g_free(value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_get_value, 0, 0, 3)
	ZEND_ARG_INFO(0, group_name)
	ZEND_ARG_INFO(0, key_name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, set_comment)
{
	zval *zval_object = getThis();
	char *group = NULL, *key = NULL, *comment = NULL;
	int group_length, key_length, comment_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", 
				&group, &group_length, &key, &key_length, &comment, &comment_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	midgard_key_config_set_comment (mkc, group, key, comment);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_set_comment, 0, 0, 3)
	ZEND_ARG_INFO(0, group_name)
	ZEND_ARG_INFO(0, key_name)
	ZEND_ARG_INFO(0, comment)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, get_comment)
{
	zval *zval_object = getThis();
	char *group = NULL, *key = NULL;
	int group_length, key_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &group, &group_length, &key, &key_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	gchar *comment = midgard_key_config_get_comment (mkc, group, key);

	if (!comment)
		RETURN_NULL();

	RETVAL_STRING(comment, 1);

	g_free(comment);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_get_comment, 0, 0, 3)
	ZEND_ARG_INFO(0, group_name)
	ZEND_ARG_INFO(0, key_name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, list_groups)
{
	RETVAL_FALSE;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	array_init (return_value);

	gint n_groups;
	gchar **groups = midgard_key_config_list_groups (mkc, &n_groups);

	if (!groups)
		return;

	guint i;
	for (i = 0; i < n_groups; i++) {

		add_assoc_string(return_value, (gchar *)groups[i], "", 1);
	}

	g_strfreev(groups);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_key_config_list_groups, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, group_exists)
{
	zval *zval_object = getThis();
	char *group = NULL;
	int group_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &group, &group_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	RETURN_BOOL(midgard_key_config_group_exists (mkc, group));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_group_exists, 0, 0, 1)
	ZEND_ARG_INFO(0, group_name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, delete_group)
{
	zval *zval_object = getThis();
	char *group = NULL;
	int group_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &group, &group_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	RETURN_BOOL(midgard_key_config_delete_group (mkc, group));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_delete_group, 0, 0, 1)
	ZEND_ARG_INFO(0, group_name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, store)
{
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	RETURN_BOOL(midgard_key_config_store (mkc));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_key_config_store, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, to_data)
{
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	gchar *data = midgard_key_config_to_data (mkc);

	if (!data)
		RETURN_NULL();

	RETVAL_STRING(data, 1);

	g_free(data);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_key_config_to_data, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config, load_from_data)
{
	zval *zval_object = getThis();
	char *data = NULL;
	int data_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_length) == FAILURE) 
		return;

	MidgardKeyConfig *mkc = (MidgardKeyConfig *) __php_gobject_ptr(zval_object);

	RETURN_BOOL(midgard_key_config_load_from_data (mkc, data));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_load_from_data, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_key_config)
{
	static function_entry midgard_key_config_methods[] = {
		PHP_ME(midgard_key_config, set_value,      arginfo_midgard_key_config_set_value,      ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, get_value,      arginfo_midgard_key_config_get_value,      ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, set_comment,    arginfo_midgard_key_config_set_comment,    ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, get_comment,    arginfo_midgard_key_config_get_comment,    ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, list_groups,    arginfo_midgard_key_config_list_groups,    ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, group_exists,   arginfo_midgard_key_config_group_exists,   ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, delete_group,   arginfo_midgard_key_config_delete_group,   ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, store,          arginfo_midgard_key_config_store,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, to_data,        arginfo_midgard_key_config_to_data,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_key_config, load_from_data, arginfo_midgard_key_config_load_from_data, ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_key_config_class_entry;

	INIT_CLASS_ENTRY(php_midgard_key_config_class_entry, "midgard_key_config", midgard_key_config_methods);

	php_midgard_key_config_class = zend_register_internal_class(&php_midgard_key_config_class_entry TSRMLS_CC);

	php_midgard_key_config_class->ce_flags = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_key_config_class->doc_comment = g_strdup(class_doc_comment);
	/* Set function to initialize underlying data */
	php_midgard_key_config_class->create_object = php_midgard_gobject_new;

	return SUCCESS;
}
