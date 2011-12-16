/* Copyright (C) 2007, 2008 Piotr Pokora <piotrek.pokora@gmail.com>
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

#include <midgard/midgard_blob.h>
#include <zend_exceptions.h>

static zend_class_entry *php_midgard_blob_class;

#define _GET_BLOB_OBJECT \
	zval *zval_object = getThis(); \
	MidgardBlob *blob = MIDGARD_BLOB(__php_gobject_ptr(zval_object)); \
	if (!blob) \
		php_error(E_ERROR, "Can not find underlying blob instance");

static PHP_METHOD(midgard_blob, __construct)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	MidgardBlob *blob = NULL;
	zval *param_object = NULL;
	zval *zval_object = getThis();
	char *encoding = NULL;
	int encoding_length;

	zend_class_entry *midgard_attachment_ce = zend_fetch_class("midgard_attachment", strlen("midgard_attachment"), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|s", &param_object, midgard_attachment_ce, &encoding, &encoding_length) == FAILURE)
		return;

	php_midgard_gobject *ppo = __php_objstore_object(param_object);

	if (!ppo) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC, "Attachment seems to be uninitialized");
		return;
	}

	MidgardObject *att = MIDGARD_OBJECT(ppo->gobject);
	blob = midgard_blob_new(att, encoding);

	if (!blob) {
		php_midgard_error_exception_throw(mgd TSRMLS_CC);
		return;
	}

	MGD_PHP_SET_GOBJECT(zval_object, blob);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_blob___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, attachment, midgard_attachment, 0)
	ZEND_ARG_INFO(0, encoding)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_blob, read_content)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	gchar *content;
	gsize bytes_read = 0;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_BLOB_OBJECT;

	content = midgard_blob_read_content(blob, &bytes_read);

	if (content == NULL) {
		php_error(E_WARNING, "Failed to get blob-contents: %s", mgd->errstr);
		RETURN_NULL();
	}

	RETURN_STRINGL(content, bytes_read, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_blob_read_content, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_blob, write_content)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *content;
	int content_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_length) == FAILURE)
		return;

	_GET_BLOB_OBJECT;

	gboolean rv = midgard_blob_write_content(blob, (const gchar *)content);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_blob_write_content, 0, 0, 1)
	ZEND_ARG_INFO(0, content)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_blob, remove_file)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_BLOB_OBJECT;

	gboolean rv = midgard_blob_remove_file(blob);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_blob_remove_file, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_blob, get_handler)
{
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *mode = "w";
	int mode_length = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &mode, &mode_length) == FAILURE)
		return;

	_GET_BLOB_OBJECT;

	const gchar *path = midgard_blob_get_path(blob);
	/* php_streams require paths so no filehandler or channel here */
	/* GIOChannel *channel = midgard_blob_get_handler(blob); */

	php_stream *stream = php_stream_open_wrapper_ex(
		(char *)path,
		mode,
		IGNORE_PATH | IGNORE_URL | STREAM_DISABLE_OPEN_BASEDIR,
		NULL, NULL
	);

	if (stream == NULL) {
		php_error_docref1(NULL TSRMLS_CC, mode, E_WARNING, "Failed to open stream. Might be permissions issue");
		return;
	}

	php_stream_to_zval(stream, return_value);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_blob_get_handler, 0, 0, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_blob, get_path)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_BLOB_OBJECT;

	const gchar *path = midgard_blob_get_path(blob);

	if (!path)
		RETURN_NULL();

	RETURN_STRING((gchar *)path, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_blob_get_path, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_blob, exists)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_BLOB_OBJECT;

	RETURN_BOOL(midgard_blob_exists(blob));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_blob_exists, 0)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_blob)
{
	static zend_function_entry blob_methods[] = {
		PHP_ME(midgard_blob, __construct,   arginfo_midgard_blob___construct,   ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_blob, read_content,  arginfo_midgard_blob_read_content,  ZEND_ACC_PUBLIC)
		PHP_ME(midgard_blob, write_content, arginfo_midgard_blob_write_content, ZEND_ACC_PUBLIC)
		PHP_ME(midgard_blob, remove_file,   arginfo_midgard_blob_remove_file,   ZEND_ACC_PUBLIC)
		PHP_ME(midgard_blob, get_handler,   arginfo_midgard_blob_get_handler,   ZEND_ACC_PUBLIC)
		PHP_ME(midgard_blob, get_path,      arginfo_midgard_blob_get_path,      ZEND_ACC_PUBLIC)
		PHP_ME(midgard_blob, exists,        arginfo_midgard_blob_exists,        ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_blob_class_entry;
	INIT_CLASS_ENTRY(php_midgard_blob_class_entry, "MidgardBlob", blob_methods);

	php_midgard_blob_class = zend_register_internal_class(&php_midgard_blob_class_entry TSRMLS_CC);
	php_midgard_blob_class->create_object = php_midgard_gobject_new;
	php_midgard_blob_class->doc_comment = strdup("Wrapper around midgard_attachment object, which provides high-level API for working with larget binary entities");

	zend_register_class_alias("midgard_blob", php_midgard_blob_class);

	return SUCCESS;
}
