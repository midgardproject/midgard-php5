/* $Id$
   Copyright (C) 2000 The Midgard Project ry
   Copyright (C) 2005, 2006, 2007, 2008 Piotr Pokora, <piotrek.pokora@gmail.com>

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <sys/file.h>

#include "php_midgard.h"
#include "php_ini.h"

#include "php_midgard_timestamp.h"

#include "php_midgard_gobject.h"
#include "php_midgard_handle.h"

#include "php_midgard__helpers.h"

#include <zend_exceptions.h>
#include <zend_extensions.h>
#include <zend_ini.h>
#include <date/php_date.h>

#include <locale.h>
#include <SAPI.h>

PHPAPI ZEND_DECLARE_MODULE_GLOBALS(midgard2);

/* True global resources - no need for thread safety here */
static MidgardSchema *midgard_global_schema = NULL;
static zend_class_entry *midgard_metadata_class;
zend_class_entry *ce_midgard_error_exception;
guint global_loghandler = 0;
zend_class_entry *zend_datetime_class_ptr = NULL;
zend_class_entry *zend_date_timezone_class_ptr = NULL;
gboolean php_midgard_log_enabled = TRUE;
zend_object_handlers php_midgard_gobject_handlers;

GHashTable *mgdg_config_names = NULL;
GHashTable *mgdg_config_files = NULL;
/* End of true globals */

/* Every user visible function must have an entry in midgard_functions[].
 */
#include "php_midgard_functions.h"
function_entry midgard2_functions[] = {
	PHP_FE(mgd_version, NULL)
	PHP_FE(mgd_is_guid, NULL)
	/* Undocumented */
	{NULL, NULL, NULL}  /* Must be the last line in midgard2_functions[] */
};

void php_midgard_error_exception_force_throw(MidgardConnection *mgd, gint errcode)
{
	midgard_connection_set_error(mgd, errcode);
	php_midgard_error_exception_throw(mgd);

	return;
}

gboolean php_midgard_error_throw_exception(MidgardConnection *mgd)
{
	TSRMLS_FETCH();
	if (mgd->errnum != MGD_ERR_OK) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, mgd->errstr);
		return TRUE;
	}

	return FALSE;
}

/* TODO , Check how msg memory (re)allocation works if msg is passed
 * to php_error already */
void php_midgard_log_errors(const gchar *domain, GLogLevelFlags level, const gchar *msg, gpointer userdata)
{
	MidgardConnection *mgd = (MidgardConnection*) userdata;
	guint mlevel = G_LOG_LEVEL_WARNING;

	if (mgd != NULL && MIDGARD_IS_CONNECTION(mgd)) {
		mlevel = midgard_connection_get_loglevel(mgd);
	}

	g_assert(msg != NULL);

	if (mlevel >= level && mgd != NULL) {
		midgard_error_default_log(domain, level, msg, MIDGARD_IS_CONNECTION(mgd) ? mgd : NULL);
	}

	if (!php_midgard_log_enabled)
		return;

	switch (level) {
	case G_LOG_FLAG_RECURSION:
		// level_ad =  "RECURSION";
		break;

	case G_LOG_FLAG_FATAL:
		// level_ad = "FATAL! ";
		break;

	case G_LOG_LEVEL_ERROR:
		// level_ad =  "ERROR";
		php_error(E_ERROR, "GLib: %s", msg);
		return;
		break;

	case G_LOG_LEVEL_CRITICAL:
		// level_ad = "CRITICAL ";
		php_error(E_WARNING, "GLib: %s", msg);
		return;
		break;

	case G_LOG_LEVEL_WARNING:
		// level_ad =  "WARNING";
		php_error(E_WARNING, "GLib: %s", msg);
		return;
		break;

	case G_LOG_LEVEL_MESSAGE:
		// level_ad = "MESSAGE";
		php_error(E_NOTICE, "GLib: %s", msg);
		return;
		break;

	case G_LOG_LEVEL_INFO:
		// level_ad = "info";
		break;

	case G_LOG_LEVEL_DEBUG:
		// level_ad = "DEBUG";
		break;

	default:
		// level_ad =  "Unknown level";
		break;
	}
}

/* pre-declaring */
static PHP_GINIT_FUNCTION(midgard2);

