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
zend_class_entry *php_midgard_execution_pool_class;

/*	Pool	*/

static PHP_METHOD(midgard_pool, get_max_n_resources)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardPool *pool = MIDGARD_POOL(__php_gobject_ptr(getThis()));
	RETURN_LONG(midgard_pool_get_max_n_resources(pool));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_pool_get_max_n_resources, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_pool, get_n_resources)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardPool *pool = MIDGARD_POOL(__php_gobject_ptr(getThis()));
	RETURN_LONG(midgard_pool_get_n_resources(pool));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_pool_get_n_resources, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_pool, set_max_n_resources)
{
	long resources = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &resources) == FAILURE) {
		return;
	}

	GError *error = NULL;
	MidgardPool *pool = MIDGARD_POOL(__php_gobject_ptr(getThis()));
	midgard_pool_set_max_n_resources(pool, resources, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to set resources max. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_pool_set_max_n_resources, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_pool, push)
{
	zval *z_object;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &z_object) == FAILURE) {
		return;
	}

	GError *error = NULL;
	MidgardPool *pool = MIDGARD_POOL(__php_gobject_ptr(getThis()));
	GObject *gobject = G_OBJECT(__php_gobject_ptr(z_object));
	midgard_pool_push(pool, gobject, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed add object to pool. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_pool_push, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	ExecutionPool	*/

static PHP_METHOD(midgard_execution_pool, __construct)
{
	php_gobject_constructor(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_execution_pool___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_pool)
{
	zend_class_entry *pool_interface = php_midgard_get_class_ptr_by_name("MidgardPool" TSRMLS_CC);

	/*	Job	*/
	static zend_function_entry midgard_pool_methods[] = {
		PHP_ME(midgard_pool,	get_max_n_resources,	arginfo_midgard_pool_get_max_n_resources,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_pool,	set_max_n_resources,	arginfo_midgard_pool_set_max_n_resources,	ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_pool,	get_n_resources,	arginfo_midgard_pool_get_n_resources,		ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		PHP_ME(midgard_pool,	push,			arginfo_midgard_pool_push,			ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
		{NULL, NULL, NULL}
	};

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	pool_interface->info.internal.builtin_functions = midgard_pool_methods;
	zend_register_functions(pool_interface, pool_interface->info.internal.builtin_functions, &pool_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#else
	pool_interface->builtin_functions = midgard_pool_methods;
	zend_register_functions(pool_interface, pool_interface->builtin_functions, &pool_interface->function_table, MODULE_PERSISTENT TSRMLS_CC);
#endif

	/*	ExecutionPool	*/
	static zend_function_entry midgard_execution_pool_methods[] = {
		PHP_ME(midgard_execution_pool,	__construct,	arginfo_midgard_execution_pool___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_pool,	get_max_n_resources,	arginfo_midgard_pool_get_max_n_resources,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_pool,	set_max_n_resources,	arginfo_midgard_pool_set_max_n_resources,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_pool,	get_n_resources,	arginfo_midgard_pool_get_n_resources,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_pool,	push,			arginfo_midgard_pool_push,			ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_execution_pool_class_entry;
	INIT_CLASS_ENTRY(php_midgard_execution_pool_class_entry, "MidgardExecutionPool", midgard_execution_pool_methods);
	php_midgard_execution_pool_class = zend_register_internal_class(&php_midgard_execution_pool_class_entry TSRMLS_CC);

	zend_class_implements(php_midgard_execution_pool_class TSRMLS_CC, 1, pool_interface);
	php_midgard_execution_pool_class->create_object = php_midgard_gobject_new;
	php_midgard_execution_pool_class->ce_flags = 0;
	CLASS_SET_DOC_COMMENT(php_midgard_execution_pool_class, strdup("Pool implementation for executions"));

	_FIXME_zend_register_class_alias("midgard_execution_pool", php_midgard_execution_pool_class);

	return SUCCESS;
}
