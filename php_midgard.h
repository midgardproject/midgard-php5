/*
 * Copyright (C) 2006, 2008, 2009 Piotr Pokora <piotrek.pokora@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of\
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef G_LOG_DOMAIN
# undef G_LOG_DOMAIN
#endif
#define G_LOG_DOMAIN "midgard-core"

#ifndef PHP_MIDGARD_H
#define PHP_MIDGARD_H

#include "config.h"

#ifdef PHP_WIN32
# include "config.w32.h"
#endif

#ifndef PHP_CONFIG_H
# define PHP_CONFIG_H
# include "php_config.h"
#endif

/* Do not load Apache regex when php compiled with 'system' as regex's TYPE */
#if REGEX == 0
# ifndef _REGEX_H
#  define _REGEX_H 1
# endif
#endif

#include "php.h"
#include "php_globals.h"
#include <zend_interfaces.h>
#include <Zend/zend_exceptions.h>
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "SAPI.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/info.h"

#include <locale.h>

#if HAVE_MIDGARD

#define PHP_MIDGARD2_EXTNAME MIDGARD_PACKAGE_NAME
#define PHP_MIDGARD2_EXTVER MIDGARD_LIB_VERSION
#define PHP_MIDGARD2_WRAPPER "midgard"
#define PHP_MIDGARD2_STREAMTYPE "midgard2stream"

#define MGD_FUNCTION(ret, name, param) \
	PHP_FUNCTION(mgd_##name)

#define MGD_FE(name, arg_types) \
	PHP_FE(mgd_##name, arg_types)

# include <midgard/midgard.h>

# ifdef ZTS
#  include <TSRM.h>
# endif

typedef struct _MgdGHolder MgdGHolder;
typedef struct _php_midgard2stream_data {
	MidgardObject *obj;
	char *buffer;
	int size;
	off_t position;
} php_midgard2stream_data;


extern guint global_loghandler;
extern gboolean php_midgard_log_enabled;

MidgardConnection *mgd_handle(TSRMLS_D);
/* FIXME, is_connected is confusing because it says about connection instance pointer only */
#define php_midgard_is_connected() (MGDG(connection_established) == TRUE)

# define phpext_midgard_ptr &midgard2_module_entry

# ifdef PHP_WIN32
#  define PHP_MIDGARD_API __declspec(dllexport)
# else
#  define PHP_MIDGARD_API
# endif

PHP_MINIT_FUNCTION(midgard2);
PHP_MSHUTDOWN_FUNCTION(midgard2);
PHP_RINIT_FUNCTION(midgard2);
PHP_RSHUTDOWN_FUNCTION(midgard2);
PHP_MINFO_FUNCTION(midgard2);

ZEND_BEGIN_MODULE_GLOBALS(midgard2)
	zend_bool connection_established;
	GHashTable *all_configs;
	MgdGHolder *midgard_global_holder;
	zend_bool can_deliver_signals;
	char *midgard_configuration;
	char *midgard_configuration_file;
	zend_bool midgard_engine;
	zend_bool midgard_http;
	zend_bool midgard_memory_debug;
	zend_bool superglobals_compat;
	zend_bool valgrind_friendly;
	zend_bool glib_loghandler;
ZEND_END_MODULE_GLOBALS(midgard2)

ZEND_EXTERN_MODULE_GLOBALS(midgard2)

# ifdef ZTS
#  define MGDG(v) TSRMG(midgard2_globals_id, zend_midgard2_globals *, v)
# else
#  define MGDG(v) (midgard2_globals.v)
# endif

#else

# define phpext_midgard2_ptr NULL

#endif

PHP_FUNCTION(_midgard_php_object_constructor);
/* FIXME , change name */
PHP_FUNCTION(_get_type_by_id);
PHP_FUNCTION(_midgard_php_object_get_by_guid);
PHP_FUNCTION(_midgard_php_object_update);
PHP_FUNCTION(_midgard_php_object_create);
PHP_FUNCTION(_midgard_php_object_is_in_parent_tree);
PHP_FUNCTION(_midgard_php_object_is_in_tree);
PHP_FUNCTION(_midgard_php_object_get_tree);
PHP_FUNCTION(_midgard_php_object_delete);
PHP_FUNCTION(_midgard_php_object_get_parent);
PHP_FUNCTION(_midgard_php_object_get);
PHP_FUNCTION(_midgard_php_object_list);
PHP_FUNCTION(_midgard_php_object_list_children);
PHP_FUNCTION(_midgard_php_object_get_by_path);
PHP_FUNCTION(_midgard_php_object_parent);
PHP_FUNCTION(_php_midgard_object_get_languages);
PHP_FUNCTION(_php_midgard_object_purge);
PHP_FUNCTION(_php_midgard_object_undelete);
PHP_FUNCTION(_php_midgard_object_parameter);
PHP_FUNCTION(_php_midgard_object_connect);

/* Underlying GObject bindings */
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type, const zend_literal *key TSRMLS_DC);
zval **php_midgard_gobject_get_property_ptr_ptr(zval *object, zval *member, const zend_literal *key TSRMLS_DC);
void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value, const zend_literal *key TSRMLS_DC);
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type, const zend_literal *key TSRMLS_DC);
#else
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type TSRMLS_DC);
zval **php_midgard_gobject_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC);
void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value TSRMLS_DC);
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type TSRMLS_DC);
#endif

