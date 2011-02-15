/* Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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

zend_class_entry *php_midgard_config_class;

/* Object constructor */
static PHP_METHOD(midgard_config, __construct)
{
	RETVAL_FALSE;

	zval *object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConfig *config = midgard_config_new();

	if (!config)
		RETURN_FALSE;

	MGD_PHP_SET_GOBJECT(object, config);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_config___construct, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, save_file)
{
	RETVAL_FALSE;
	zend_bool zbool = FALSE;
	char *name;
	int name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &name, &name_length, &zbool) == FAILURE) {
		return;
	}

	MidgardConfig *config = (MidgardConfig *) __php_gobject_ptr(getThis());
	gboolean rv = midgard_config_save_file(config ,name, zbool, NULL);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_save_file, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, read_file)
{
	RETVAL_FALSE;
	zend_bool zbool = FALSE;
	char *name;
	int name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &name, &name_length, &zbool) == FAILURE) {
		return;
	}

	MidgardConfig *config = (MidgardConfig *) __php_gobject_ptr(getThis());
	gboolean rv = midgard_config_read_file(config, name, zbool, NULL);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_read_file, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, read_file_at_path)
{
	RETVAL_FALSE;
	char *path;
	int path_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_length) == FAILURE) {
		return;
	}

	MidgardConfig *config = (MidgardConfig *) __php_gobject_ptr(getThis());
	gboolean rv = midgard_config_read_file_at_path(config, path, NULL);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_read_file_at_path, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, read_data)
{
	RETVAL_FALSE;
	char *data;
	int data_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_length) == FAILURE) {
		return;
	}

	MidgardConfig *config = (MidgardConfig *) __php_gobject_ptr(getThis());
	gboolean rv = midgard_config_read_data(config, data, NULL);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_read_data, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, list_files)
{
	RETVAL_FALSE;
	zend_bool user = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &user) == FAILURE) {
		return;
	}

	array_init(return_value);

	gchar **files = midgard_config_list_files(user);

	if (!files)
		return;

	size_t i = 0;
	while (files[i] != NULL) {
		add_index_string(return_value, i, files[i], 1);
		i++;
	}

	g_strfreev(files);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_config_list_files, 0, 0, 0)
	ZEND_ARG_INFO(0, user)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_config, create_blobdir)
{
	RETVAL_FALSE;
	gboolean rv;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConfig *config = NULL;

	if (zval_object) {
		config = (MidgardConfig *) __php_gobject_ptr(zval_object);
	}

	rv = midgard_config_create_blobdir(config);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_config_create_blobdir, 0)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_config)
{
	static function_entry midgard_config_methods[] = {
		PHP_ME(midgard_config, __construct,       arginfo_midgard_config___construct,       ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_config, save_file,         arginfo_midgard_config_save_file,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config, read_file,         arginfo_midgard_config_read_file,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config, read_file_at_path, arginfo_midgard_config_read_file_at_path, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config, read_data,         arginfo_midgard_config_read_data,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_config, list_files,        arginfo_midgard_config_list_files,        ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_config, create_blobdir,    arginfo_midgard_config_create_blobdir,    ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_config_class_entry;
	INIT_CLASS_ENTRY(php_midgard_config_class_entry, "midgard_config", midgard_config_methods);

	php_midgard_config_class = zend_register_internal_class(&php_midgard_config_class_entry TSRMLS_CC);
	php_midgard_config_class->create_object = php_midgard_gobject_new;
	php_midgard_config_class->doc_comment = strdup("Represents Midgard unified configuration file");

	return SUCCESS;
}
