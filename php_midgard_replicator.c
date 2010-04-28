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

static MidgardObject *_get_object(zval *object)
{
	TSRMLS_FETCH();
	return MIDGARD_OBJECT(__php_gobject_ptr(object));
}

static PHP_METHOD(midgard_replicator, serialize)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *xml;
	zval *zobject;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;
	
	GObject *object = G_OBJECT(__php_gobject_ptr(zobject));
	xml = midgard_replicator_serialize(object);

	if (xml == NULL)
		RETURN_NULL();

	/* We have to duplicate xml string because Zend seems to ... */

	RETVAL_STRING(xml, 1);

	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_serialize, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, object, midgard_dbobject, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gboolean exported;
	zval *zobject;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;
	
	MidgardObject *object = _get_object(zobject);
	exported = midgard_replicator_export(MIDGARD_DBOBJECT(object));

	RETURN_BOOL(exported);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, object, midgard_dbobject, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export_by_guid)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *guid;
	guint guid_length;
	gboolean rv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &guid, &guid_length) == FAILURE) 
		return;

	rv = midgard_replicator_export_by_guid(mgd_handle(), (const gchar *) guid);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_export_by_guid, 0, 0, 1)
	ZEND_ARG_INFO(0, guid)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, export_purged)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *startdate = NULL , *enddate = NULL , *xml = NULL;
	guint start_length, end_length;
	zval *ook; /* Object Or Klass name zval */
	MidgardObjectClass *klass = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|ss",
				&ook, &startdate, &start_length, &enddate, &end_length) == FAILURE)
	{
		return;
	}

	if ((Z_TYPE_P(ook) != IS_STRING) && Z_TYPE_P(ook) != IS_OBJECT)
	{
		php_error(E_WARNING,
				"%s() accepts object or string as first argument",
				get_active_function_name(TSRMLS_C));
		return;
	}

	const gchar *classname;

	if (Z_TYPE_P(ook) == IS_STRING) {
		classname = (const gchar *)Z_STRVAL_P(ook);
	} else if (Z_TYPE_P(ook) == IS_OBJECT) {
		classname = (const gchar *)Z_OBJCE_P(ook)->name;
	}

	klass = MIDGARD_OBJECT_GET_CLASS_BY_NAME(classname);

	if (!klass) {
		php_error(E_WARNING, "MidgardObjectClass not found");
		return;
	}

	xml = midgard_replicator_export_purged(mgd_handle(), classname, startdate, enddate);

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

static PHP_METHOD(midgard_replicator, serialize_blob)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *xml;
	zval *zobject;	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &zobject) == FAILURE) 
		return;

	MidgardObject *object = _get_object(zobject);
	xml = midgard_replicator_serialize_blob(object);

	if (xml == NULL)
		RETURN_NULL();

	RETVAL_TRUE;
	RETVAL_STRING(xml, 1);
	g_free(xml);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_serialize_blob, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, object, midgard_attachment, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, unserialize)
{
	RETVAL_FALSE;
	CHECK_MGD;

	char *xml;
	long xml_length;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &xml, &xml_length, &zbool) == FAILURE) 
		return;

	array_init(return_value);

	GObject **objects = midgard_replicator_unserialize(mgd_handle(), (const gchar *)xml , (gboolean) zbool);

	if (!objects)
		RETURN_FALSE;

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
	CHECK_MGD;

	gboolean imported;
	zval *zobject;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o|b", &zobject, &zbool) == FAILURE) 
		return;

	MidgardObject *object = _get_object(zobject);
	imported = midgard_replicator_import_object(MIDGARD_DBOBJECT(object), zbool);

	RETURN_BOOL(imported);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_import_object, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_replicator, import_from_xml)
{
	RETVAL_FALSE;
	CHECK_MGD;

	gchar *xml;
	guint xml_length;
	zend_bool zbool = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &xml, &xml_length, &zbool) == FAILURE) 
		return;

	midgard_replicator_import_from_xml(mgd_handle(), (const gchar *)xml, zbool);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_replicator_import_from_xml, 0, 0, 1)
	ZEND_ARG_INFO(0, xml)
	ZEND_ARG_INFO(0, force)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_replicator)
{
	static function_entry replicator_methods[] = {
		PHP_ME(midgard_replicator, serialize,       arginfo_midgard_replicator_serialize,       ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export,          arginfo_midgard_replicator_export,          ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export_by_guid,  arginfo_midgard_replicator_export_by_guid,  ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, export_purged,   arginfo_midgard_replicator_export_purged,   ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, serialize_blob,  arginfo_midgard_replicator_serialize_blob,  ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, unserialize,     arginfo_midgard_replicator_unserialize,     ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, import_object,   arginfo_midgard_replicator_import_object,   ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		PHP_ME(midgard_replicator, import_from_xml, arginfo_midgard_replicator_import_from_xml, ZEND_ACC_STATIC|ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_replicator_class_entry;
	INIT_CLASS_ENTRY(php_midgard_replicator_class_entry, "midgard_replicator", replicator_methods);

	php_midgard_replicator_class = zend_register_internal_class(&php_midgard_replicator_class_entry TSRMLS_CC);

	return SUCCESS;
}
