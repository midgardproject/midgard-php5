/*
 * Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of\
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "php_midgard.h"
#include "php_midgard_timestamp.h"
#include <zend_interfaces.h>
#include <date/php_date.h>
#include "php_midgard_gobject.h"

#include "php_midgard__helpers.h"

static zend_class_entry *php_midgard_datetime_class;

#define _GET_MIDGARD_DATE_OBJECT { \
	zval *zval_object = getThis(); \
	midgard_date_obj *intern = \
		(midgard_date_obj *)zend_object_store_get_object(zval_object TSRMLS_CC); \
	if (!intern) \
		php_error(E_ERROR, "Can not find underlying midgard_datetime object instance"); }

zval * get_UTC_timezone(TSRMLS_D)
{
	zval *timezone_object;
	zval *tz_str;

	MAKE_STD_ZVAL(tz_str);
	ZVAL_STRING(tz_str, "UTC", 1);

	MAKE_STD_ZVAL(timezone_object);
	object_init_ex(timezone_object, zend_date_timezone_class_ptr);

	zend_call_method_with_1_params(&timezone_object, zend_date_timezone_class_ptr, &zend_date_timezone_class_ptr->constructor, "__construct", NULL, tz_str);

	zval_ptr_dtor(&tz_str);

	return timezone_object;
}


/* Object constructor */
static PHP_METHOD(midgard_datetime, __construct)
{
	char *time_str = "now";
	int time_str_len;
	zval *self = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &time_str, &time_str_len) == FAILURE)
		return;

	zval *timezone_object = get_UTC_timezone(TSRMLS_C);

	/* Copy of time-string */
	zval *dt_str;
	MAKE_STD_ZVAL(dt_str);
	ZVAL_STRING(dt_str, time_str, 1);

	/* Call parent constructor - parent::__construct */
	zend_call_method_with_2_params(&self, zend_datetime_class_ptr, &zend_datetime_class_ptr->constructor, "__construct", NULL, dt_str, timezone_object);
	zval_ptr_dtor(&dt_str);
	zval_ptr_dtor(&timezone_object);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, date)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_datetime, setTimezone)
{
	php_error(E_WARNING, "midgard_datetime object already initialized for UTC timezone");
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime_settimezone, 0, 0, 1)
	ZEND_ARG_INFO(0, timezone)
ZEND_END_ARG_INFO()

static void _set_gobject_timestamp_property(zval *zobject)
{
	g_assert(zobject != NULL);

	zval *_object;
	zval *_propname;
	TSRMLS_FETCH();

	_object = zend_read_property(php_midgard_datetime_class, zobject, "object", sizeof("object")-1, 0 TSRMLS_CC);
	_propname = zend_read_property(php_midgard_datetime_class, zobject, "property", sizeof("property")-1, 0 TSRMLS_CC);

	if (Z_TYPE_P(_object) == IS_STRING && Z_STRLEN_P(_object) == 0) {
		return;
	}

	/* Find underlying GObject */
	php_midgard_gobject *php_gobject = __php_objstore_object(_object);
	GObject *gobject = NULL;

	if (php_gobject != NULL && php_gobject->gobject != NULL) {
		/* php_error(E_NOTICE, "GOBJECT %p", php_gobject->gobject); */
		if (!G_IS_OBJECT(php_gobject->gobject))
			php_error(E_WARNING, "Underlying GObject pointer is not a GObject type");
		else
			gobject = G_OBJECT(php_gobject->gobject);
	}

	/* Prepare DateTime::format argument */
	zval *fmt;
	MAKE_STD_ZVAL(fmt);
	ZVAL_STRING(fmt, "c", 1);

	/* Invoke Datetime::format */
	zval *_retval;
	zend_call_method_with_1_params(&zobject, Z_OBJCE_P(zobject), NULL, "format", &_retval, fmt);
	zval_ptr_dtor(&fmt);

	/* GObject is found, sets it property */
	if (gobject) {

		g_object_set(gobject, (const gchar *) Z_STRVAL_P(_propname), Z_STRVAL_P(_retval), NULL);
	}

	zval_dtor(_retval);

	return;
}

