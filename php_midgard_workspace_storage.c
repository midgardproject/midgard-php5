/*
 * Copyright (C) 2011 Piotr Pokora <piotrek.pokora@gmail.com>
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

zend_class_entry *php_midgard_workspace_storage_class;
zend_class_entry *php_midgard_workspace_context_class;
zend_class_entry *php_midgard_workspace_class;

/* MidgardWorkspaceStorage */
static PHP_METHOD (midgard_workspace_storage, get_path)
{
	if (zend_parse_parameters_none () == FAILURE)
		return;

	MidgardWorkspaceStorage *self = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (getThis()));
	const char *path = midgard_workspace_storage_get_path (self);

	if (path)
		RETURN_STRING (path, 1);

	RETURN_NULL ();
}

ZEND_BEGIN_ARG_INFO (arginfo_midgard_workspace_storage_get_path, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_storage, get_workspace_by_name)
{
	char *name;
	int name_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_length) == FAILURE)
		return;

	MidgardWorkspaceStorage *self = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (getThis()));
	MidgardWorkspaceStorage *workspace = midgard_workspace_storage_get_workspace_by_name (self, name);

	if (!workspace)
		RETURN_NULL ();

	if (MIDGARD_IS_WORKSPACE (self))
		object_init_ex (return_value, php_midgard_workspace_class);
	else
		object_init_ex (return_value, php_midgard_workspace_context_class);

	MGD_PHP_SET_GOBJECT (return_value, workspace);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_storage_get_workspace_by_name, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_storage, list_children)
{
	if (zend_parse_parameters_none () == FAILURE)
		return;

	guint n_objects;
	MidgardWorkspaceStorage *self = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (getThis()));
	MidgardWorkspaceStorage **children = midgard_workspace_storage_list_children (self, &n_objects);

	array_init (return_value);

	if (!children)
		return;

	const char *g_class_name = G_OBJECT_TYPE_NAME (children[0]);
	const char *ws_cname = g_class_name_to_php_class_name (g_class_name);
	zend_class_entry *ce = zend_fetch_class ((char *) ws_cname, strlen (ws_cname), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	int i;
	for (i = 0; i < n_objects; i++) {
		zval *zobject;
                MAKE_STD_ZVAL (zobject);

                php_midgard_gobject_new_with_gobject (zobject, ce, G_OBJECT (children[i]), TRUE TSRMLS_CC);
                zend_hash_next_index_insert (HASH_OF (return_value), &zobject, sizeof (zval *), NULL);
	}
}

ZEND_BEGIN_ARG_INFO (arginfo_midgard_workspace_storage_list_children, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_storage, list_workspace_names)
{
	if (zend_parse_parameters_none () == FAILURE)
		return;

	guint n_names;
	MidgardWorkspaceStorage *self = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (getThis()));
	char **names = midgard_workspace_storage_list_workspace_names (self, &n_names);

	array_init (return_value);

	if (!names)
		return;

	int i;
	for (i = 0; i < n_names; i++)
		add_index_string (return_value, i, names[i], 1);
}

ZEND_BEGIN_ARG_INFO (arginfo_midgard_workspace_storage_list_workspace_names, 0)
ZEND_END_ARG_INFO ()

/* MidgardWorkspace */

static PHP_METHOD (midgard_workspace, __construct)
{
	zval *object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardWorkspace *self = midgard_workspace_new ();
	if (!self)
		RETURN_FALSE;

	MGD_PHP_SET_GOBJECT (object, self);
}

ZEND_BEGIN_ARG_INFO (arginfo_midgard_workspace___construct, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace, get_context)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardWorkspace *self = MIDGARD_WORKSPACE (__php_gobject_ptr (getThis()));
	const MidgardWorkspaceContext *context = midgard_workspace_get_context (self);

	if (!context)
		RETURN_NULL ();

	object_init_ex (return_value, php_midgard_workspace_context_class);
	MGD_PHP_SET_GOBJECT (return_value, g_object_ref (G_OBJECT (context)));
}

ZEND_BEGIN_ARG_INFO (arginfo_midgard_workspace_get_context, 0)
ZEND_END_ARG_INFO ()

/* MidgardWorkspaceContext */

static PHP_METHOD (midgard_workspace_context, __construct)
{
	zval *object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardWorkspaceContext *self = midgard_workspace_context_new ();
	if (!self)
		RETURN_FALSE;

	MGD_PHP_SET_GOBJECT (object, self);
}

ZEND_BEGIN_ARG_INFO (arginfo_midgard_workspace_context___construct, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_context, has_workspace)
{
	zval *z_workspace = NULL;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "O", &z_workspace, php_midgard_workspace_class) == FAILURE) {
		return;
	}

	MidgardWorkspaceContext *self = MIDGARD_WORKSPACE_CONTEXT (__php_gobject_ptr (getThis ()));
	MidgardWorkspace *workspace = MIDGARD_WORKSPACE (__php_gobject_ptr (z_workspace));

	zend_bool result = midgard_workspace_context_has_workspace (self, workspace);
	RETURN_BOOL(result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_context_has_workspace, 0, 0, 1)
	ZEND_ARG_OBJ_INFO (0, workspace, midgard_workspace, 0)
