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
	CHECK_MGD;
	zval *zval_object = getThis();
	gchar *domain = NULL;
	guint domain_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &domain, &domain_length) != SUCCESS) {
		return;
	}

	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));
	MgdObject **objects = midgard_object_list_parameters(mobj, domain);
	array_init(return_value);

	if (objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_parameter", return_value TSRMLS_CC);
		g_free(objects);
	}
}

PHP_FUNCTION(php_midgard_object_has_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	zend_bool rv = (zend_bool)midgard_object_has_parameters(mobj);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_delete_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params) != SUCCESS) {
		return;
	}

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));
	zend_bool rv = (zend_bool) midgard_object_delete_parameters(mobj, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_purge_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params) != SUCCESS) {
		return;
	}

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));
	zend_bool rv = (zend_bool) midgard_object_purge_parameters(mobj, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_find_parameters)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	zval *params = NULL;
	MgdObject **objects = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &params) != SUCCESS) {
		return;
	}

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params);
	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));
	objects = midgard_object_find_parameters(mobj, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	array_init(return_value);

	if (objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_parameter", return_value TSRMLS_CC);
		g_free(objects);
	}
}

PHP_FUNCTION(_php_midgard_object_get_parameter)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *domain, *name;
	guint domain_length, name_length;
	const GValue *gvalue;
	zval *zval_object = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"ss",
				&domain, &domain_length,
				&name, &name_length) != SUCCESS)
		return;

	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));
	gvalue = midgard_object_get_parameter(mobj, domain, name);

	if (gvalue == NULL)
		RETURN_NULL();

	php_midgard_gvalue2zval((GValue *)gvalue, return_value);
}

PHP_FUNCTION(_php_midgard_object_set_parameter)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *domain, *name;
	guint domain_length, name_length;
	zend_bool zbool = FALSE;
	gboolean rv;
	zval *zval_object = getThis();
	gchar *strval;
	guint strval_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
				"sss|z",
				&domain, &domain_length,
				&name, &name_length,
				&strval, &strval_length, &zbool) != SUCCESS)
	{
		return;
	}

	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	if (strval == NULL)
		strval = "";

	GValue sval = {0, };
	g_value_init(&sval, G_TYPE_STRING);
	g_value_set_string(&sval, strval);

	rv = midgard_object_set_parameter(mobj, domain, name, (GValue *)&sval);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_parameter)
{
	RETVAL_FALSE;
	CHECK_MGD;
	gchar *domain, *name;
	guint domain_length, name_length;
	guint _args = ZEND_NUM_ARGS();
	gboolean rv;
	zval *zval_object = getThis();
	gchar *strval;
	guint strval_length;
	GValue *sval;

	if (zend_parse_parameters(_args TSRMLS_CC,
				"ss|s",
				&domain, &domain_length,
				&name, &name_length,
				&strval, &strval_length) != SUCCESS)
	{
		return;
	}

	MgdObject *mobj = MIDGARD_OBJECT(__php_gobject_ptr(zval_object));

	switch (_args) {
		case 2:
			// get parameter
			{
				const GValue *gvalue;
				gvalue = midgard_object_get_parameter(mobj, domain, name);
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

				sval = g_new0(GValue, 1);
				g_value_init(sval, G_TYPE_STRING);
				g_value_set_string(sval, strval);
				rv = midgard_object_set_parameter(mobj, domain, name, (GValue *)sval);

				RETURN_BOOL(rv);
			}
			break;
	}
}