HashTable *php_midgard_zendobject_get_properties (zval *zobject TSRMLS_DC);

void php_midgard_array_from_objects(GObject **objects, const gchar *class_name, zval *zarray TSRMLS_DC);

GValue *php_midgard_zval2gvalue(const zval *zvalue TSRMLS_DC);
zend_bool php_midgard_gvalue2zval(const GValue *gvalue, zval *zvalue TSRMLS_DC);

GParameter *php_midgard_array_to_gparameter(zval *params, guint *n_params TSRMLS_DC);

gboolean php_midgard_is_derived_from_class(const gchar *classname,
		GType basetype, gboolean check_parent, zend_class_entry **base_class TSRMLS_DC);

#define PHP_MIDGARD_PARSE_CLASS_ARGUMENT(__name, __type, __check_parent, __base_ce) { \
	gboolean __isderived = \
		php_midgard_is_derived_from_class(__name, __type, __check_parent, __base_ce TSRMLS_CC); \
	if (!__isderived) { \
		php_error(E_WARNING, "Expected %s derived class", g_type_name(__type)); \
		php_midgard_error_exception_force_throw(mgd_handle(TSRMLS_C), MGD_ERR_INVALID_OBJECT); \
		return; \
	} \
}

/* DBObject routines and hooks */
int php_midgard_serialize_dbobject_hook(zval *zobject, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC);
int php_midgard_unserialize_dbobject_hook(zval **zobject, zend_class_entry *ce, const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC);

/* closures */
void php_midgard_object_connect_class_closures(GObject *object, zval *zobject TSRMLS_DC);
void php_midgard_gobject_closure_hash_new();
void php_midgard_gobject_closure_hash_reset();
void php_midgard_gobject_closure_hash_free();

/* Midgard core's classes */
PHP_MINIT_FUNCTION(midgard2_urlwrapper);
PHP_MINIT_FUNCTION(midgard2_collector);
PHP_MINIT_FUNCTION(midgard2_config);
PHP_MINIT_FUNCTION(midgard2_user);
PHP_MINIT_FUNCTION(midgard2_blob);
PHP_MINIT_FUNCTION(midgard2_query_builder);
PHP_MINIT_FUNCTION(midgard2_object_class);
PHP_MINIT_FUNCTION(midgard2_reflection_property);
PHP_MINIT_FUNCTION(midgard2_connection);
PHP_MINIT_FUNCTION(midgard2_object);
PHP_MINIT_FUNCTION(midgard2_dbus);
PHP_MINIT_FUNCTION(midgard2_replicator);
PHP_MINIT_FUNCTION(midgard2_datetime);
PHP_MINIT_FUNCTION(midgard2_error);
PHP_MINIT_FUNCTION(midgard2_transaction);
PHP_MINIT_FUNCTION(midgard2_view);
PHP_MINIT_FUNCTION(midgard2_storage);
PHP_MINIT_FUNCTION(midgard2_key_config_context);
PHP_MINIT_FUNCTION(midgard2_key_config_file_context);
PHP_MINIT_FUNCTION(midgard2_key_config);
PHP_MINIT_FUNCTION(midgard2_key_config_file);
PHP_MINIT_FUNCTION(midgard2_reflection_workaround);
PHP_MINIT_FUNCTION(midgard2_query);
PHP_MINIT_FUNCTION(midgard2_g_mainloop);
PHP_MINIT_FUNCTION(midgard2_workspaces);
PHP_MINIT_FUNCTION(midgard2_base_abstract);
PHP_MINIT_FUNCTION(midgard2_base_interface);
PHP_MINIT_FUNCTION(midgard2_reflector_object);
PHP_MINIT_FUNCTION(midgard2_reflector_property);
PHP_MINIT_FUNCTION(midgard2_repligard);
PHP_MINIT_FUNCTION(midgard2_query_selectors);