ZEND_END_ARG_INFO ()

/* MidgardWorkspaceManager */
static PHP_METHOD (midgard_workspace_manager, __construct)
{
	zval *z_mgd = NULL;
	const gchar *g_class_name = g_type_name (MIDGARD_TYPE_CONNECTION);
	const char *php_class_name = g_class_name_to_php_class_name (g_class_name);
	zend_class_entry *ce = zend_fetch_class ((char *)php_class_name, strlen(php_class_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "O", &z_mgd, ce) == FAILURE) {
		return;
	}

       	zval *object = getThis();

	MidgardWorkspaceManager *self = midgard_workspace_manager_new (MIDGARD_CONNECTION (__php_gobject_ptr (z_mgd)));
        if (!self)
		RETURN_FALSE;

	MGD_PHP_SET_GOBJECT (object, self);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO (0, mgd, midgard_connection, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, get_workspace_by_path)
{
	char *path;
	int path_length;
	zval *z_workspace;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "Os", &z_workspace, php_midgard_workspace_storage_class, &path, &path_length) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
      	MidgardWorkspaceStorage *workspace = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (z_workspace));

	GError *error = NULL;
        zend_bool result = midgard_workspace_manager_get_workspace_by_path (self, workspace, path, &error);

	if (error) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to get workspace by path. %s", error->message);
		g_error_free(error);
	}

	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_get_workspace_by_path, 0, 0, 2)
	ZEND_ARG_OBJ_INFO (0, workspace, midgard_workspace_storage, 0)
	ZEND_ARG_INFO (0, path)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, create_workspace)
{
	char *path;
	int path_length;
	zval *z_workspace;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "Os", &z_workspace, php_midgard_workspace_storage_class, &path, &path_length) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
      	MidgardWorkspaceStorage *workspace = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (z_workspace));

	GError *error = NULL;
        zend_bool result = midgard_workspace_manager_create_workspace (self, workspace, path, &error);

	if (error) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to create workspace. %s", error->message);
		g_error_free(error);
	}

	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_create_workspace, 0, 0, 2)
	ZEND_ARG_OBJ_INFO (0, workspace, midgard_workspace_storage, 0)
	ZEND_ARG_INFO (0, path)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, update_workspace)
{
	zval *z_workspace;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "O", &z_workspace, php_midgard_workspace_storage_class) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
      	MidgardWorkspaceStorage *workspace = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (z_workspace));

	GError *error = NULL;
        zend_bool result = midgard_workspace_manager_update_workspace (self, workspace, &error);

	if (error) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to update workspace. %s", error->message);
		g_error_free(error);
	}

	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_update_workspace, 0, 0, 1)
	ZEND_ARG_OBJ_INFO (0, workspace, midgard_workspace_storage, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, purge_workspace)
{
	zval *z_workspace;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "O", &z_workspace, php_midgard_workspace_storage_class) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
      	MidgardWorkspaceStorage *workspace = MIDGARD_WORKSPACE_STORAGE (__php_gobject_ptr (z_workspace));

	GError *error = NULL;
        zend_bool result = midgard_workspace_manager_purge_workspace (self, workspace, &error);

	if (error) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to purge workspace. %s", error->message);
		g_error_free(error);
	}

	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_purge_workspace, 0, 0, 1)
	ZEND_ARG_OBJ_INFO (0, workspace, midgard_workspace_storage, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, path_exists)
{
	char *path;
	int path_length;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "s", &path, &path_length) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
        zend_bool result = midgard_workspace_manager_path_exists (self, path);
	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_path_exists, 0, 0, 1)
	ZEND_ARG_INFO (0, path)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, purge_content)
{
	char *type;
	int type_length;
	zval *z_workspace;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "sO", &type, &type_length, &z_workspace, php_midgard_workspace_class) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
      	MidgardWorkspace *workspace = MIDGARD_WORKSPACE (__php_gobject_ptr (z_workspace));

	GError *error = NULL;
        zend_bool result = midgard_workspace_manager_purge_content (self, type, workspace, &error);

	if (error) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to purge %s content from workspace. %s", type, error->message);
		g_error_free(error);
	}

	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_purge_content, 0, 0, 2)
	ZEND_ARG_INFO (0, type)
	ZEND_ARG_OBJ_INFO (0, workspace, midgard_workspace_storage, 0)
ZEND_END_ARG_INFO ()

static PHP_METHOD (midgard_workspace_manager, move_content)
{
	char *type;
	int type_length;
	zval *z_src_workspace;
	zval *z_dest_workspace;

	if (zend_parse_parameters (ZEND_NUM_ARGS () TSRMLS_CC, "sOO", &type, &type_length,
				&z_src_workspace, php_midgard_workspace_class,
				&z_dest_workspace, php_midgard_workspace_class) == FAILURE) {
		return;
	}

	MidgardWorkspaceManager *self = MIDGARD_WORKSPACE_MANAGER (__php_gobject_ptr (getThis ()));
      	MidgardWorkspace *src_workspace = MIDGARD_WORKSPACE (__php_gobject_ptr (z_src_workspace));
      	MidgardWorkspace *dest_workspace = MIDGARD_WORKSPACE (__php_gobject_ptr (z_dest_workspace));

	GError *error = NULL;
        zend_bool result = midgard_workspace_manager_move_content (self, type, src_workspace, dest_workspace, &error);

	if (error) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Failed to move %s content betwen workspaces. %s", type, error->message);
		g_error_free(error);
	}

	RETURN_BOOL (result);
}

