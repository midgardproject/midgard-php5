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

static const gchar *class_doc_comment = "Context for key-value (ini like) configurations stored in files";

static zend_class_entry *php_midgard_key_config_file_context_class;

/* Object constructor */
static PHP_METHOD(midgard_key_config_file_context, __construct)
{
	RETVAL_FALSE;

	zval *object = getThis();
	gchar *path;
	guint path_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_length) == FAILURE)
		return;

	GError *error = NULL;
	MidgardKeyConfigFileContext *kctx = midgard_key_config_file_context_new(path, &error);

	if (error) {

		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, 
				error && error->message ? error->message : "Unknown reason");
		g_clear_error (&error);
		return;
	}

	MGD_PHP_SET_GOBJECT(object, kctx);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_file_context___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, context_path)
ZEND_END_ARG_INFO()


/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_key_config_file_context)
{
	static function_entry midgard_key_config_file_context_methods[] = {
		PHP_ME(midgard_key_config_file_context, __construct, arginfo_midgard_key_config_file_context___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_key_config_file_context_class_entry;
	INIT_CLASS_ENTRY(php_midgard_key_config_file_context_class_entry, "midgard_key_config_file_context", midgard_key_config_file_context_methods);

	php_midgard_key_config_file_context_class = zend_register_internal_class_ex(&php_midgard_key_config_file_context_class_entry, NULL, "midgard_key_config_context" TSRMLS_CC);

	php_midgard_key_config_file_context_class->doc_comment = g_strdup(class_doc_comment);
	php_midgard_key_config_file_context_class->create_object = php_midgard_gobject_new;

	return SUCCESS;
}