static PHP_METHOD(midgard_datetime, setDate)
{
	zval *this;
	zval *arg1, *arg2, *arg3;
	zval *retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	this = getThis();

	zend_call_method_with_3_params(&this, zend_datetime_class_ptr, (zend_function **)NULL, "setdate", &retval, arg1, arg2, arg3);

	_set_gobject_timestamp_property(this);

	RETURN_ZVAL(retval, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime_setdate, 0, 0, 3)
	ZEND_ARG_INFO(0, year)
	ZEND_ARG_INFO(0, month)
	ZEND_ARG_INFO(0, day)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_datetime, setTime)
{
	zval *this;
	zval *retval, *arg1, *arg2, *arg3 = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	this = getThis();

	if (arg3) {
		zend_call_method_with_3_params(&this, zend_datetime_class_ptr, (zend_function **)NULL, "settime", &retval, arg1, arg2, arg3);
	} else {
		zend_call_method_with_2_params(&this, zend_datetime_class_ptr, (zend_function **)NULL, "settime", &retval, arg1, arg2);
	}

	_set_gobject_timestamp_property(this);
	RETURN_ZVAL(retval, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime_settime, 0, 0, 2)
	ZEND_ARG_INFO(0, hour)
	ZEND_ARG_INFO(0, minute)
	ZEND_ARG_INFO(0, second)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_datetime, setISODate)
{
	zval *this;
	zval *retval, *arg1, *arg2, *arg3 = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	this = getThis();

	if (arg3) {
		zend_call_method_with_3_params(&this, zend_datetime_class_ptr, (zend_function **)NULL, "setisodate", &retval, arg1, arg2, arg3);
	} else {
		zend_call_method_with_2_params(&this, zend_datetime_class_ptr, (zend_function **)NULL, "setisodate", &retval, arg1, arg2);
	}

	_set_gobject_timestamp_property(this);
	RETURN_ZVAL(retval, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime_setisodate, 0, 0, 2)
	ZEND_ARG_INFO(0, year)
	ZEND_ARG_INFO(0, week)
	ZEND_ARG_INFO(0, day)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_datetime, modify)
{
	zval *this;
	zval *arg1, *retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg1) == FAILURE) {
		return;
	}

	this = getThis();

	zend_call_method_with_1_params(&this, zend_datetime_class_ptr, (zend_function **)NULL, "modify", &retval, arg1);

	_set_gobject_timestamp_property(this);

	RETURN_ZVAL(retval, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime_modify, 0, 0, 1)
	ZEND_ARG_INFO(0, date)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_datetime, __toString)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	/* Prepare DateTime::format argument */
	zval *fmt;
	MAKE_STD_ZVAL(fmt);
	ZVAL_STRING(fmt, "c", 0);

	/* Invoke Datetime::format */
	zval *zend_object = getThis();
	zval *strv = NULL;
	zend_call_method_with_1_params(&zend_object, Z_OBJCE_P(zend_object), NULL, "format", &strv, fmt);

	RETURN_ZVAL(strv, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_datetime___tostring, 0, 0, 0)
ZEND_END_ARG_INFO()

