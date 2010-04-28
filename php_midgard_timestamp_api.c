#include "php_midgard.h"
#include "php_midgard_gobject.h"

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