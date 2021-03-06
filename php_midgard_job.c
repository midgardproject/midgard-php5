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
#include "php_gobject.h"
#include "php_midgard__helpers.h"

/* midgard core classes */
zend_class_entry *php_midgard_sql_content_manager_job_class;

/*	Job	*/

static PHP_METHOD(midgard_job, is_running)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardJob *job = MIDGARD_JOB(__php_gobject_ptr(getThis()));
	RETURN_BOOL(midgard_job_is_running(job));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_job_is_running, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_job, is_executed)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardJob *job = MIDGARD_JOB(__php_gobject_ptr(getThis()));
	RETURN_BOOL(midgard_job_is_executed(job));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_job_is_executed, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_job, is_failed)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardJob *job = MIDGARD_JOB(__php_gobject_ptr(getThis()));
	RETURN_BOOL(midgard_job_is_failed(job));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_job_is_failed, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	ContentManagerJob	*/

static PHP_METHOD(midgard_content_manager_job, get_content_object)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	GError *error = NULL;
	MidgardContentManagerJob *job = MIDGARD_CONTENT_MANAGER_JOB(__php_gobject_ptr(getThis()));
	GObject *object = midgard_content_manager_job_get_content_object(job, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get content object. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	php_midgard_gobject_init(return_value, G_OBJECT_TYPE_NAME(object), object, TRUE TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_content_manager_job_get_content_object, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_content_manager_job, get_reference)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	GError *error = NULL;
	MidgardContentManagerJob *job = MIDGARD_CONTENT_MANAGER_JOB(__php_gobject_ptr(getThis()));
	MidgardObjectReference *reference = midgard_content_manager_job_get_reference(job, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get reference. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	php_midgard_gobject_init(return_value, G_OBJECT_TYPE_NAME(reference), G_OBJECT(reference), TRUE TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_content_manager_job_get_reference, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_content_manager_job, get_model)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	GError *error = NULL;
	MidgardContentManagerJob *job = MIDGARD_CONTENT_MANAGER_JOB(__php_gobject_ptr(getThis()));
	MidgardModel *model = midgard_content_manager_job_get_model(job, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get model. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	if (!model)
		RETURN_NULL();

	php_midgard_gobject_init(return_value, G_OBJECT_TYPE_NAME(model), G_OBJECT(model), TRUE TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_content_manager_job_get_model, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	SqlContentManagerJob	*/

static PHP_METHOD(midgard_sql_content_manager_job, __construct)
{
	php_gobject_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_sql_content_manager_job___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_sql_content_manager_job, get_connection)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardSqlContentManagerJob *job = MIDGARD_SQL_CONTENT_MANAGER_JOB(__php_gobject_ptr(getThis()));
	GError *error = NULL;
	MidgardConnection *connection = midgard_sql_content_manager_job_get_connection(job, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"SqlContentManager - Failed to get connection. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	if (connection == NULL)
		RETURN_NULL();

	php_midgard_gobject_new_with_gobject (return_value, php_midgard_connection_class, G_OBJECT (connection), TRUE TSRMLS_CC);
	/* Add reference, it's mandatory only for current MidgardConnection bindings.
	 * Not valid per generic GObject reference */
	Z_ADDREF_P(return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_sql_content_manager_job_get_connection, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	Validable	*/
static PHP_METHOD(midgard_validable, is_valid)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardValidable *validable = MIDGARD_VALIDABLE(__php_gobject_ptr(getThis()));
	RETURN_BOOL(midgard_validable_is_valid(validable));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_validable_is_valid, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_validable, validate)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardValidable *validable = MIDGARD_VALIDABLE(__php_gobject_ptr(getThis()));
	GError *error = NULL;
	midgard_validable_validate(validable, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Validation error. %s", error && error->message ? error->message : "Unknown reason");
		g_clear_error(&error);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_validable_validate, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	Executable	*/
static PHP_METHOD(midgard_executable, execute)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardExecutable *executable = MIDGARD_EXECUTABLE(__php_gobject_ptr(getThis()));
	GError *error = NULL;
	midgard_executable_execute(executable, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Execution error. %s", error && error->message ? error->message : "Unknown reason");
		g_clear_error(&error);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_executable_execute, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_executable, execute_async)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardExecutable *executable = MIDGARD_EXECUTABLE(__php_gobject_ptr(getThis()));
	GError *error = NULL;
	midgard_executable_execute_async(executable, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Async execution error. %s", error && error->message ? error->message : "Unknown reason");
		g_clear_error(&error);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_executable_execute_async, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	Signals	*/
static PHP_METHOD(midgard_sql_content_manager_job, connect)
{
	php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_sql_content_manager_job_connect, 0, 0, 2)
	ZEND_ARG_INFO(0, signal)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, user_data)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_job)
{
	zend_class_entry *executable_interface = php_midgard_get_class_ptr_by_name("MidgardExecutable" TSRMLS_CC);
	zend_class_entry *validable_interface = php_midgard_get_class_ptr_by_name("MidgardValidable" TSRMLS_CC);
	zend_class_entry *job_interface = php_midgard_get_class_ptr_by_name("MidgardJob" TSRMLS_CC);
	zend_class_entry *content_manager_job_interface = php_midgard_get_class_ptr_by_name("MidgardContentManagerJob" TSRMLS_CC);

	/*	Job	*/
	static zend_function_entry midgard_job_methods[] = {
		PHP_ME(midgard_job,	is_running,	arginfo_midgard_job_is_running,		ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_job,	is_executed,	arginfo_midgard_job_is_executed,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_job,	is_failed,	arginfo_midgard_job_is_failed,		ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		{NULL, NULL, NULL}
	};

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	job_interface->info.internal.builtin_functions = midgard_job_methods;
	zend_register_functions(job_interface, job_interface->info.internal.builtin_functions, &job_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#else
	job_interface->builtin_functions = midgard_job_methods;
	zend_register_functions(job_interface, job_interface->builtin_functions, &job_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#endif

	/*	ContentManagerJob	*/
	static zend_function_entry midgard_content_manager_job_methods[] = {
		PHP_ME(midgard_content_manager_job,	get_content_object,	arginfo_midgard_content_manager_job_get_content_object,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_content_manager_job,	get_reference,		arginfo_midgard_content_manager_job_get_reference,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_content_manager_job,	get_model,		arginfo_midgard_content_manager_job_get_model,		ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		{NULL, NULL, NULL}
	};

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	content_manager_job_interface->info.internal.builtin_functions = midgard_content_manager_job_methods;
	zend_register_functions(content_manager_job_interface, content_manager_job_interface->info.internal.builtin_functions, &content_manager_job_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#else
	content_manager_job_interface->builtin_functions = midgard_content_manager_job_methods;
	zend_register_functions(content_manager_job_interface, content_manager_job_interface->builtin_functions, &content_manager_job_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#endif

	/*	SqlContentManagerJob	*/
	static zend_function_entry midgard_sql_content_manager_job_methods[] = {
		PHP_ME(midgard_sql_content_manager_job,	__construct,	arginfo_midgard_sql_content_manager_job___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_sql_content_manager_job,	get_connection,	arginfo_midgard_sql_content_manager_job_get_connection,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_sql_content_manager_job,	connect,	arginfo_midgard_sql_content_manager_job_connect,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_job,	is_running,	arginfo_midgard_job_is_running,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_job,	is_executed,	arginfo_midgard_job_is_executed,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_job,	is_failed,	arginfo_midgard_job_is_failed,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_content_manager_job,	get_content_object,	arginfo_midgard_content_manager_job_get_content_object,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_content_manager_job,	get_reference,		arginfo_midgard_content_manager_job_get_reference,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_content_manager_job,	get_model,		arginfo_midgard_content_manager_job_get_model,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_validable,		is_valid,		arginfo_midgard_validable_is_valid,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_validable,		validate,		arginfo_midgard_validable_validate,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_executable,		execute,		arginfo_midgard_executable_execute,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_executable,		execute_async,		arginfo_midgard_executable_execute_async,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_sql_content_manager_job_class_entry;
	INIT_CLASS_ENTRY(php_midgard_sql_content_manager_job_class_entry, "MidgardSqlContentManagerJob", midgard_sql_content_manager_job_methods);

	php_midgard_sql_content_manager_job_class = zend_register_internal_class(&php_midgard_sql_content_manager_job_class_entry TSRMLS_CC);
	zend_class_implements(php_midgard_sql_content_manager_job_class TSRMLS_CC, 4, executable_interface, validable_interface, job_interface, content_manager_job_interface);
	php_midgard_sql_content_manager_job_class->create_object = php_midgard_gobject_new;
	php_midgard_sql_content_manager_job_class->ce_flags = 0;
	CLASS_SET_DOC_COMMENT(php_midgard_sql_content_manager_job_class, strdup("ContentManagerJob implementation for SQL storage"));

	_FIXME_zend_register_class_alias("midgard_sql_content_manager_job", php_midgard_sql_content_manager_job_class);

	/*	SqlContentManagerJobLoad	*/
	static zend_class_entry job_load_class_entry;
	INIT_CLASS_ENTRY(job_load_class_entry, "MidgardSqlContentManagerJobLoad", NULL);
	zend_class_entry *job_load_class = zend_register_internal_class_ex(&job_load_class_entry, 
			php_midgard_sql_content_manager_job_class, "MidgardSqlContentManagerJob" TSRMLS_CC); 
	job_load_class->create_object = php_midgard_gobject_new;
	_FIXME_zend_register_class_alias("midgard_sql_content_manager_job_load", job_load_class);

	/*	SqlContentManagerJobCreate	*/
	static zend_class_entry job_create_class_entry;
	INIT_CLASS_ENTRY(job_create_class_entry, "MidgardSqlContentManagerJobCreate", NULL);
	zend_class_entry *job_create_class = zend_register_internal_class_ex(&job_create_class_entry, 
			php_midgard_sql_content_manager_job_class, "MidgardSqlContentManagerJob" TSRMLS_CC); 
	job_create_class->create_object = php_midgard_gobject_new;
	_FIXME_zend_register_class_alias("midgard_sql_content_manager_job_create", job_create_class);

	/*	SqlContentManagerJobUpdate	*/
	static zend_class_entry job_update_class_entry;
	INIT_CLASS_ENTRY(job_update_class_entry, "MidgardSqlContentManagerJobUpdate", NULL);
	zend_class_entry *job_update_class = zend_register_internal_class_ex(&job_update_class_entry, 
			php_midgard_sql_content_manager_job_class, "MidgardSqlContentManagerJob" TSRMLS_CC); 
	job_create_class->create_object = php_midgard_gobject_new;
	_FIXME_zend_register_class_alias("midgard_sql_content_manager_job_update", job_update_class);

	/*	SqlContentManagerJobDelete	*/
	static zend_class_entry job_delete_class_entry;
	INIT_CLASS_ENTRY(job_delete_class_entry, "MidgardSqlContentManagerJobDelete", NULL);
	zend_class_entry *job_delete_class = zend_register_internal_class_ex(&job_delete_class_entry, 
			php_midgard_sql_content_manager_job_class, "MidgardSqlContentManagerJob" TSRMLS_CC); 
	job_delete_class->create_object = php_midgard_gobject_new;
	_FIXME_zend_register_class_alias("midgard_sql_content_manager_job_update", job_delete_class);

	/*	SqlContentManagerJobPurge	*/
	static zend_class_entry job_purge_class_entry;
	INIT_CLASS_ENTRY(job_purge_class_entry, "MidgardSqlContentManagerJobPurge", NULL);
	zend_class_entry *job_purge_class = zend_register_internal_class_ex(&job_purge_class_entry, 
			php_midgard_sql_content_manager_job_class, "MidgardSqlContentManagerJob" TSRMLS_CC); 
	job_purge_class->create_object = php_midgard_gobject_new;
	_FIXME_zend_register_class_alias("midgard_sql_content_manager_job_purge", job_purge_class);

	return SUCCESS;
}