static zend_module_dep midgard2_deps[] = {
	ZEND_MOD_REQUIRED("date")
	{NULL, NULL, NULL}
};

zend_module_entry midgard2_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	midgard2_deps,
	PHP_MIDGARD2_EXTNAME,
	midgard2_functions,
	PHP_MINIT(midgard2),
	PHP_MSHUTDOWN(midgard2),
	PHP_RINIT(midgard2),
	PHP_RSHUTDOWN(midgard2),
	PHP_MINFO(midgard2),
	PHP_MIDGARD2_EXTVER, /* extension version number (string) */
	PHP_MODULE_GLOBALS(midgard2),
	PHP_GINIT(midgard2),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("midgard.configuration",         "",  PHP_INI_SYSTEM, OnUpdateString, midgard_configuration,      zend_midgard2_globals, midgard2_globals)
STD_PHP_INI_ENTRY("midgard.configuration_file",    "",  PHP_INI_SYSTEM, OnUpdateString, midgard_configuration_file, zend_midgard2_globals, midgard2_globals)
STD_PHP_INI_BOOLEAN("midgard.http",                "0", PHP_INI_SYSTEM, OnUpdateBool,   midgard_http,               zend_midgard2_globals, midgard2_globals)
STD_PHP_INI_BOOLEAN("midgard.engine",              "1", PHP_INI_ALL,    OnUpdateBool,   midgard_engine,             zend_midgard2_globals, midgard2_globals)
STD_PHP_INI_BOOLEAN("midgard.memory_debug",        "0", PHP_INI_ALL,    OnUpdateBool,   midgard_memory_debug,       zend_midgard2_globals, midgard2_globals)
STD_PHP_INI_BOOLEAN("midgard.superglobals_compat", "0", PHP_INI_SYSTEM, OnUpdateBool,   superglobals_compat,        zend_midgard2_globals, midgard2_globals)
// quota isn't used?
// STD_PHP_INI_BOOLEAN("midgard.quota",        "0", PHP_INI_ALL,    OnUpdateBool, midgard_quota,        midgard2_globals *, midgard2_globals)
PHP_INI_END()

static zend_bool php_midgard_engine_is_enabled()
{
	TSRMLS_FETCH();
	return MGDG(midgard_engine);
}

static zend_bool php_midgard_is_http_env()
{
	TSRMLS_FETCH();
	return MGDG(midgard_http);
}

/* populates global hash with "file => MidgardConfig" pairs. called from MINIT */
static zend_bool php_midgard_initialize_configs()
{
	TSRMLS_FETCH();

	if (MGDG(all_configs) != NULL)
		return TRUE;

	gchar **files = midgard_config_list_files(FALSE);

	if (!files) {
		php_error(E_WARNING, "Didn't find any config file");
		return FALSE;
	}

	MGDG(all_configs) = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	guint i = 0;

	while (files[i] != NULL) {
		MidgardConfig *config = midgard_config_new();
		GError *err = NULL;

		if (!midgard_config_read_file(config, files[i], FALSE, &err)) {
			php_error(E_WARNING, "Failed to read %s config file. %s", files[i],
					  err->message ? err->message : "");
			g_object_unref(config);

			return FALSE;
		}

		g_hash_table_insert(MGDG(all_configs), g_strdup(files[i]), config);
		i++;
	}

	g_strfreev(files);

	return TRUE;
}