zend_class_entry *php_midgard_get_baseclass_ptr(zend_class_entry *ce);
zend_class_entry *php_midgard_get_baseclass_ptr_by_name(const char *name TSRMLS_DC);
zend_class_entry *php_midgard_get_class_ptr_by_name(const char *name TSRMLS_DC);

/* Exceptions */
gboolean php_midgard_error_exception_throw(MidgardConnection *mgd TSRMLS_DC);
void php_midgard_error_exception_force_throw(MidgardConnection *mgd, gint errcode TSRMLS_DC);

/* Logging */
void php_midgard_log_errors(const gchar *domain, GLogLevelFlags level, const gchar *msg, gpointer userdata);

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
#define CHECK_MGD(handle) \
{ \
	if (!handle) { \
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Failed to get connection"); \
		return; \
	} \
	const gchar *_check_cname_space = ""; \
	const gchar *_check_class_name = get_active_class_name(&_check_cname_space TSRMLS_CC); \
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, " %s%s%s(...)", \
		_check_class_name, _check_cname_space, get_active_function_name(TSRMLS_C)); \
}
#else
#define CHECK_MGD(handle) \
{ \
	if (!handle) { \
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Failed to get connection"); \
		return; \
	} \
	gchar *_check_cname_space = NULL; \
	gchar *_check_class_name = get_active_class_name(&_check_cname_space TSRMLS_CC); \
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, " %s%s%s(...)", \
		_check_class_name, _check_cname_space, get_active_function_name(TSRMLS_C)); \
}
#endif

/* RAGNAROEK COMPATIBLE*/

extern zend_class_entry *php_midgard_connection_class;
extern zend_class_entry *php_midgard_config_class;
extern zend_class_entry *php_midgard_collector_class;
extern zend_class_entry *php_midgard_query_builder_class;
extern zend_class_entry *php_midgard_reflection_property_class;
extern zend_class_entry *php_midgard_user_class;
extern zend_class_entry *ce_midgard_error_exception;
extern zend_class_entry *php_midgard_datetime_class;
extern zend_class_entry *php_midgard_workspace_storage_class;
extern zend_class_entry *php_midgard_workspace_class;
extern zend_class_entry *php_midgard_reflector_object_class;
extern zend_class_entry *php_midgard_reflector_property_class;
extern zend_class_entry *php_midgard_query_executor_class;

#define __php_objstore_object(instance) ((php_midgard_gobject *)zend_object_store_get_object(instance TSRMLS_CC))
#define __php_gobject_ptr(instance) (__php_objstore_object(instance)->gobject)
#define __midgard_connection_get_ptr(instance) MIDGARD_CONNECTION(__php_gobject_ptr(instance))
#define __midgard_object_get_ptr(instance)     MIDGARD_OBJECT(__php_gobject_ptr(instance))
#define __midgard_dbobject_get_ptr(instance)   MIDGARD_DBOBJECT(__php_gobject_ptr(instance))

#define MGD_PHP_SET_GOBJECT_G(instance, object) (__php_gobject_ptr(instance) = object)
#define MGD_PHP_SET_GOBJECT(instance, object)  MGD_PHP_SET_GOBJECT_G(instance, G_OBJECT(object))

#endif	/* PHP_MIDGARD_H */
