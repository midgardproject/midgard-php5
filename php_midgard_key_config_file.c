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

#include <zend_exceptions.h>

static zend_class_entry *php_midgard_key_config_file_class;

/* Object constructor */
static PHP_METHOD(midgard_key_config_file, __construct)
{
	RETVAL_FALSE;

	zval *object = getThis();
	char *path;
	int path_length;
	zval *ctx_object;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "os", &ctx_object, &path, &path_length) == FAILURE) 
		return;

	GError *error = NULL;
	MidgardKeyConfigFileContext *mkcf_ctx = (MidgardKeyConfigFileContext *) __php_gobject_ptr(ctx_object);
	MidgardKeyConfigFile *mkcf = midgard_key_config_file_new(mkcf_ctx, path, &error);

	if (error) {
		
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
				error && error->message ? error->message : "Unknown reason");
		g_clear_error (&error);
		return;
	}
	
	MGD_PHP_SET_GOBJECT(object, mkcf);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_file___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, context_object, midgard_key_config_file_context, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_key_config_file)
{
	static zend_function_entry midgard_key_config_file_methods[] = {
		PHP_ME(midgard_key_config_file, __construct, arginfo_midgard_key_config_file___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_key_config_file_class_entry;
	INIT_CLASS_ENTRY(php_midgard_key_config_file_class_entry, "MidgardKeyConfigFile", midgard_key_config_file_methods);

	php_midgard_key_config_file_class = zend_register_internal_class_ex(&php_midgard_key_config_file_class_entry, NULL, "midgard_key_config" TSRMLS_CC);

	CLASS_SET_DOC_COMMENT(php_midgard_key_config_file_class, strdup("File based key-value (ini like) configurations"));
	php_midgard_key_config_file_class->create_object = php_midgard_gobject_new;

	zend_register_class_alias("midgard_key_config_file", php_midgard_key_config_file_class);

	return SUCCESS;
}
