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
#include "php_midgard__helpers.h"
#include <zend_exceptions.h>

zend_class_entry *php_midgard_user_class;

static const gchar *class_doc_comment = "<a href=\"http://www.midgard-project.org/api-docs/midgard/core/mjolnir/midgard-user.html\">Core API docs</a>";

#define _GET_USER_OBJECT \
	zval *zval_object = getThis(); \
	MidgardUser *user = MIDGARD_USER(__php_gobject_ptr(zval_object)); \
	if (!user) \
		php_error(E_ERROR, "Can not find underlying user instance");

/* Object constructor */
static PHP_METHOD(midgard_user, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	GObject *gobject;

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		zval *params = NULL;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params) != SUCCESS) {
			zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "invalid parameter-list passed");
			return;
		}

		guint n_params = 0;
		GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
		MidgardUser *user = midgard_user_new(mgd_handle(), n_params, parameters);
	
		PHP_MGD_FREE_GPARAMETERS(parameters, n_params);	

		if (!user) {
			php_midgard_error_exception_throw(mgd_handle());
			return;
		}

		MGD_PHP_SET_GOBJECT(zval_object, user);
	} else {
		// we already have gobject injected
	}

	php_midgard_init_properties_objects(zval_object);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user___construct, 0, 1, 1)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, auth)
{
	RETVAL_FALSE;
	CHECK_MGD;
	const gchar *name, *password, *sitegroup = NULL;
	gint name_length, password_length, sitegroup_length;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|sb",
				&name, &name_length,
				&password, &password_length,
				&sitegroup, &sitegroup_length,
				&zbool) == FAILURE)
	{
		return;
	}

	MidgardConnection *mgd = mgd_handle();
	MidgardUser *user = midgard_user_auth(mgd, name, password, sitegroup, zbool);

	if (user == NULL)
		RETURN_FALSE;

	object_init_ex(return_value, php_midgard_user_class);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(user));
	zend_call_method_with_0_params(&return_value, php_midgard_user_class, &php_midgard_user_class->constructor, "__construct", NULL);

	/* 1. Returned midgard_user is kind of reference, so mark it as reference.
	 * 2. Increase reference count, so object is not destroyed by zend before request end */
	Z_SET_ISREF_P(return_value);
	zval_add_ref(&return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_auth, 0, 1, 2)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, sitegroup)
	ZEND_ARG_INFO(0, trusted_auth)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, set_person)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zobject;
	zend_bool rv = FALSE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;

	_GET_USER_OBJECT;
	rv = midgard_user_set_person (user, MIDGARD_OBJECT(__php_gobject_ptr(zobject)));
	RETURN_BOOL (rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_set_person, 0, 0, 1)
	ZEND_ARG_OBJ_INFO (0, person, midgard_person, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, get_person)
{
	RETVAL_FALSE;
	CHECK_MGD;

	zend_class_entry *person_ce =
		php_midgard_get_mgdschema_class_ptr_by_name("midgard_person");

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;

	const MgdObject *person = midgard_user_get_person(user);

	if (person == NULL)
		RETURN_NULL();

	object_init_ex(return_value, person_ce);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(person));
	zend_call_method_with_0_params(&return_value, person_ce, &person_ce->constructor, "__construct", NULL);

	Z_SET_ISREF_P(return_value);
	zval_add_ref(&return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_get_person, 0, 1, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, is_user)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_is_user(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_is_user, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, is_admin)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_is_admin(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_is_admin, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, get)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &params) != SUCCESS) 
		return;

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MidgardUser *user = midgard_user_get(mgd_handle(), n_params, parameters);
	
	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	if (user == NULL)
		RETURN_NULL();

	/* HACK, there's no safe API for this (or at least is unknown for me) */
	char *class_name = EG(scope)->name;
	zend_class_entry *ce = zend_fetch_class(class_name, strlen(class_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	object_init_ex(return_value, ce);
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(user));
	zend_call_method_with_0_params(&return_value, ce, &ce->constructor, "__construct", NULL);

	Z_SET_ISREF_P(return_value);
	zval_add_ref(&return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_get, 0, 1, 1)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, query)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &params) != SUCCESS) 
		return;

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MidgardUser **users = midgard_user_query(mgd_handle(), n_params, parameters);
	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	array_init(return_value);

	if (!users)
		return;

	guint i = 0;

	while (users[i] != NULL) {

		zval *zobject;
		MAKE_STD_ZVAL(zobject);
		object_init_ex(zobject, php_midgard_user_class); 

		php_midgard_gobject *php_gobject = __php_objstore_object(zobject);
		php_gobject->gobject = G_OBJECT(users[i]);

		zend_hash_next_index_insert(HASH_OF(return_value), &zobject, sizeof(zval *), NULL);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_query, 0, 0, 1)
	ZEND_ARG_INFO(0, properties)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, create)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_create(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_create, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, update)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_update(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_update, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, delete)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_delete(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_delete, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, login)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_login(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_login, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_user_root_login, 0, 0, 2)
	ZEND_ARG_INFO(0, login)
	ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_user, logout)
{
	RETVAL_FALSE;
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_USER_OBJECT;
	RETURN_BOOL(midgard_user_logout(user));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_user_logout, 0)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
void php_midgard_user_init (int module_number)
{
	static function_entry midgard_user_methods[] = {
		PHP_ME(midgard_user,	__construct,	arginfo_midgard_user___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_user,	auth,		arginfo_midgard_user_auth,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)	
		PHP_ME(midgard_user,    is_user,	arginfo_midgard_user_is_user,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    is_admin,       arginfo_midgard_user_is_admin,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    set_person,     arginfo_midgard_user_set_person,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    get_person,     arginfo_midgard_user_get_person,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    get,     	arginfo_midgard_user_get,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_user,    query,     	arginfo_midgard_user_query,		ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
		PHP_ME(midgard_user,    create,     	arginfo_midgard_user_create,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    update,     	arginfo_midgard_user_update,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    delete,     	arginfo_midgard_user_delete,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_user,    login,     	arginfo_midgard_user_login,		ZEND_ACC_PUBLIC)	
		PHP_ME(midgard_user,    logout,     	arginfo_midgard_user_logout,		ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_user_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			php_midgard_user_class_entry,
			"midgard_user", midgard_user_methods);

	php_midgard_user_class =
		zend_register_internal_class_ex (
				&php_midgard_user_class_entry, NULL, "midgard_dbobject" TSRMLS_CC);

	/* Set function to initialize underlying data */
	php_midgard_user_class->create_object = php_midgard_gobject_new;
	php_midgard_user_class->doc_comment = g_strdup (class_doc_comment);
}
