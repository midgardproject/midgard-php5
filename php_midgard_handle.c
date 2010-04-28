/* Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
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

#include "php_midgard_handle.h"

/* Get configuration name declared in vhost configuration.
   midgard.configuration = "midgard"; */
static const gchar *__get_configuration_name()
{
	TSRMLS_FETCH();
	const gchar *name = MGDG(midgard_configuration);

	if (!name || *name == '\0')
		return NULL;

	return (const gchar *) name;
}

/* Get configuration file path. Shoul dbe declaored in vhost configuration
   midgard.configuration_file = "/home/user/config/midgard.conf" */
static const gchar *__get_configuration_filepath()
{
	TSRMLS_FETCH();
	const gchar *path = MGDG(midgard_configuration_file);

	if (!path || *path == '\0')
		return NULL;

	return (const gchar *) path;
}

struct _MgdGHolder {
	GHashTable *names;
	GHashTable *files;
};

/* Initialize global midgard handle holder */
void php_midgard_handle_holder_init(MgdGHolder **mgh)
{
	if (*mgh != NULL)
		return;

	*mgh = g_new(MgdGHolder, 1);

	(*mgh)->names = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	(*mgh)->files = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);

	return;
}

/* Free handle holder */
void php_midgard_handle_holder_free(MgdGHolder **mgh)
{
	if ((*mgh)->names != NULL)
		g_hash_table_destroy((*mgh)->names);

	if ((*mgh)->files != NULL)
		g_hash_table_destroy((*mgh)->files);

	g_free(*mgh);

	*mgh = NULL;

	return;
}

/* Initialize per request copy. Early sets loghandler so we can log messages. */
MidgardConnection *__handle_set(MidgardConnection *mgd)
{
	TSRMLS_FETCH();
	g_assert(mgd != NULL);

	MidgardConnection *copy = midgard_connection_copy(mgd);

	guint loghandler = g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_MASK, php_midgard_log_errors, (gpointer)copy);
	midgard_connection_set_loghandler(copy, loghandler);

	if (MGDG(midgard_memory_debug)) {
		php_printf("---> midgard_connection_set_loghandler(%d)\n", loghandler);
	}

	return copy;
}

static MidgardConnection *__handle_from_global_config(MgdGHolder *mgh, GHashTable *global_cfgs, const gchar *config_name)
{
	if (global_cfgs == NULL)
		return NULL;

	/* This is per process connection handler */
	MidgardConnection *mgd = NULL;
	/* This is per request connection handler */
	MidgardConnection *mgd_copy = NULL;

	/* First, let's check if given configuration name already eixts in global holder.
	   If it exists, get a pointer */
	mgd = g_hash_table_lookup(mgh->names, config_name);

	if (mgd != NULL) {
		if (!midgard_connection_reopen(mgd)) {
			php_error(E_WARNING, "Failed to reopen lost connection");
			return NULL;
		}

		mgd_copy = __handle_set(mgd);
		return mgd_copy;
	}

	/* Check if configuration exists in system config directory. */
	MidgardConfig *config = g_hash_table_lookup(global_cfgs, config_name);

	if (config == NULL)
		return NULL;

	mgd = midgard_connection_new();

	if (!midgard_connection_open_config(mgd, config)) {
		php_error(E_WARNING, "Failed to open connection using given '%s' configuration", config_name);
		g_object_unref(mgd);
		return NULL;
	}

	mgd_copy = __handle_set(mgd);

	/* Insert connection in process' pool */
	g_hash_table_insert(mgh->names, g_strdup(config_name), mgd);

	return mgd_copy;
}

static MidgardConnection *__handle_from_filepath(MgdGHolder *mgh, const gchar *config_path)
{
	/* This is per process connection handler */
	MidgardConnection *mgd = NULL;
	/* This is per request connection handler */
	MidgardConnection *mgd_copy = NULL;

	/* First, let's check if given configuration path already eixts in global holder.
	   If it exists, get a pointer */
	mgd = g_hash_table_lookup(mgh->files, config_path);

	if (mgd != NULL) {
		if (!midgard_connection_reopen(mgd)) {
			php_error(E_WARNING, "Failed to reopen lost connection");
			return NULL;
		}

		mgd_copy = __handle_set(mgd);
		return mgd_copy;
	}

	/* Open configuration from given filepath */
	mgd = midgard_connection_new();
	GError *err = NULL;

	if (!midgard_connection_open_from_file(mgd, config_path, &err)) {
		php_error(E_WARNING, "Failed to open connection using given '%s' configuration file: %s", 
		                     config_path,
		                     err && err->message ? err->message : "Unknown reason");
		g_error_free(err);
		g_object_unref(mgd);
		return NULL;
	}

	mgd_copy = __handle_set(mgd);

	/* Insert connection in process' pool */
	g_hash_table_insert(mgh->files, g_strdup(config_path), mgd);

	return mgd_copy;
}

MidgardConnection *php_midgard_handle_lookup(MgdGHolder **mgh, GHashTable *global_cfgs)
{
	if (*mgh == NULL)
		php_midgard_handle_holder_init(mgh);

	const gchar *config_path = __get_configuration_filepath();

	if (config_path != NULL) {
		MidgardConnection *handle = __handle_from_filepath(*mgh, config_path);

		if (handle != NULL) {
			return handle;
		}

		// handle is NULL. falling back to file-based search of config
	}

	const gchar *config_name = __get_configuration_name();

	if (config_name != NULL)
		return __handle_from_global_config(*mgh, global_cfgs, config_name);

	return NULL;
}
