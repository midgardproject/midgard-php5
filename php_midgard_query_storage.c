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
#include "php_midgard_gobject.h"

#include "php_midgard__helpers.h"

zend_class_entry *php_midgard_query_storage_class;

/* Object constructor */
static PHP_METHOD(midgard_query_storage, __construct)
{
	char *classname;
	int classname_length = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &classname_length) == FAILURE) {
		return;
	}

	const gchar *g_classname = php_class_name_to_g_class_name(classname);
	MidgardQueryStorage *storage = midgard_query_storage_new(g_classname);

	if (!storage) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Can not wrap this class in midgard_query_storage");
		return;
	}

	MGD_PHP_SET_GOBJECT(getThis(), storage);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_query_storage___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_query_storage)
{
	static zend_function_entry midgard_query_storage_methods[] = {
		PHP_ME(midgard_query_storage, __construct, arginfo_midgard_query_storage___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_query_storage_class_entry;
	INIT_CLASS_ENTRY(php_midgard_query_storage_class_entry, "MidgardQueryStorage", midgard_query_storage_methods);

	php_midgard_query_storage_class = zend_register_internal_class(&php_midgard_query_storage_class_entry TSRMLS_CC);
	php_midgard_query_storage_class->create_object = php_midgard_gobject_new;
	php_midgard_query_storage_class->doc_comment = strdup("Wraps DBObject for use with midgard_query_* classes");

	zend_register_class_alias("midgard_query_storage", php_midgard_query_storage_class);

	return SUCCESS;
}
