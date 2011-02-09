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
#include "php_midgard_handle.h"

#include "php_midgard__helpers.h"

#include <zend_interfaces.h>

zend_class_entry *php_midgard_connection_class;

#define PHP_MGD_HASH_UPDATE(k, v) zend_hash_update(Z_ARRVAL_PP(_midgard), k, sizeof(k), v, sizeof(zval *), NULL)

/* Object constructor */
PHP_METHOD(midgard_connection, __construct)
{
	MidgardConnection *mgd = NULL;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] midgard_connection::__construct()\n", getThis());
	}

	if (zend_parse_parameters_none() == FAILURE)
		return;

	if (MGDG(midgard_http)) {
		/* trying to reuse saved connection */
		mgd = php_midgard_handle_lookup(&MGDG(midgard_global_holder), MGDG(all_configs) TSRMLS_CC);

		if (mgd == NULL) {
			/* @todo throw exception, instead */
			php_error(E_ERROR, "Midgard handle-lookup failed (could not connect to database)");
			return;
		}
	} else {
		mgd = midgard_connection_new();
		if (mgd == NULL) {
			/* @todo throw exception, instead */
			php_error(E_ERROR, "Failed to create underlying GObject instance");
			return;
		}
	}

	if (global_loghandler) {
		if (MGDG(midgard_memory_debug)) {
			php_printf("---> g_log_remove_handler(..., %d)\n", global_loghandler);
		}
		g_log_remove_handler(G_LOG_DOMAIN, global_loghandler);
	}

	// midgard_connection_set_loglevel(mgd, "warning", NULL);
	global_loghandler = midgard_connection_get_loghandler(mgd);
	if (MGDG(midgard_memory_debug)) {
		php_printf("---> global_loghandler = %d\n", global_loghandler);
	}

	/* storing midgard_connection in object's store-structure */
	MGD_PHP_SET_GOBJECT(getThis(), mgd);

	// explicitly enable replication (to stay compatible with mjölnir)
	midgard_connection_enable_replication(mgd, TRUE);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= midgard_connection::__construct()\n", getThis());
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection___construct, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(midgard_connection, __destruct)
{
	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] midgard_connection::__destruct()\n", getThis());
	}

	MidgardConnection *mgd = __midgard_connection_get_ptr(getThis());
	int loghandler = midgard_connection_get_loghandler(mgd);

	if (loghandler) {
		if (MGDG(midgard_memory_debug)) {
			php_printf("[%p] ---> g_log_remove_handler(..., %d)\n", getThis(), loghandler);
		}

		g_log_remove_handler(G_LOG_DOMAIN, loghandler);

		if (global_loghandler != loghandler && MGDG(midgard_memory_debug)) {
			php_printf("[%p] ---> (?) global_loghandler != connection's handler\n", getThis());
		}

		// still just null it, as it is not valid anyway
		global_loghandler = 0;
		midgard_connection_set_loghandler (mgd, 0);
	}

	MGDG(connection_established) = FALSE;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= midgard_connection::__destruct()\n", getThis());
	}
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection___destruct, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(midgard_connection, get_instance)
{
	zval *instance;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (MGDG(midgard_memory_debug)) {
		php_printf(":: midgard_connection::get_isntance()\n");
	}

	instance = zend_read_static_property(php_midgard_connection_class, "instance", sizeof("instance")-1, 0 TSRMLS_CC);

	if (MGDG(midgard_memory_debug)) {
		php_printf(":: ==> instance=%p\n", instance);
	}

	if (ZVAL_IS_NULL(instance)) {
		object_init_ex(instance, php_midgard_connection_class);
		zend_call_method_with_0_params(&instance, php_midgard_connection_class, &php_midgard_connection_class->constructor, "__construct", NULL);

		MGDG(connection_established) = TRUE;
	}

	Z_ADDREF_P(instance);

	if (MGDG(midgard_memory_debug)) {
		php_printf(":: ==> refcount=%d\n", Z_REFCOUNT_P(instance));
	}

	zval_ptr_dtor(return_value_ptr);
	*return_value_ptr = instance;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_get_instance, 0, 1, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(midgard_connection, copy)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd = __midgard_connection_get_ptr(getThis());
	MidgardConnection *copy = midgard_connection_copy(mgd);

	guint loghandler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, php_midgard_log_errors, (gpointer)copy);
	midgard_connection_set_loghandler(copy, loghandler);

	MGD_PHP_SET_GOBJECT(getThis(), copy);

	RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_copy, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, open)
{
	RETVAL_FALSE;
	char *cnf_name;
	int cnf_name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &cnf_name, &cnf_name_length) == FAILURE)
		return;

	MidgardConnection *mgd = __midgard_connection_get_ptr(getThis());
	gboolean rv = midgard_connection_open(mgd, (const gchar *)cnf_name, NULL);

	if (rv) {
		guint loghandler = midgard_connection_get_loghandler(mgd);

		if (loghandler)
			g_log_remove_handler(G_LOG_DOMAIN, loghandler);

		global_loghandler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, php_midgard_log_errors, (gpointer)mgd);
		midgard_connection_set_loghandler(mgd, global_loghandler);

		if (MGDG(midgard_memory_debug)) {
			php_printf("---> global_loghandler = %d\n", global_loghandler);
		}
	}

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_open, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, reopen)
{
	RETVAL_FALSE;
	zend_bool rv;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd = __midgard_connection_get_ptr(getThis());
	rv = (zend_bool) midgard_connection_reopen(mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_reopen, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, open_config)
{
	RETVAL_FALSE;
	zval *cnf_object;
	zend_bool rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &cnf_object, php_midgard_config_class) == FAILURE)
		return;

	MidgardConfig *config = MIDGARD_CONFIG(__php_gobject_ptr(cnf_object));

	MidgardConnection *mgd = __midgard_connection_get_ptr(getThis());
	rv = (zend_bool) midgard_connection_open_config(mgd, config);

	// if (rv) {
	// 	Z_ADDREF_P(cnf_object);
	// }

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_open_config, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, config, midgard_config, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, is_connected)
{
	RETVAL_FALSE;
	zend_bool rv;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd = __midgard_connection_get_ptr (getThis());
	rv = (zend_bool) midgard_connection_is_connected (mgd);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_is_connected, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_error)
{
	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	long err_code = midgard_connection_get_error(mgd);
	RETURN_LONG(err_code);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_error, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_error)
{
	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	CHECK_MGD(mgd);
	long errcode;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &errcode) == FAILURE)
		return;

	if (errcode > 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "errcode must be negative");
		return;
	}

	midgard_connection_set_error(mgd, errcode);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_error, 0, 0, 1)
	ZEND_ARG_INFO(0, errorcode)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_error_string)
{
	RETVAL_NULL();
	/* Disable debug log for function call.
	 * It resets error. Keep it like this for backward compatibility */
	/* CHECK_MGD; */

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	const char *err_string = midgard_connection_get_error_string(mgd);

	RETURN_STRING((gchar *)err_string, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_error_string, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, get_user)
{
	RETVAL_NULL();
	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardUser *user = midgard_connection_get_user(mgd);

	if (user == NULL)
		RETURN_NULL();

	g_object_ref(user); // this is a direct-pointer: we need to "ref" it explicitly

	php_midgard_gobject_new_with_gobject(return_value, php_midgard_user_class, G_OBJECT(user), TRUE TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_get_user, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, set_loglevel)
{
	RETVAL_NULL();
	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	CHECK_MGD(mgd);

	char *level;
	int level_length;
	zval *callback;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &level, &level_length, &callback) == FAILURE)
		return;

	/* no support for callback atm */
	gboolean rv = midgard_connection_set_loglevel(mgd, (gchar *)level, php_midgard_log_errors);
	global_loghandler = midgard_connection_get_loghandler(mgd);

	if (MGDG(midgard_memory_debug)) {
		php_printf("---> global_loghandler = %d\n", global_loghandler);
	}

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_set_loglevel, 0, 0, 1)
	ZEND_ARG_INFO(0, level)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_connection, connect)
{
	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	CHECK_MGD(mgd);

	php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_connection_connect, 0, 0, 2)
	ZEND_ARG_INFO(0, signal)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, userdata)