ZEND_BEGIN_ARG_INFO_EX (arginfo_midgard_workspace_manager_move_content, 0, 0, 3)
	ZEND_ARG_INFO (0, type)
	ZEND_ARG_OBJ_INFO (0, src_workspace, midgard_workspace_storage, 0)
	ZEND_ARG_OBJ_INFO (0, dest_workspace, midgard_workspace_storage, 0)
ZEND_END_ARG_INFO ()

PHP_MINIT_FUNCTION (midgard2_workspaces)
{
	static function_entry midgard_workspace_storage_methods[] = {
		PHP_ME (midgard_workspace_storage, get_path,			arginfo_midgard_workspace_storage_get_path,    ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_storage, get_workspace_by_name,	arginfo_midgard_workspace_storage_get_workspace_by_name,    ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_storage, list_children,		arginfo_midgard_workspace_storage_list_children,    ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_storage, list_workspace_names,	arginfo_midgard_workspace_storage_list_workspace_names,    ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_workspace_storage_class_entry;
	INIT_CLASS_ENTRY (php_midgard_workspace_storage_class_entry, "midgard_workspace_storage", midgard_workspace_storage_methods);

	php_midgard_workspace_storage_class = zend_register_internal_class (&php_midgard_workspace_storage_class_entry TSRMLS_CC);
	php_midgard_workspace_storage_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_workspace_storage_class->create_object = php_midgard_gobject_new;
	php_midgard_workspace_storage_class->doc_comment = strdup ("Base class for workspaces");


	static function_entry midgard_workspace_methods[] = {
		PHP_ME (midgard_workspace, __construct,		arginfo_midgard_workspace___construct,		ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME (midgard_workspace, get_context,		arginfo_midgard_workspace_get_context,		ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_workspace_class_entry;
	INIT_CLASS_ENTRY (php_midgard_workspace_class_entry, "midgard_workspace", midgard_workspace_methods);

	php_midgard_workspace_class = zend_register_internal_class_ex (&php_midgard_workspace_class_entry, php_midgard_workspace_storage_class, "midgard_workspace_storage" TSRMLS_CC);
	php_midgard_workspace_class->create_object = php_midgard_gobject_new;
	php_midgard_workspace_class->doc_comment = strdup ("Represents single workspace");

	static function_entry midgard_workspace_context_methods[] = {
		PHP_ME (midgard_workspace_context, __construct,		arginfo_midgard_workspace_context___construct,		ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME (midgard_workspace_context, has_workspace,	arginfo_midgard_workspace_context_has_workspace,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_workspace_context_class_entry;
	INIT_CLASS_ENTRY (php_midgard_workspace_context_class_entry, "midgard_workspace_context", midgard_workspace_context_methods);

	php_midgard_workspace_context_class = zend_register_internal_class_ex (&php_midgard_workspace_context_class_entry, php_midgard_workspace_storage_class, "midgard_workspace_storage" TSRMLS_CC);
	php_midgard_workspace_context_class->create_object = php_midgard_gobject_new;
	php_midgard_workspace_context_class->doc_comment = strdup ("Represents workspaces' tree");

	return SUCCESS;

	static function_entry midgard_workspace_manager_methods[] = {
		PHP_ME (midgard_workspace_manager, __construct,		arginfo_midgard_workspace_manager___construct,		ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME (midgard_workspace_manager, get_workspace_by_path,	arginfo_midgard_workspace_manager_get_workspace_by_path,	ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_manager, create_workspace,	arginfo_midgard_workspace_manager_create_workspace,	ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_manager, update_workspace,	arginfo_midgard_workspace_manager_update_workspace,	ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_manager, purge_workspace,	arginfo_midgard_workspace_manager_purge_workspace,	ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_manager, path_exists,		arginfo_midgard_workspace_manager_path_exists,		ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_manager, purge_content,	arginfo_midgard_workspace_manager_purge_content,	ZEND_ACC_PUBLIC)
		PHP_ME (midgard_workspace_manager, move_content,	arginfo_midgard_workspace_manager_move_content,		ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_workspace_manager_class_entry;
	INIT_CLASS_ENTRY (php_midgard_workspace_manager_class_entry, "midgard_workspace_manager", midgard_workspace_manager_methods);

	php_midgard_workspace_context_class = zend_register_internal_class (&php_midgard_workspace_manager_class_entry TSRMLS_CC);
	php_midgard_workspace_context_class->create_object = php_midgard_gobject_new;
	php_midgard_workspace_context_class->doc_comment = strdup ("Workspaces' manager");

	return SUCCESS;
}