PHP_MINIT_FUNCTION(midgard2)
{
	if (zend_get_extension("midgard") != NULL) {
		php_error(E_ERROR, "Module midgard (1.x) already loaded");
		return FAILURE;
	}

	if (zend_get_extension(MIDGARD_PACKAGE_NAME) != NULL) {
		php_error(E_NOTICE, "Module %s already loaded. It's recommended to load it via php.ini",
				  MIDGARD_PACKAGE_NAME);
		return SUCCESS;
	}

	if (PHP_MINIT(midgard2_urlwrapper)(INIT_FUNC_ARGS_PASSTHRU) == FAILURE) {
		return FAILURE;
	}

	global_loghandler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, midgard_error_default_log,
										  NULL);

	//g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL);
	//g_log_set_fatal_mask("GLib-GObject", G_LOG_LEVEL_CRITICAL);

	/* Get DateTime class pointer and set global */
	zend_datetime_class_ptr = php_date_get_date_ce();
	zend_date_timezone_class_ptr = php_date_get_timezone_ce();

	midgard_init();

	/* register Gtype types from schemas */
	if (!g_type_from_name("midgard_language") && midgard_global_schema == NULL) {
		midgard_global_schema = g_object_new(MIDGARD_TYPE_SCHEMA, NULL);
		midgard_schema_init((MidgardSchema *) midgard_global_schema, NULL);
		midgard_schema_read_dir((MidgardSchema *) midgard_global_schema, NULL);
	}

	/* Initialize handlers */
	memcpy(&php_midgard_gobject_handlers, zend_get_std_object_handlers(),
		   sizeof(zend_object_handlers));

	/* Custom handlers hooks */
	php_midgard_gobject_handlers.clone_obj = NULL;
	php_midgard_gobject_handlers.read_property = php_midgard_gobject_read_property;
	php_midgard_gobject_handlers.get_property_ptr_ptr = php_midgard_gobject_get_property_ptr_ptr;
	php_midgard_gobject_handlers.write_property = php_midgard_gobject_write_property;
	php_midgard_gobject_handlers.get_properties = php_midgard_zendobject_get_properties;
	php_midgard_gobject_handlers.has_property = php_midgard_gobject_has_property;

	/* Register midgard_exception class */
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "midgard_error_exception", NULL);
	zend_class_entry *exc_ce = zend_exception_get_default(TSRMLS_C);
	ce_midgard_error_exception = zend_register_internal_class_ex(&ce, exc_ce, NULL TSRMLS_CC);

	// Init various classes
	PHP_MINIT(midgard2_reflection_workaround)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_reflection_property)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_query_builder)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_config)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_blob)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_object_class)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_object)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_user)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_collector)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_connection)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_dbus)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_replicator)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_datetime)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_error)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_transaction)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_view)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_storage)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_key_config_context)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_key_config_file_context)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_key_config)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_key_config_file)(INIT_FUNC_ARGS_PASSTHRU);

	/* Register midgard_metadata class */
	static zend_class_entry midgard_metadata_class_entry;
	INIT_CLASS_ENTRY(midgard_metadata_class_entry, "midgard_metadata", NULL);
	midgard_metadata_class = zend_register_internal_class(&midgard_metadata_class_entry TSRMLS_CC);
	midgard_metadata_class->create_object = php_midgard_gobject_new;

#define MGD_PHP_REGISTER_CONSTANT(name) \
	REGISTER_LONG_CONSTANT(#name, name, CONST_CS | CONST_PERSISTENT)

	/* Register midgard_user types */
	MGD_PHP_REGISTER_CONSTANT(MIDGARD_USER_TYPE_NONE);
	MGD_PHP_REGISTER_CONSTANT(MIDGARD_USER_TYPE_USER);
	MGD_PHP_REGISTER_CONSTANT(MIDGARD_USER_TYPE_ADMIN);

	/* Register properties' midgard types */
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_NONE);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_STRING);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_INT);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_UINT);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_FLOAT);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_BOOLEAN);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_TIMESTAMP);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_LONGTEXT);
	MGD_PHP_REGISTER_CONSTANT(MGD_TYPE_GUID);

	/* Register errcode constants */
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OK);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_ERROR);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_ACCESS_DENIED);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_NO_METADATA);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_NOT_OBJECT);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_INVALID_NAME);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_DUPLICATE);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_HAS_DEPENDANTS);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_RANGE);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_NOT_CONNECTED);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_SG_NOTFOUND);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_INVALID_OBJECT);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_QUOTA);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_INTERNAL);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_NAME_EXISTS);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_NO_STORAGE);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_NO_PARENT);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_INVALID_PROPERTY_VALUE);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_INVALID_PROPERTY);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_USER_DATA);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_DELETED);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_PURGED);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_EXPORTED);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_OBJECT_IMPORTED);
	MGD_PHP_REGISTER_CONSTANT(MGD_ERR_MISSED_DEPENDENCE);