void php_midgard_datetime_init(int module_number)
{
	static function_entry midgard_datetime_methods[] = {
		PHP_ME(midgard_datetime,    __construct,	arginfo_midgard_datetime___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_datetime,    setTimezone,	arginfo_midgard_datetime_settimezone,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_datetime,    setTime,		arginfo_midgard_datetime_settime,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_datetime,    setDate,		arginfo_midgard_datetime_setdate,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_datetime,    setISODate,		arginfo_midgard_datetime_setisodate,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_datetime,    modify,		arginfo_midgard_datetime_modify,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_datetime,    __toString,		arginfo_midgard_datetime___tostring,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_datetime_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			php_midgard_datetime_class_entry,
			"midgard_datetime", midgard_datetime_methods);

	php_midgard_datetime_class =
		zend_register_internal_class_ex(
				&php_midgard_datetime_class_entry, zend_datetime_class_ptr, "DateTime" TSRMLS_CC);

	/* Register properties */
	zend_declare_property_string(php_midgard_datetime_class, "object", sizeof("object")-1, "", ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(php_midgard_datetime_class, "property", sizeof("property")-1, "", ZEND_ACC_PRIVATE TSRMLS_CC);
}


/* ROUTINES */
gboolean php_midgard_is_property_timestamp(MidgardDBObjectClass *klass, const gchar *property)
{
	g_assert(klass != NULL);
	g_assert(property != NULL);

	MidgardReflectionProperty *mrp =
		midgard_reflection_property_new(MIDGARD_DBOBJECT_CLASS(klass));
	GType midgard_type =
		midgard_reflection_property_get_midgard_type(mrp, property);

	g_object_unref(mrp);

	if (midgard_type == MGD_TYPE_TIMESTAMP)
		return TRUE;

	return FALSE;
}

zval *php_midgard_datetime_get_timestamp(zval *object)
{
	/* Prepare DateTime::format argument */
	zval *fmt;
	MAKE_STD_ZVAL(fmt);
	ZVAL_STRING(fmt, "c", 0);

	/* Invoke Datetime::format */
	zval *_retval;
	TSRMLS_FETCH();
	zend_call_method_with_1_params(&object, Z_OBJCE_P(object), NULL, "format", &_retval, fmt);

	return _retval;
}

void php_midgard_datetime_from_gvalue(const GValue *gval, zval *zvalue)
{
	g_assert(gval != NULL);
	g_assert(zvalue != NULL);
	g_return_if_fail(G_VALUE_HOLDS(gval, MGD_TYPE_TIMESTAMP));

	GValue str_val = {0, };
	g_value_init(&str_val, G_TYPE_STRING);
	g_value_transform(gval, &str_val);
	const gchar *timestamp = g_value_get_string(&str_val);
	TSRMLS_FETCH();

	if (timestamp == NULL)
		timestamp = g_strdup(MIDGARD_DEFAULT_DATETIME);

	zval *date;
	MAKE_STD_ZVAL(date);
	ZVAL_STRING(date, (gchar *)timestamp, 1);

	g_value_unset(&str_val);

	if (zvalue == NULL)
		MAKE_STD_ZVAL(zvalue);

	object_init_ex(zvalue, php_midgard_datetime_class);

	zend_call_method_with_1_params(&zvalue, php_midgard_datetime_class, &php_midgard_datetime_class->constructor, "__construct", NULL, date);
	zval_ptr_dtor(&date);

	return;
}

zval *php_midgard_datetime_object_from_property(zval *object, const gchar *property)
{
	g_assert(object != NULL);
	g_assert(property != NULL);

	const gchar *timestamp;
	TSRMLS_FETCH();

	GObject *gobject = G_OBJECT(__php_gobject_ptr(object));

	if (gobject == NULL) {
		php_error(E_ERROR, "Can not find underlying GObject for given %s zend object", Z_OBJCE_P(object)->name);
	}

	GValue tprop = {0, };
	g_value_init(&tprop, MGD_TYPE_TIMESTAMP);

	g_object_get_property(gobject, property, &tprop);

	GValue str_val = {0, };
	g_value_init(&str_val, G_TYPE_STRING);
	g_value_transform(&tprop, &str_val);
	timestamp = g_value_get_string(&str_val);

	if (timestamp == NULL)
		timestamp = g_strdup(MIDGARD_DEFAULT_DATETIME);

	zval *date;
	MAKE_STD_ZVAL(date);
	ZVAL_STRING(date, (gchar *)timestamp, 1);

	g_value_unset(&str_val);
	g_value_unset(&tprop);

	zval *mdate_object;
	MAKE_STD_ZVAL(mdate_object);
	object_init_ex(mdate_object, php_midgard_datetime_class);

	zend_call_method_with_1_params(&mdate_object, php_midgard_datetime_class, &php_midgard_datetime_class->constructor, "__construct", NULL, date);
	zval_ptr_dtor(&date);

	zend_update_property_string(php_midgard_datetime_class, mdate_object, "property", sizeof("property")-1, (gchar *)property TSRMLS_CC);
	zend_update_property(php_midgard_datetime_class, mdate_object, "object", sizeof("object")-1, object TSRMLS_CC);

	return mdate_object;
}
