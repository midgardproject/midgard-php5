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

static zend_class_entry *php_midgard_key_config_context_class;

/* Object constructor */
// static PHP_METHOD(midgard_key_config_context, __construct)
// {
// 	RETVAL_FALSE;
// 
// 	zval *object = getThis();
// 	char *path;
// 	int path_length;
// 
// 	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_length) == FAILURE) 
// 		return;
// 
// 	MidgardKeyConfigContext *kctx = midgard_key_config_context_new(path);
// 
// 	if (!kctx)
// 		RETURN_FALSE;
// 
// 	MGD_PHP_SET_GOBJECT(object, kctx);
// }
// 
// ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_key_config_context___construct, 0, 0, 1)
// 	ZEND_ARG_INFO(0, context_path)
// ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_key_config_context, list_key_config)
{
	RETVAL_FALSE;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardKeyConfigContext *kctx = (MidgardKeyConfigContext *) __php_gobject_ptr(zval_object);

	array_init (return_value);

	gint n_cfgs;
	gchar **cfgs = midgard_key_config_context_list_key_config (kctx, &n_cfgs);

	if (!cfgs)
		return;

	guint i;
	for (i = 0; i < n_cfgs; i++) {

		add_assoc_string(return_value, (gchar *)cfgs[i], "", 1);
	}

        g_strfreev(cfgs);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_key_config_context_list_key_config, 0)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_key_config_context)
{
	static zend_function_entry midgard_key_config_context_methods[] = {
		// PHP_ME(midgard_key_config_context, __construct,     arginfo_midgard_key_config_context___construct,     ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_key_config_context, list_key_config, arginfo_midgard_key_config_context_list_key_config, ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_key_config_context_class_entry;
	INIT_CLASS_ENTRY(php_midgard_key_config_context_class_entry, "MidgardKeyConfigContext", midgard_key_config_context_methods);

	php_midgard_key_config_context_class = zend_register_internal_class(&php_midgard_key_config_context_class_entry TSRMLS_CC);

	php_midgard_key_config_context_class->ce_flags = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_key_config_context_class->doc_comment = strdup("Abstract class for key-value (ini like) configuration contexts");
	php_midgard_key_config_context_class->create_object = php_midgard_gobject_new;

	zend_register_class_alias("midgard_key_config_context", php_midgard_key_config_context_class);

	return SUCCESS;
}