#undef MGD_PHP_REGISTER_CONSTANT

	REGISTER_INI_ENTRIES();

	if (MGDG(superglobals_compat)) {
		php_error(E_DEPRECATED, "midgard.superglobals_compat option is deprecated and might be removed in next release");

		php_mgd_register_auto_global(_MIDGARD);
		php_mgd_register_auto_global(_MIDCOM);
		php_mgd_register_auto_global(_MIDGARD_CONNECTION);
	}

	/* midgard.http is on so we populate list of all available configs */
	if (php_midgard_is_http_env()) {
		if (!php_midgard_initialize_configs()) {
			php_error(E_WARNING, "[Midgard2 minit] Failed to initialize configs");
			return FAILURE;
		}
	}

	if (global_loghandler) {

		g_log_remove_handler(G_LOG_DOMAIN, global_loghandler);
		global_loghandler = 0;
	}

	/* g_log_set_always_fatal(G_LOG_LEVEL_WARNING);
	   g_log_set_fatal_mask("GLib-GObject", G_LOG_LEVEL_WARNING);
	   g_log_set_fatal_mask("GLib", G_LOG_LEVEL_WARNING);
	   g_log_set_fatal_mask("GLib-GObject", G_LOG_LEVEL_CRITICAL);
	   g_log_set_fatal_mask("GLib", G_LOG_LEVEL_CRITICAL); */

	php_midgard_log_enabled = TRUE;

	if (MGDG(midgard_memory_debug)) {
		php_printf("MINIT done (pid = %d)\n", getpid());
	}

	return SUCCESS;
}

static PHP_GINIT_FUNCTION(midgard2)
{
	midgard2_globals->can_deliver_signals = FALSE;
	midgard2_globals->connection_established = FALSE;
	midgard2_globals->all_configs = NULL;
	midgard2_globals->midgard_global_holder = NULL;
}

static void __free_connections(gpointer key, gpointer val, gpointer ud)
{
	MidgardConnection *cnc = MIDGARD_CONNECTION(val);
	g_object_unref(cnc);
}

PHP_MSHUTDOWN_FUNCTION(midgard2_urlwrapper);
PHP_MSHUTDOWN_FUNCTION(midgard2)
{
	UNREGISTER_INI_ENTRIES();

	if (PHP_MSHUTDOWN(midgard2_urlwrapper)(INIT_FUNC_ARGS_PASSTHRU) == FAILURE) {
		return FAILURE;
	}

	// next line is a hack. we do not free resources
	return SUCCESS;

	/* Free schema */
	if (midgard_global_schema != NULL)
		g_object_unref(midgard_global_schema);

	/* Free connections */
	php_midgard_handle_holder_free(&MGDG(midgard_global_holder));

	/*
	  if (all_configs != NULL) {

	  g_hash_table_foreach(all_configs, __free_connections, NULL);

	  } else {

	  MidgardConnection *mgd = mgd_handle();
	  if (mgd != NULL)
	  g_object_unref(mgd);
	  }*/

	return SUCCESS;
}

PHP_RINIT_FUNCTION(midgard2)
{
	if (!php_midgard_engine_is_enabled())
		return FAILURE;

	if (MGDG(midgard_memory_debug)) {
		php_printf("RINIT\n");
	}

	if (php_midgard_is_http_env()) {
		/* all_configs is set during MINIT */
		if (MGDG(all_configs) == NULL) {
			php_error(E_ERROR, "[Midgard2 rinit] Can not handle request without midgard connection");
			return FAILURE;
		}

		// preinitialization of connection (in the future it won't be needed)
		zval *instance;
		zend_call_method_with_0_params(NULL, php_midgard_connection_class, NULL, "get_instance",
									   &instance);
		zval_ptr_dtor(&instance);
	}

	if (MGDG(connection_established) == FALSE) {
		global_loghandler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK,
											  midgard_error_default_log, NULL);
		if (MGDG(midgard_memory_debug)) {
			php_printf("---> g_log_set_handler() => %d\n", global_loghandler);
		}
	}

	/* Initialize closure hash */
	php_midgard_gobject_closure_hash_new();

	if (MGDG(midgard_memory_debug)) {
		php_printf("<= RINIT\n");
	}

	MGDG(can_deliver_signals) = 1;

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(midgard2)
{
	MGDG(can_deliver_signals) = 0;

	if (MGDG(midgard_memory_debug)) {
		php_printf("RSHUTDOWN\n");
	}

	if (global_loghandler) {
		// connection's destructor is not called, if exception happened
		// we need to enforce logging-destructor in this case
		if (MGDG(midgard_memory_debug)) {
			php_printf("---> g_log_remove_handler(..., %d)\n", global_loghandler);
		}
		g_log_remove_handler(G_LOG_DOMAIN, global_loghandler);
		global_loghandler = 0;
		
		if (mgd_handle())
			midgard_connection_set_loghandler (mgd_handle(), 0);
	}

	/* Free all closures
	 * We can not keep them persistant as all data changes per request. */
	if (MGDG(midgard_memory_debug)) {
		php_printf("---> php_midgard_gobject_closure_hash_free()\n");
	}
	php_midgard_gobject_closure_hash_free();

	if (MGDG(midgard_memory_debug)) {
		/* It's not safe, but allow valgrind to print function names.
		 * It simply forces Zend to not unload midgard module */
		zend_module_entry *module;
		int rv = zend_hash_find(&module_registry,
								MIDGARD_PACKAGE_NAME, strlen(MIDGARD_PACKAGE_NAME) + 1,
								(void**)&module);
		if (rv == SUCCESS) {
			php_printf("---> disable module handle\n");
			module->handle = 0;
		}
	}

	if (MGDG(midgard_memory_debug)) {
		php_printf("<= RSHUTDOWN\n");
	}

	return SUCCESS;
}

