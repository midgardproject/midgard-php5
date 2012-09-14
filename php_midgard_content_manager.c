/* 
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
#include "php_midgard_gobject.h"
#include "php_midgard__helpers.h"
#include "php_gobject.h"

/* midgard core classes */
zend_class_entry *php_midgard_sql_content_manager_class;

/*	ContentManager	*/

static PHP_METHOD(midgard_content_manager, get_connection)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardContentManager *manager = MIDGARD_CONTENT_MANAGER(__php_gobject_ptr(getThis()));
	GError *error = NULL;
	MidgardConnection *connection = midgard_content_manager_get_connection(manager, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get connection. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	if (connection == NULL)
		RETURN_NULL();

	object_init_ex(return_value, php_midgard_connection_class);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(connection));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_content_manager_get_connection, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_content_manager, create_job)
{
	int type;
	zval *z_content = NULL;
	zval *z_reference = NULL;
	zval *z_model = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "loO!O", &type, &z_content, &z_reference, &z_model) == FAILURE)
		return;

	MidgardContentManager *manager = MIDGARD_CONTENT_MANAGER(__php_gobject_ptr(getThis()));
	GObject *gobject =  G_OBJECT(__php_gobject_ptr(z_content));
	MidgardObjectReference *reference = MIDGARD_OBJECT_REFERENCE(__php_gobject_ptr(z_content));
	MidgardModel *model = NULL;

	if (z_model)
		model = MIDGARD_MODEL(__php_gobject_ptr(z_content));

	GError *error = NULL;
	MidgardContentManagerJob *job = midgard_content_manager_create_job(manager, type, gobject, reference, model, &error);

	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to create new job. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	zend_class_entry *job_class_entry = php_midgard_get_class_ptr_by_name(G_OBJECT_TYPE_NAME(job) TSRMLS_CC);
	object_init_ex(return_value, job_class_entry);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(return_value));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_content_manager_create_job, 0, 0, 4)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, content)
	ZEND_ARG_INFO(0, reference)
	ZEND_ARG_INFO(0, model)
ZEND_END_ARG_INFO()

/*	SqlContentManager	*/

static PHP_METHOD(midgard_sql_content_manager, __construct)
{
	php_gobject_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_sql_content_manager___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

#define _REGISTER_CONSTANT(name) \
		REGISTER_LONG_CONSTANT(#name, name, CONST_CS | CONST_PERSISTENT)

PHP_MINIT_FUNCTION(midgard2_content_manager)
{
	/* Register job type constants */
	_REGISTER_CONSTANT(MIDGARD_CONTENT_MANAGER_JOB_LOAD);
	_REGISTER_CONSTANT(MIDGARD_CONTENT_MANAGER_JOB_CREATE);
	_REGISTER_CONSTANT(MIDGARD_CONTENT_MANAGER_JOB_UPDATE);
	_REGISTER_CONSTANT(MIDGARD_CONTENT_MANAGER_JOB_DELETE);
	_REGISTER_CONSTANT(MIDGARD_CONTENT_MANAGER_JOB_PURGE);

	zend_class_entry *content_manager_interface = php_midgard_get_class_ptr_by_name("MidgardContentManager" TSRMLS_CC);

	/*	ContentManager	*/
	static zend_function_entry midgard_content_manager_methods[] = {
		PHP_ME(midgard_content_manager,	get_connection,	arginfo_midgard_content_manager_get_connection,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_content_manager,	create_job,	arginfo_midgard_content_manager_create_job,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		{NULL, NULL, NULL}
	};

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	content_manager_interface->info.internal.builtin_functions = midgard_content_manager_methods;
	zend_register_functions(content_manager_interface, content_manager_interface->info.internal.builtin_functions, &content_manager_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#else
	content_manager_interface->builtin_functions = midgard_content_manager_methods;
	zend_register_functions(content_manager_interface, content_manager_interface->builtin_functions, &content_manager_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#endif

	/*	SqlContentManager	*/

	static zend_function_entry midgard_sql_content_manager_methods[] = {
		PHP_ME(midgard_sql_content_manager,	__construct,	arginfo_midgard_sql_content_manager___construct,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_content_manager,		get_connection,	arginfo_midgard_content_manager_get_connection,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_content_manager,		create_job,	arginfo_midgard_content_manager_create_job,		ZEND_ACC_PUBLIC)
	};

	static zend_class_entry php_midgard_sql_content_manager_class_entry;
	INIT_CLASS_ENTRY(php_midgard_sql_content_manager_class_entry, "MidgardSqlContentManager", midgard_sql_content_manager_methods);
	php_midgard_sql_content_manager_class = zend_register_internal_class(&php_midgard_sql_content_manager_class_entry TSRMLS_CC); 

	zend_class_implements(php_midgard_sql_content_manager_class TSRMLS_CC, 1, content_manager_interface);
	php_midgard_sql_content_manager_class->create_object = php_midgard_gobject_new;
	php_midgard_sql_content_manager_class->ce_flags = 0;
	CLASS_SET_DOC_COMMENT(php_midgard_sql_content_manager_class, strdup("ContentManager implementation for SQL storage"));

	zend_register_class_alias("midgard_sql_content_manager", php_midgard_sql_content_manager_class);

	return SUCCESS;
}
