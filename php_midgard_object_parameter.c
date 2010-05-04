/*
 * Copyright (C) 2006, 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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

#include "php_midgard.h"
#include "php_midgard_gobject.h"

#include "php_midgard__helpers.h"

PHP_FUNCTION(_php_midgard_object_list_parameters)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *domain = NULL;
	int domain_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &domain, &domain_length) != SUCCESS) {
		return;
	}

	array_init(return_value);

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	MidgardObject **objects = midgard_object_list_parameters(mobj, domain);

	if (objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_parameter", return_value TSRMLS_CC);
		g_free(objects);
	}
}

PHP_FUNCTION(php_midgard_object_has_parameters)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	zend_bool rv = (zend_bool)midgard_object_has_parameters(mobj);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_delete_parameters)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params) != SUCCESS) {
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);

	zend_bool rv = (zend_bool) midgard_object_delete_parameters(mobj, n_params, parameters);
	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_purge_parameters)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params) != SUCCESS) {
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);

	zend_bool rv = (zend_bool) midgard_object_purge_parameters(mobj, n_params, parameters);
	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_find_parameters)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *params = NULL;
	MidgardObject **objects = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &params) != SUCCESS) {
		return;
	}

	array_init(return_value);

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);

	objects = midgard_object_find_parameters(mobj, n_params, parameters);
	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	if (objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_parameter", return_value TSRMLS_CC);
		g_free(objects);
	}
}

PHP_FUNCTION(_php_midgard_object_get_parameter)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *domain, *name;
	int domain_length, name_length;
	const GValue *gvalue;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &domain, &domain_length, &name, &name_length) != SUCCESS) {
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	gvalue = midgard_object_get_parameter(mobj, domain, name);

	if (gvalue == NULL)
		RETURN_NULL();

	php_midgard_gvalue2zval((GValue *)gvalue, return_value);
}

PHP_FUNCTION(_php_midgard_object_set_parameter)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *domain, *name, *strval;
	int domain_length, name_length, strval_length;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|z",
				&domain, &domain_length,
				&name, &name_length,
				&strval, &strval_length, &zbool) != SUCCESS)
	{
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	if (strval == NULL)
		strval = "";

	GValue sval = {0, };
	g_value_init(&sval, G_TYPE_STRING);
	g_value_set_string(&sval, strval);

	gboolean rv = midgard_object_set_parameter(mobj, domain, name, (GValue *)&sval);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_parameter)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *domain, *name, *strval = NULL;
	int domain_length, name_length, strval_length;
	int _args = ZEND_NUM_ARGS();

	if (zend_parse_parameters(_args TSRMLS_CC, "ss|s",
				&domain, &domain_length,
				&name, &name_length,
				&strval, &strval_length) != SUCCESS)
	{
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	switch (_args) {
		case 2:
			// get parameter
			{
				const GValue *gvalue = midgard_object_get_parameter(mobj, domain, name);

				if (gvalue != NULL)
					php_midgard_gvalue2zval((GValue *)gvalue, return_value);
				else
					RETURN_NULL();
			}
			break;

		case 3:
			// set parameter
			{
				if (strval == NULL)
					strval = "";

				GValue *sval = g_new0(GValue, 1);
				g_value_init(sval, G_TYPE_STRING);
				g_value_set_string(sval, strval);

				gboolean rv = midgard_object_set_parameter(mobj, domain, name, (GValue *)sval);

				RETURN_BOOL(rv);
			}
			break;
	}
}
