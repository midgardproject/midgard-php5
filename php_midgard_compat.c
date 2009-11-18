/*
 * Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
 *
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

#include "php_midgard_compat.h"
#include "php_midgard__helpers.h"

PHP_FUNCTION(mgd_version)
{
	RETVAL_NULL();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	const gchar *version = midgard_version();

	if (!version)
		version = "";

	RETURN_STRING((gchar *)version, 1);
}

PHP_FUNCTION(mgd_auth_midgard)
{
	CHECK_MGD;
	RETVAL_FALSE;

	php_error(E_NOTICE, "mgd_auth_midgard is obsolete. Use midgard_user::auth instead.");

	gchar *username, *password , *sgname = NULL;
	guint username_length, password_length;
	guint oldcookie;
	MidgardConnection *mgd = mgd_handle();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l",
						&username, &username_length,
						&password, &password_length,
						&oldcookie) == FAILURE)
	{
		return;
	}

	/* TODO, split username ( to get SG name ) if needed */
	MidgardUser *user = midgard_user_auth(mgd, username, password, sgname, FALSE);

	if (!user)
		RETURN_FALSE;

	/* It's important to initialize *any* object here.
	 * MidgardUser will be automagicaly unref'ed at request end */

	zval *zobject = NULL;

	MAKE_STD_ZVAL(zobject);
	object_init_ex(zobject, php_midgard_user_class);
	MGD_PHP_SET_GOBJECT(zobject, G_OBJECT(user));
	zend_call_method_with_0_params(&zobject, php_midgard_user_class, &php_midgard_user_class->constructor, "__construct", NULL);

	RETURN_TRUE;
}

PHP_FUNCTION(mgd_unsetuid)
{
	CHECK_MGD;
	RETVAL_FALSE;

	php_error(E_NOTICE, "mgd_unsetuid is obsolete.");
}

PHP_FUNCTION(mgd_issetuid)
{
	CHECK_MGD;
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	/*
	gint setuid = 3;
	MidgardConnection *mgd = mgd_handle(); */

	php_error(E_NOTICE, "mgd_issetuid is obsolete.");
}