PHP_MINFO_FUNCTION(midgard2)
{
	size_t i = 0;

	/* first block */
	php_info_print_table_start();
	php_info_print_table_header(2, "Midgard2 Support", "enabled");
	php_info_print_table_row(2, "Midgard2 version", midgard_version());

	while (midgard2_module_entry.functions[i].fname) {
		php_info_print_table_row(2, "", midgard2_module_entry.functions[i].fname);
		i++;
	}

	php_info_print_table_end();

	/* second block */
	php_info_print_table_start();
	php_info_print_table_header(2, "MgdSchema technology support", "enabled");
	php_info_print_table_row(2, "Midgard2 version", midgard_version());
	php_info_print_table_end();

	/* third block */
	/* TODO: we shouldn't output HTML in CLI-mode */
	php_info_print_box_start(0);
	PUTS("<h3><a href=\"http://www.midgard-project.org/\">");
	PUTS("The Midgard Project</a></h3>\n");

	php_printf("This program makes use of the Midgard Content Management engine:<br />");
	php_printf("&copy; 1998-2001 The Midgard Project Ry <br />\n");
	php_printf("&copy; 2002-2009 The Midgard Community<br />\n");
	php_info_print_box_end();

	/* ini-settings block */
	DISPLAY_INI_ENTRIES();
}

/* Fetch static globals. Unfortunately these need to be here since the
   module globals are declared static by the Zend macros
*/
MidgardConnection *mgd_handle()
{
	zval *instance;
	TSRMLS_FETCH();

	if (!php_midgard_is_connected())
		return NULL;

	zend_call_method_with_0_params(NULL, php_midgard_connection_class, NULL, "get_instance",
								   &instance);

	MidgardConnection *connection = __midgard_connection_get_ptr(instance);
	zval_ptr_dtor(&instance);
	return connection;
}

zend_class_entry *midgard_php_register_internal_class(const gchar *class_name, GType class_type,
													  zend_class_entry ce, function_entry *fe)
{
	TSRMLS_FETCH();

	g_assert(class_name != NULL);
	g_assert(&ce != NULL);
	g_assert(fe != NULL);

	if (class_type == 0) {
		php_error(E_ERROR, "'%s' class  is not registered in GType system!", class_name);
		return NULL;
	}

	if (!G_TYPE_IS_DERIVED(class_type)) {
		php_error(E_ERROR, "'%s' class doesn't have base-class in GType system!", class_name);
		return NULL;
	}

	GType parent_type = g_type_parent(class_type);

	if (parent_type <= 0) {
		php_error(E_ERROR, "'%s' class has invalid base-class in GType system!", class_name);
		return NULL;
	}

	gchar *parent_name = (gchar *) g_type_name(parent_type);
	zend_class_entry *pce = php_midgard_get_class_ptr_by_name(parent_name);

	if (NULL == pce) {
		php_error(E_ERROR, "'%s' class's parent '%s' is not registered in php", class_name, parent_name);
		return NULL;
	}

	return zend_register_internal_class_ex(&ce , pce, parent_name TSRMLS_CC);
}

ZEND_GET_MODULE(midgard2)

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
