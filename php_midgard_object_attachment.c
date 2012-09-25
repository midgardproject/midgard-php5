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

PHP_FUNCTION(_php_midgard_object_list_attachments)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	array_init(return_value);

	guint n_objects;
	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	MidgardObject **objects = midgard_object_list_attachments(mobj, &n_objects);

	if (objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_attachment", return_value TSRMLS_CC);
		g_free(objects);
	}
}

PHP_FUNCTION(php_midgard_object_has_attachments)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	zend_bool rv = (zend_bool)midgard_object_has_attachments(mobj);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_delete_attachments)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params) != SUCCESS) {
		return;
	}

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params TSRMLS_CC);
	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	zend_bool rv = (zend_bool) midgard_object_delete_attachments(mobj, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_purge_attachments)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *params = NULL;
	zend_bool zbool = TRUE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &zbool, &params) != SUCCESS) {
		return;
	}

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params TSRMLS_CC);
	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	zend_bool rv = (zend_bool) midgard_object_purge_attachments(mobj, (gboolean) zbool, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	RETURN_BOOL(rv);
}

PHP_FUNCTION(_php_midgard_object_find_attachments)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &params) != SUCCESS) {
		return;
	}

	array_init(return_value);

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(params, &n_params TSRMLS_CC);
	MidgardObject *mobj = __midgard_object_get_ptr(getThis());

	MidgardObject **objects = midgard_object_find_attachments(mobj, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	if (objects) {
		php_midgard_array_from_objects((GObject **)objects, "midgard_attachment", return_value TSRMLS_CC);
		g_free(objects);
	}
}

PHP_FUNCTION(_php_midgard_object_create_attachment)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	const char *name = NULL, *title = NULL, *mimetype = NULL;
	int name_length, title_length, mimetype_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sss",
				&name, &name_length,
				&title, &title_length,
				&mimetype, &mimetype_length) != SUCCESS
	) {
		return;
	}

	MidgardObject *mobj = __midgard_object_get_ptr(getThis());
	MidgardObject *att = midgard_object_create_attachment(mobj, name, title, mimetype);

	if (!att) {
		RETURN_NULL();
	}

	char *type_name = (char *)G_OBJECT_TYPE_NAME((GObject*)att);
	zend_class_entry *ce = zend_fetch_class(type_name, strlen(type_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	php_midgard_gobject_new_with_gobject(return_value, ce, G_OBJECT(att), TRUE TSRMLS_CC);
}

/* It's not binded from core. This is PHP specific */
PHP_FUNCTION(_php_midgard_object_serve_attachment)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	const char *guid;
	int guid_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length) != SUCCESS) {
		return;
	}

	if (!midgard_is_guid(guid)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Given parameter is not a guid");
		return;
	}

	MidgardObject *att = NULL;

	{
		GValue gval = {0, };
		g_value_init(&gval, G_TYPE_STRING);
		g_value_set_string(&gval, guid);
		att = midgard_object_new(mgd, "midgard_attachment", &gval);
	}

	/* error is set by core */
	if (!att)
		return;

	MidgardBlob *blob = midgard_blob_new(att, NULL);

	if (!blob)
		return;

	{
		gchar *mimetype;
		g_object_get(G_OBJECT(att), "mimetype", &mimetype, NULL);

		gchar *content_type = g_strconcat("Content-type: ", mimetype, NULL);
		sapi_add_header(content_type, strlen(content_type), 1);
		g_free(content_type);
	}

	if (sapi_send_headers(TSRMLS_C) != SUCCESS)
		return;

	// FIXME: we should use streams here, instead
	const gchar *path = midgard_blob_get_path(blob);

	FILE *fp;
	if (!(fp = fopen(path, "r"))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "File doesn't exist");
		MIDGARD_ERRNO_SET(mgd, MGD_ERR_INTERNAL);
		return;
	}

	int b;
	char buf[1024];
	while ((b = fread(buf, 1, sizeof(buf), fp)) > 0) {
		PHPWRITE(buf, b);
	}

	fclose(fp);

	RETVAL_TRUE;;
}
