/* Copyright (C) 2006 Piotr Pokora <piotrek.pokora@gmail.com>
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

static zend_class_entry *php_midgard_replicator_class;

static PHP_METHOD(midgard_replicator, export)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zobject;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;

	gboolean exported = midgard_replicator_export(__midgard_dbobject_get_ptr(zobject));
	RETURN_BOOL(exported);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, object, midgard_dbobject, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export_by_guid)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *guid;
	int guid_length;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length) == FAILURE) 
		return;

	gboolean rv = midgard_replicator_export_by_guid(mgd, (const gchar *) guid);
	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export_by_guid, 0, 0, 1)
	ZEND_ARG_INFO(0, guid)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export_purged)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *startdate = NULL, *enddate = NULL;
	int start_length, end_length;
	zval *ook; /* Object Or Klass name zval */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ss", &ook, &startdate, &start_length, &enddate, &end_length) == FAILURE) {
		return;
	}

	const gchar *classname;

	if (Z_TYPE_P(ook) == IS_STRING) {
		classname = (const gchar *)Z_STRVAL_P(ook);
	} else if (Z_TYPE_P(ook) == IS_OBJECT) {
		classname = (const gchar *)Z_OBJCE_P(ook)->name;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "first parameter should be object or string");
		return;
	}

	MidgardObjectClass *klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME(classname);

	if (!klass) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s class is not found", classname);
		return;
	}

	char *xml = midgard_replicator_export_purged(mgd, classname, startdate, enddate);

	if (xml == NULL)
		RETURN_NULL();

	RETVAL_STRING(xml, 1);
	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export_purged, 0, 0, 1)
	ZEND_ARG_INFO(0, class)
	ZEND_ARG_INFO(0, startdate)
	ZEND_ARG_INFO(0, enddate)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, serialize)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	gchar *xml;
	zval *zobject;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;

	GObject *object = __php_gobject_ptr(zobject);
	xml = midgard_replicator_serialize(object);

	if (xml == NULL)
		RETURN_NULL();

	RETVAL_STRING(xml, 1);

	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_serialize, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, object, midgard_dbobject, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, serialize_blob)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zobject;

	zend_class_entry *midgard_attachment_ce = zend_fetch_class("midgard_attachment", strlen("midgard_attachment"), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zobject, midgard_attachment_ce) == FAILURE)
		return;

	gchar *xml = midgard_replicator_serialize_blob(__midgard_object_get_ptr(zobject));

	if (xml == NULL)
		RETURN_NULL();

	RETVAL_STRING(xml, 1);
	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_serialize_blob, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, object, midgard_attachment, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, unserialize)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *xml = NULL;
	int xml_length;
	zend_bool force = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &xml, &xml_length, &force) == FAILURE)
		return;

	array_init(return_value);

	GObject **objects = midgard_replicator_unserialize(mgd, (const gchar *)xml, (gboolean)force);

	if (!objects)
		return;

	size_t i = 0;
	zend_class_entry *ce;

	while (objects[i] != NULL) {
		zval *zobject;
		MAKE_STD_ZVAL(zobject);

		char *class_name = (char *)G_OBJECT_TYPE_NAME(G_OBJECT(objects[i]));
		ce = zend_fetch_class(class_name, strlen(class_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

		object_init_ex(zobject, ce);
		MGD_PHP_SET_GOBJECT(zobject, objects[i]);
		zend_call_method_with_0_params(&zobject, ce, &ce->constructor, "__construct", NULL);

		zend_hash_next_index_insert(HASH_OF(return_value), &zobject, sizeof(zval *), NULL);

		i++;
	};

	g_free(objects);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_unserialize, 0, 0, 1)
	ZEND_ARG_INFO(0, xml)
	ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, import_object)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	zval *zobject;
	zend_bool force = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|b", &zobject, &force) == FAILURE)
		return;

	gboolean imported = midgard_replicator_import_object(__midgard_dbobject_get_ptr(zobject), force);

	RETURN_BOOL(imported);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_import_object, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, import_from_xml)
{
	RETVAL_FALSE;
	MidgardConnection *mgd = mgd_handle(TSRMLS_C);
	CHECK_MGD(mgd);

	char *xml = NULL;
	int xml_length;
	zend_bool force = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &xml, &xml_length, &force) == FAILURE)
		return;

	midgard_replicator_import_from_xml(mgd, (const gchar *)xml, force);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_import_from_xml, 0, 0, 1)
	ZEND_ARG_INFO(0, xml)
	ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_replicator)
{
	static function_entry replicator_methods[] = {
		PHP_ME(midgard_replicator, export,          arginfo_midgard_replicator_export,          ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export_by_guid,  arginfo_midgard_replicator_export_by_guid,  ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export_purged,   arginfo_midgard_replicator_export_purged,   ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, serialize,       arginfo_midgard_replicator_serialize,       ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, serialize_blob,  arginfo_midgard_replicator_serialize_blob,  ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, unserialize,     arginfo_midgard_replicator_unserialize,     ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, import_object,   arginfo_midgard_replicator_import_object,   ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, import_from_xml, arginfo_midgard_replicator_import_from_xml, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_replicator_class_entry;
	INIT_CLASS_ENTRY(php_midgard_replicator_class_entry, "midgard_replicator", replicator_methods);

	php_midgard_replicator_class = zend_register_internal_class(&php_midgard_replicator_class_entry TSRMLS_CC);
	php_midgard_replicator_class->doc_comment = strdup("Collection of static methods for serializing, unserializing data to XML; importing and exporting it");

	return SUCCESS;
}