ZEND_END_ARG_INFO()

static PHP_METHOD (midgard_connection, list_auth_types)
{
	MidgardConnection *mgd =__midgard_connection_get_ptr(getThis());
	CHECK_MGD(mgd);
	array_init (return_value);

	if (zend_parse_parameters_none () == FAILURE)
		return;

	guint n_types;
	gchar **auth_types = midgard_connection_list_auth_types(mgd, &n_types);

	if (!auth_types)
		return;

	guint i;

	for (i = 0; i < n_types; i++) {
		add_next_index_string(return_value, auth_types[i], 1);
	}

	g_free(auth_types);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_connection_list_auth_types, 0)
ZEND_END_ARG_INFO()

int __serialize_cnc_hook(zval *zobject, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	php_error(E_WARNING, "Unable to serialize midgard_connection object");
	return FAILURE;
}

int __unserialize_cnc_hook(zval **zobject, zend_class_entry *ce, const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	php_error(E_WARNING, "Unable to unserialize midgard_connection object");
	return FAILURE;
}


PHP_MINIT_FUNCTION(midgard2_connection)
{
	static function_entry connection_methods[] = {
		PHP_ME(midgard_connection, __construct,      arginfo_midgard_connection___construct,      ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
		PHP_ME(midgard_connection, __destruct,       arginfo_midgard_connection___destruct,       ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
		PHP_ME(midgard_connection, get_instance,     arginfo_midgard_connection_get_instance,     ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_connection, copy,             arginfo_midgard_connection_copy,             ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, open,             arginfo_midgard_connection_open,             ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, reopen,           arginfo_midgard_connection_reopen,           ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, open_config,      arginfo_midgard_connection_open_config,      ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, is_connected,     arginfo_midgard_connection_is_connected,     ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, connect,          arginfo_midgard_connection_connect,          ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, get_error,        arginfo_midgard_connection_get_error,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, set_error,        arginfo_midgard_connection_set_error,        ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, get_error_string, arginfo_midgard_connection_get_error_string, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, get_user,         arginfo_midgard_connection_get_user,         ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, set_loglevel,     arginfo_midgard_connection_set_loglevel,     ZEND_ACC_PUBLIC)
		PHP_ME(midgard_connection, list_auth_types,	 arginfo_midgard_connection_list_auth_types,  ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_connection_class_entry;

	INIT_CLASS_ENTRY(php_midgard_connection_class_entry, "midgard_connection", connection_methods);
	php_midgard_connection_class = zend_register_internal_class(&php_midgard_connection_class_entry TSRMLS_CC);

	php_midgard_connection_class->create_object = php_midgard_gobject_new;
	php_midgard_connection_class->serialize = __serialize_cnc_hook;
	php_midgard_connection_class->unserialize = __unserialize_cnc_hook;
	php_midgard_connection_class->doc_comment = strdup("midgard_connection class represents connection to underlying data-source and is responsible for holding and setting environmental variables (like error, authenticated user, debug level, etc.)");

	zend_declare_property_null(php_midgard_connection_class, "instance", sizeof("instance")-1, ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}
