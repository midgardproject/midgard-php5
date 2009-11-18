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
#include <TSRM.h>
#include <zend_interfaces.h>
#include <locale.h>
#include "php_midgard_timestamp.h"

#include "php_midgard__helpers.h"

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION > 5
# define MGD_IS_CALLABLE(a, b, c) zend_is_callable(a, b, c TSRMLS_CC)
#else
# define MGD_IS_CALLABLE(a, b, c) zend_is_callable(a, b, c)
#endif

/* static prototypes */

static void __php_midgard_closure_free(gpointer data);

static void php_midgard_closure_default_marshal(GClosure *closure,
		GValue *return_value, guint n_param_values,
		const GValue *param_values, gpointer invocation_hint,
		gpointer marshal_data);
static GClosure *php_midgard_closure_new_default(zend_fcall_info fci, zend_fcall_info_cache fci_cache, zval *zobject, zval *zval_array TSRMLS_DC);

php_midgard_gobject *php_midgard_zend_object_store_get_object(zval *zobject TSRMLS_DC);

/* GVALUE ROUTINES */

static zend_bool php_midgard_gvalue_from_zval(zval *zvalue, GValue *gvalue)
{
	g_assert(zvalue != NULL);

	HashTable *zhash;
	GValueArray *array;
	zval **value;
	HashPosition iterator;
	GValue *tmpval;
	zval*date_zval;
	gchar *lstring;
	TSRMLS_FETCH();

	switch (Z_TYPE_P(zvalue)) {
		case IS_ARRAY:
			zhash = Z_ARRVAL_P(zvalue);
			array =	g_value_array_new(zend_hash_num_elements(zhash));

			zend_hash_internal_pointer_reset_ex(zhash, &iterator);
			while (zend_hash_get_current_data_ex(
						zhash, (void **)&value, &iterator) == SUCCESS)
			{

				tmpval = php_midgard_zval2gvalue(*value);
				g_value_array_append(array, tmpval);
				g_value_unset(tmpval);
				g_free(tmpval);
				zend_hash_move_forward_ex(zhash, &iterator);
			}

			g_value_init(gvalue, G_TYPE_VALUE_ARRAY);
			g_value_take_boxed(gvalue, array);
			break;

		case IS_BOOL:
			g_value_init(gvalue, G_TYPE_BOOLEAN);
			g_value_set_boolean(gvalue, Z_BVAL_P(zvalue));
			break;

		case IS_LONG:
			g_value_init(gvalue, G_TYPE_INT);
			g_value_set_int(gvalue, Z_LVAL_P(zvalue));
			break;

		case IS_DOUBLE:
			g_value_init(gvalue, G_TYPE_FLOAT);
			lstring = setlocale(LC_NUMERIC, "0");
			setlocale(LC_NUMERIC, "C");
			g_value_set_float(gvalue, (gfloat)Z_DVAL_P(zvalue));
			setlocale(LC_ALL, lstring);
			break;

		case IS_STRING:
			g_value_init(gvalue, G_TYPE_STRING);
			g_value_set_string(gvalue, Z_STRVAL_P(zvalue));
			break;

		case IS_OBJECT:
			/* DateTime object, convert to string value */
			if (Z_OBJCE_P(zvalue) == zend_datetime_class_ptr
					|| Z_OBJCE_P(zvalue)->parent == zend_datetime_class_ptr)
			{
				date_zval = php_midgard_datetime_get_timestamp(zvalue);
				GValue str_val = {0, };
				g_value_init(&str_val, G_TYPE_STRING);
				g_value_set_string(&str_val, Z_STRVAL_P(date_zval));

				g_value_init(gvalue, MGD_TYPE_TIMESTAMP);
				g_value_transform(&str_val, gvalue);
				g_value_unset(&str_val);
				zval_dtor(date_zval);
			} else {
				php_midgard_gobject *php_gobject = php_midgard_zend_object_store_get_object(zvalue TSRMLS_CC);

				if (php_gobject && php_gobject->magic == PHP_MIDGARD_GOBJ_MAGIC && php_gobject->gobject) {
					if (!G_IS_OBJECT(php_gobject->gobject)) {
						g_warning("zval2gvalue conversion failed");
						return FALSE;
					}

					GObject *gobject = G_OBJECT(php_gobject->gobject);

					g_value_init(gvalue, G_TYPE_OBJECT);
					g_value_set_object(gvalue, gobject);
				} else {
					// NOT GOBJECT. Don't know what to do with it
					return FALSE;
				}
			}
			break;

		case IS_RESOURCE:
			/* There's no way to handle resource gracefully */
			return FALSE;
			break;

		default:
			/* FIXME, we can not fallback to string type */
			convert_to_string(zvalue);
			g_value_init(gvalue, G_TYPE_STRING);
			g_value_set_string(gvalue, Z_STRVAL_P(zvalue));
	}

	return TRUE;
}

GValue *php_midgard_zval2gvalue(zval *zvalue)
{
	g_assert(zvalue != NULL);

	GValue *gvalue = g_new0(GValue, 1);

	if (!php_midgard_gvalue_from_zval(zvalue, gvalue))
		return NULL;

	return gvalue;
}

zend_bool php_midgard_gvalue2zval(GValue *gvalue, zval *zvalue)
{
	g_assert(gvalue);
	g_assert(zvalue);

	gchar *tmpstr;
	double f, dpval, tmp_val;
	const gchar *gclass_name;
	GValueArray *array;
	GValue *arr_val;
	zval *zarr_val;

	/* Generic GValue */
	switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(gvalue))) {
		case G_TYPE_STRING:
			tmpstr = (gchar *)g_value_get_string(gvalue);
			if (!tmpstr)
				tmpstr = "";
			ZVAL_STRING(zvalue, tmpstr, 1);
			return TRUE;
			break;

		case G_TYPE_INT:
			ZVAL_LONG(zvalue, g_value_get_int(gvalue));
			return TRUE;
			break;

		case G_TYPE_UINT:
			ZVAL_LONG(zvalue, g_value_get_uint(gvalue));
			return TRUE;
			break;

		case G_TYPE_BOOLEAN:
			ZVAL_BOOL(zvalue, g_value_get_boolean(gvalue));
			return TRUE;
			break;

		case G_TYPE_FLOAT:
			/* I follow code from PHP_ROUND_WITH_FUZZ macro.
			 * If You find better way to add double property value,
			 * fell free to change this code. We do not have to worry
			 * about locale settings at this point ( I hope so ) */
			dpval = (gdouble)g_value_get_float(gvalue);
			tmp_val = dpval;

			f = pow(10.0, (double) 6);
			tmp_val *= f;

			if (tmp_val >= 0.0) {
				tmp_val = floor(tmp_val + 0.50000000001);
			} else {
				tmp_val = ceil(tmp_val - 0.50000000001);
			}

			tmp_val /= f;
			dpval = !zend_isnan(tmp_val) ? tmp_val : dpval;

			ZVAL_DOUBLE(zvalue, dpval);
			return TRUE;
			break;

		case G_TYPE_OBJECT:
			{
				GObject *gobject_property = g_value_get_object(gvalue);

				if (gobject_property) {
					gclass_name = G_OBJECT_TYPE_NAME(gobject_property);

					if (!gclass_name)
						return FALSE;

					/* TODO , check zval ref and alloc */
					php_midgard_gobject_init(zvalue, gclass_name, gobject_property, TRUE);

					return TRUE;
				} else {
					ZVAL_NULL(zvalue);
					/* TODO, implement this, currently we do not
					 * handle such case */
					/*zvalue->value.obj =
						php_midgard_gobject_new(*ce TSRMLS_CC);
						//zend_objects_new(&zobject, *ce TSRMLS_CC);
					Z_OBJ_HT_P(zvalue) = &php_midgard_gobject_handlers;
					zend_object_std_init(zobject, *ce TSRMLS_CC); */
				}
			}

			break;

		case G_TYPE_BOXED:
			if (G_VALUE_TYPE(gvalue) == G_TYPE_VALUE_ARRAY) {
				array_init(zvalue);

				array = (GValueArray *) g_value_get_boxed(gvalue);
				if (array == NULL)
					return TRUE;

				guint i;
				for (i = 0; i < array->n_values; i++) {
					arr_val = g_value_array_get_nth(array, i);
					MAKE_STD_ZVAL(zarr_val);
					php_midgard_gvalue2zval(arr_val, zarr_val);
					add_index_zval(zvalue, i, zarr_val);
				}
			}

			if (G_VALUE_TYPE(gvalue) == MGD_TYPE_TIMESTAMP) {
				php_midgard_datetime_from_gvalue(gvalue, zvalue);
				return TRUE;
			}

			break;

		default:
			ZVAL_NULL(zvalue);
			break;
	}

	return FALSE;
}

/* OBJECTS ROUTINES */

/* getting object-definition struct from zval* */
php_midgard_gobject *php_midgard_zend_object_store_get_object(zval *zobject TSRMLS_DC)
{
	php_midgard_gobject *php_gobject = (php_midgard_gobject *) zend_object_store_get_object(zobject TSRMLS_CC);

	if (!php_gobject)
		return NULL;

	/* Important! We must ensure we have underlying GObject always! */
	/* Failed code case: custom construstor wich does not invoke parent's one */
	/* Or any other behaviour which makes underlying gobject
	 * not being initialized */

	if (php_gobject->gobject)
		return php_gobject;

	php_error(E_NOTICE, "Invalid object's constructor (%s)", Z_OBJCE_P(zobject)->name);

	zend_class_entry *ce = Z_OBJCE_P(zobject);
	while (ce->type != ZEND_INTERNAL_CLASS && ce->parent != NULL) {
		ce = ce->parent;
	}

	guint classname_length = strlen(ce->name);
	gchar *_classname = g_ascii_strdown(ce->name, classname_length);

	GType class_type = g_type_from_name((const gchar *) _classname);

	if (g_type_parent(class_type) == MIDGARD_TYPE_OBJECT) {
		/* php_error(E_NOTICE, "IMPLICIT %s CONSTRUCTOR", _classname); */
		php_gobject->gobject = (GObject *)midgard_object_new(mgd_handle(), (const gchar *) _classname, NULL);
	} else {
		g_warning("Creating new underlying '%s' GObject. Missed constructor?", g_type_name(class_type));
		php_gobject->gobject = g_object_new(class_type, NULL);
	}

	g_free(_classname);

	return php_gobject;
}

/* check if there is glib-property of object linked to zval */
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int type TSRMLS_DC)
{
	php_midgard_gobject *php_gobject = php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);
	GObject *gobject = G_OBJECT(php_gobject->gobject);
	char *prop_name = Z_STRVAL_P(prop);

	if (prop_name == NULL) {
		g_warning("Can not check property with NULL name");
		return 0;
	}

	if (g_str_equal(prop_name, "")) {
		g_warning("Can not check property with empty name");
		return 0;
	}

	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(gobject), prop_name);

	if (!pspec) {
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		return std_hnd->has_property(zobject, prop, BP_VAR_NA TSRMLS_CC);
	}

	if (type == 2) {
		return 1;
	}

	GValue pval = {0, };
	g_value_init(&pval, pspec->value_type);
	g_object_get_property(gobject, prop_name, &pval);

	int _retval = 0;

	switch (pspec->value_type) {
		case G_TYPE_STRING:
		{
			const gchar *emptystr = g_value_get_string(&pval);

			_retval = 1;
			if (!emptystr || *emptystr == '\0' || (*emptystr == '0' && *++emptystr == '\0'))
				_retval = 0;
		}
			break;

		case G_TYPE_INT:
			_retval = (g_value_get_int(&pval) != 0);
			break;

		case G_TYPE_UINT:
			_retval = (g_value_get_uint(&pval) != 0);
			break;

		case G_TYPE_OBJECT:
			_retval = (NULL != g_value_get_object(&pval));
			break;

		default:
			_retval = 1;
			break;
	}

	g_value_unset(&pval);
	return _retval;
}

/* Read Zend object property using underlying GObject's one */
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type TSRMLS_DC)
{
	zval *_retval = NULL;
	zend_object_handlers *std_hnd;
	gboolean is_native_property = FALSE;
	gboolean is_datetime_property = FALSE;
	GParamSpec *pspec = NULL;
	GObjectClass *klass = NULL;

	const gchar *propname = Z_STRVAL_P(prop);

	if (propname == NULL || *propname == '\0')
		php_error(E_ERROR, "Can not read empty property name");

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_gobject_read_property(%s)\n", zobject, propname);
	}

	php_midgard_gobject *php_gobject = php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);

	GObject *gobject = G_OBJECT(php_gobject->gobject);

	if (gobject && Z_STRVAL_P(prop) != NULL) {
		klass = G_OBJECT_GET_CLASS(gobject);

		/* Find GObject property */
		if (G_IS_OBJECT_CLASS(klass)) {
			const gchar *propname = Z_STRVAL_P(prop);

			pspec =	g_object_class_find_property(klass, propname);

			if (pspec != NULL) {
				is_native_property = TRUE;
			}
		}
	}

	/* If found, get property's gvalue. Create zval from it and return */
	if (is_native_property) {
		if (MIDGARD_IS_DBOBJECT_CLASS(klass)) {
			is_datetime_property = php_midgard_is_property_timestamp(MIDGARD_DBOBJECT_CLASS(klass), propname);
		}

		/* Property of object type. $obj->metadata for example. */
		if (pspec->value_type == G_TYPE_OBJECT) {
			if (MGDG(midgard_memory_debug)) {
				php_printf("==========> G_TYPE_OBJECT\n");
			}

			zval **property;
			if (zend_hash_find(Z_OBJPROP_P(zobject), Z_STRVAL_P(prop), Z_STRLEN_P(prop)+1 ,(void **) &property) == SUCCESS) {
				_retval = *property;
				// zval_add_ref(property);
			} else {
				MAKE_STD_ZVAL(_retval);
				ZVAL_NULL(_retval);
				return _retval;
			}
		/* Datetime property. $obj->metadata->created for example. */
		} else if (is_datetime_property) {
			if (MGDG(midgard_memory_debug)) {
				php_printf("==========> DateTime\n");
			}

			zval **dtp;
			if (zend_hash_find(Z_OBJPROP_P(zobject), Z_STRVAL_P(prop), Z_STRLEN_P(prop)+1 ,(void **) &dtp) == SUCCESS
				&& Z_TYPE_PP(dtp) == IS_OBJECT)
			{
				if (Z_REFCOUNT_P(*dtp) < 2) {
					/* FIXME, property value (object) seems to be duplicated here */
					_retval = php_midgard_datetime_object_from_property(zobject, Z_STRVAL_P(prop));
					zend_hash_update(Z_OBJPROP_P(zobject), (gchar *) propname, strlen(propname)+1, (void *)&_retval, sizeof(zval *), NULL);
					Z_ADDREF_P(*dtp);
				} else {
					_retval = *dtp;
				}
			} else {
				_retval = php_midgard_datetime_object_from_property(zobject, propname);
				zend_hash_update(Z_OBJPROP_P(zobject), (gchar *) propname, strlen(propname)+1, (void *)&_retval, sizeof(zval *), NULL);
			}
		/* Property of generic type. String, int, float, etc */
		} else {
			if (MGDG(midgard_memory_debug)) {
				php_printf("==========> scalar\n");
			}

			GValue pval = {0, };
			g_value_init(&pval, pspec->value_type);
			g_object_get_property(G_OBJECT(gobject), Z_STRVAL_P(prop), &pval);

			MAKE_STD_ZVAL(_retval);
			php_midgard_gvalue2zval(&pval, _retval);
			Z_DELREF_P(_retval); // we don't have local reference, so need to decrement refcount

			g_value_unset(&pval);	

			if (MGDG(midgard_memory_debug)) {
				php_printf("[%p] property's tmp-var refcount: %d [%s]\n", zobject, Z_REFCOUNT_P(_retval), propname);
			}
		}
	/* Fallback to zend. */
	} else {
		/* Property is not found. Fallback to zend's property handler
		 * Piotras: I have no idea what type should be passed instead
		 * of BP_VAR_NA. The point is to throw warning when property
		 * is not registered for (sub)class. */
		std_hnd = zend_get_std_object_handlers();
		_retval = std_hnd->read_property(zobject, prop, BP_VAR_NA TSRMLS_CC);
	}

	return _retval;
}

zval **php_midgard_gobject_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC)
{
	if (MGDG(midgard_memory_debug)) {
		const gchar *propname = Z_STRVAL_P(member);
		php_printf("[%p] php_midgard_gobject_get_property_ptr_ptr(%s)\n", object, propname);
	}

	// we don't want to provide direct access to underlying properties
	return NULL;
}

/* convert zval to type described by glib-type */
static void _convert_value(zval *value, GType vtype)
{
	switch (vtype) {
		case G_TYPE_STRING:
			if (Z_TYPE_P(value) != IS_STRING)
				convert_to_string(value);
			break;

		case G_TYPE_UINT:
		case G_TYPE_INT:
			if (Z_TYPE_P(value) != IS_LONG)
				convert_to_long(value);
			break;

		case G_TYPE_BOOLEAN:
			if (Z_TYPE_P(value) != IS_BOOL)
				convert_to_boolean(value);
			break;

		case G_TYPE_FLOAT:
			if (Z_TYPE_P(value) != IS_DOUBLE)
				convert_to_double(value);
			break;

		case G_TYPE_OBJECT:
			if (Z_TYPE_P(value) == IS_NULL)
				return;
			//if (Z_TYPE_P(value) != IS_OBJECT)
			//	convert_to_object(value);
			break;
	}

	return;
}

void php_midgard_gobject_write_property(zval *zobject, zval *prop, zval *value TSRMLS_DC)
{
	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_gobject_write_property()\n", zobject);
	}

	zend_object_handlers *std_hnd = zend_get_std_object_handlers();
	php_midgard_gobject *php_gobject = php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);
	GObject *gobject = G_OBJECT(php_gobject->gobject);

	/* Find GObject property */
	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(gobject), Z_STRVAL_P(prop));

	/* If found, set property's gvalue.*/
	if (pspec) {
		/* Property type might be initialized with IS_STRING or unset type.
		 * Check property's type and convert if needed. */
		_convert_value(value, pspec->value_type);

		GValue *gvalue = php_midgard_zval2gvalue(value);

		if (gvalue) {
			g_object_set_property(gobject, Z_STRVAL_P(prop), gvalue);

			if (Z_TYPE_P(value) != IS_OBJECT) {
				g_value_unset(gvalue);
			}

			g_free(gvalue);
		}
	}

	std_hnd->write_property(zobject, prop, value TSRMLS_CC);

	return;
}

void php_midgard_gobject_unset_property(zval *object, zval *member TSRMLS_DC)
{
	zend_class_entry *ce = Z_OBJCE_P(object);

	SEPARATE_ARG_IF_REF(member);
	zend_call_method_with_1_params(&object, ce, &ce->__unset, ZEND_UNSET_FUNC_NAME, NULL, member);
	zval_ptr_dtor(&member);
}

/* "Register" object's properties ( constructor time ) */
void php_midgard_zendobject_register_properties(zval *zobject, GObject *gobject)
{
	if (gobject == NULL)
		return;

	guint n_prop, i;
	zval *tmp_object;
	const gchar *gclass_name;
	GValue pval = {0, };

	GParamSpec **props =
		g_object_class_list_properties(
				G_OBJECT_GET_CLASS(gobject), &n_prop);
	TSRMLS_FETCH();

	for (i = 0; i < n_prop; i++) {
		switch (props[i]->value_type) {
			case G_TYPE_STRING:
				add_property_string(zobject,
						(gchar*)props[i]->name, "", 1);
				break;

			case G_TYPE_UINT:
			case G_TYPE_INT:
				add_property_long(zobject,
						(gchar*)props[i]->name, 0);
				break;

			case G_TYPE_BOOLEAN:
				add_property_bool(zobject,
						(gchar*)props[i]->name, FALSE);
				break;

			case G_TYPE_FLOAT:
				add_property_double(zobject,
						(gchar*)props[i]->name, 0);
				break;

			case G_TYPE_OBJECT:
				g_value_init(&pval, props[i]->value_type);
				g_object_get_property(gobject, (gchar*)props[i]->name, &pval);
				gclass_name = g_type_name(G_OBJECT_TYPE(G_OBJECT(g_value_get_object(&pval))));

				if (gclass_name) {
					zend_class_entry *ce = php_midgard_get_class_ptr_by_name(gclass_name);

					if (NULL != ce) {
						MAKE_STD_ZVAL(tmp_object);
						object_init_ex(tmp_object, ce);
						php_midgard_zendobject_register_properties(tmp_object, G_OBJECT(g_value_get_object(&pval)));
						add_property_zval(zobject, (gchar*)props[i]->name, tmp_object);
					}
				}
				break;

			default:
				add_property_unset(zobject, (gchar*)props[i]->name);
				break;
		}
	}

	g_free(props);

	return;
}

/* Get object's properties */
HashTable *php_midgard_zendobject_get_properties(zval *zobject TSRMLS_DC)
{
	if (zobject == NULL)
		return NULL;

	php_midgard_gobject *php_gobject = php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);

	if (!G_IS_OBJECT(php_gobject->gobject))
		php_error(E_ERROR, "Underlying object is not GObject");
		/* php_error(E_ERROR, "Underlying object(%x) is not GObject", php_gobject->gobject); */

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_zendobject_get_properties()\n", zobject);
	}

	GObject *gobject = php_gobject->gobject;	
	guint propn, i;
	GParamSpec **props = g_object_class_list_properties(G_OBJECT_GET_CLASS(gobject), &propn);

	for (i = 0; i < propn; i++) {

		GValue pval = {0, };	
		g_value_init(&pval, props[i]->value_type);      
		g_object_get_property(gobject, (gchar*)props[i]->name, &pval);

		zval *tmp;
		MAKE_STD_ZVAL(tmp);

		php_midgard_gvalue2zval(&pval, tmp);
		zend_hash_update(php_gobject->zo.properties,
				props[i]->name, strlen(props[i]->name)+1,
				(void *)&tmp, sizeof(zval *), NULL);

		g_value_unset(&pval);
	}

	g_free(props);

	return php_gobject->zo.properties;
}

static void __php_midgard_gobject_dtor(void *object TSRMLS_DC);

static void __object_properties_dtor(zend_object *zo)
{
	HashPosition iterator;
	zval **zvalue = NULL;
	HashTable *props = zo->properties;
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] __object_properties_dtor()\n", zo);
	}

	zend_hash_internal_pointer_reset_ex(props, &iterator);

	while (zend_hash_get_current_data_ex(props, (void **)&zvalue, &iterator) == SUCCESS) {
		char *str_index;
		uint str_len;
		ulong num_index;
		zend_hash_get_current_key_ex(props, &str_index, &str_len, &num_index, 0, &iterator);

		if (MGDG(midgard_memory_debug)) {
			php_printf("[%p] property's [%p] refcount: %d (before) [%s]\n", zo, *zvalue, Z_REFCOUNT_P(*zvalue), str_index);
		}

		zend_hash_del(props, str_index, str_len);

		// first element was deleted, need to start over
		zend_hash_internal_pointer_reset_ex(props, &iterator);
	}

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= __object_properties_dtor()\n", zo);
	}
}

/* Object destructor */
static void __php_midgard_gobject_dtor(void *object TSRMLS_DC)
{
	if (!object)
		return;

	php_midgard_gobject *php_gobject = (php_midgard_gobject *) object;

	if (MGDG(midgard_memory_debug)) {
		zend_class_entry *obj_ce = php_gobject->zo.ce;
		php_printf("[%p] __php_midgard_gobject_dtor(%s)\n", php_gobject, obj_ce->name);
	}

	if (&php_gobject->zo == NULL)
		return;

	if (php_gobject->gobject == NULL) {
		/* This may be disabled, it's just for debugging purpose */
		/* php_error(E_NOTICE, "__php_midgard_gobject_dtor. Underlying GObject is NULL"); */
	} else if (G_IS_OBJECT(php_gobject->gobject)) {
		if (MGDG(midgard_memory_debug)) {
			php_printf("[%p] =========> G_IS_OBJECT\n", object);
		}

		if (G_OBJECT_TYPE_NAME(G_OBJECT(php_gobject->gobject)) != NULL) {
			if (MGDG(midgard_memory_debug)) {
				php_printf("[%p] =========> ..._TYPE_NAME != NULL\n", object);
			}

			if (MGDG(midgard_memory_debug)) {
				php_printf("[%p] =========> gobject's refcount = %d (before unref)\n", object, php_gobject->gobject->ref_count);
			}

			/*php_error(E_NOTICE, "%s DTOR (%p)", G_OBJECT_TYPE_NAME(G_OBJECT(php_gobject->gobject)), (void*)php_gobject->gobject); */
			/* TODO, find a way to destroy properties of object type.
			 * Memory usage will be a bit abused, but I really have no idea how it should be implemented */
			__object_properties_dtor(&php_gobject->zo);
			g_object_unref(G_OBJECT(php_gobject->gobject));
			php_gobject->gobject = NULL;
		}
	}

	zend_object_std_dtor(&php_gobject->zo TSRMLS_CC);

	php_gobject->gobject = NULL;
	efree(php_gobject);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= __php_midgard_gobject_dtor()\n", php_gobject);
	}

	php_gobject = NULL;
	object = NULL;
}

void php_midgard_init_properties_objects(zval *zobject)
{
	if (zobject == NULL)
		return;

	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_init_properties_objects\n", zobject);
	}

	GObject *gobject = G_OBJECT(__php_gobject_ptr(zobject));

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] ==> gobject = [%p]\n", zobject, gobject);
	}

	if (!gobject)
		return;

	php_midgard_gobject *php_gobject = php_midgard_zend_object_store_get_object(zobject TSRMLS_CC);
	guint propn, i;
	GParamSpec **pspecs = g_object_class_list_properties(G_OBJECT_GET_CLASS(gobject), &propn);

	for (i = 0; i < propn; i++) {
		
		/* Property which is not object type will be initialized in read_property hook.
		 * Workaround for PHP read/write property bug */
		if(G_TYPE_FUNDAMENTAL(pspecs[i]->value_type) != G_TYPE_OBJECT)
			                        continue;
		
		zval *prop_zobject;
		MAKE_STD_ZVAL(prop_zobject);

		GValue oval = {0, };
		g_value_init(&oval, G_TYPE_OBJECT);
		g_object_get_property(gobject, pspecs[i]->name, &oval);
		GObject *prop_gobject = g_value_get_object(&oval);

		zend_class_entry *ce =
			php_midgard_get_baseclass_ptr_by_name(G_OBJECT_TYPE_NAME(prop_gobject));

		object_init_ex(prop_zobject, ce);
		zval_add_ref(&prop_zobject);

		php_midgard_gobject *php_gobject =
			(php_midgard_gobject *)zend_object_store_get_object(prop_zobject TSRMLS_CC);

		php_gobject->gobject = prop_gobject;

		zend_update_property(Z_OBJCE_P(zobject), zobject,
      				pspecs[i]->name, strlen(pspecs[i]->name),
				prop_zobject TSRMLS_CC);

		g_value_unset(&oval);
	}

	g_free(pspecs);

	php_gobject->has_properties = TRUE;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= php_midgard_init_properties_objects\n", zobject);
	}
}

/* Object constructor */
zend_object_value php_midgard_gobject_new(zend_class_entry *class_type TSRMLS_DC)
{
	php_midgard_gobject *php_gobject;
	zend_object_value retval;
	zval *tmp;

	php_gobject = ecalloc(1, sizeof(php_midgard_gobject));
	zend_object_std_init(&php_gobject->zo, class_type TSRMLS_CC);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_gobject_new(%s)\n", &php_gobject->zo, class_type->name);
	}

	/* php_error(E_NOTICE, "CONSTRUCTOR %s", class_type->name); */

	php_gobject->gobject = NULL;
	php_gobject->has_properties = FALSE;
	php_gobject->magic = PHP_MIDGARD_GOBJ_MAGIC;

	/* Do not free these members. Those are owned by Zend. We just re use them. */
	php_gobject->user_ce = NULL;
	php_gobject->user_class_name = NULL;

	zend_hash_copy(php_gobject->zo.properties,
			&class_type->default_properties,
			(copy_ctor_func_t) zval_add_ref,
			(void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(php_gobject,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			__php_midgard_gobject_dtor,
			NULL TSRMLS_CC);

	retval.handlers = &php_midgard_gobject_handlers;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= php_midgard_gobject_new()\n", &php_gobject->zo);
	}

	return retval;
}

void php_midgard_gobject_init(zval *zvalue, const char *classname, GObject *gobject, gboolean dtor)
{
	zend_class_entry *ce = NULL;
	TSRMLS_FETCH();

	if (zvalue == NULL)
		MAKE_STD_ZVAL(zvalue);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_gobject_init(%s)\n", zvalue, classname);
	}

	ce = php_midgard_get_class_ptr_by_name(classname);

	if (ce == NULL)
		php_error(E_ERROR, "Class '%s' is not registered", classname);

	php_midgard_gobject_new_with_gobject(zvalue, ce, gobject, dtor);
}

void php_midgard_gobject_new_with_gobject(zval *zvalue, zend_class_entry *ce, GObject *gobject, gboolean dtor)
{
	php_midgard_gobject *php_gobject;
	zval *tmp;
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_gobject_new_with_gobject(%s)\n", zvalue, ce->name);
	}

	Z_TYPE_P(zvalue) = IS_OBJECT;

	php_gobject = ecalloc(1, sizeof(php_midgard_gobject));
	php_gobject->gobject = gobject;
	php_gobject->has_properties = FALSE;
	php_gobject->magic = PHP_MIDGARD_GOBJ_MAGIC;

	zend_object_std_init(&php_gobject->zo, ce TSRMLS_CC);

	zend_hash_copy(php_gobject->zo.properties,
			&ce->default_properties,
			(copy_ctor_func_t) zval_add_ref,
			(void *) &tmp, sizeof(zval *));

	if (dtor) {
		zvalue->value.obj.handle = zend_objects_store_put(php_gobject,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			__php_midgard_gobject_dtor,
			NULL TSRMLS_CC);
	} else {
		zvalue->value.obj.handle = zend_objects_store_put(php_gobject,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			NULL,
			NULL TSRMLS_CC);
	}

	Z_OBJ_HT_P(zvalue) = &php_midgard_gobject_handlers;

	if (ce->constructor) {
		php_printf("\n\nfound constructor\n\n");
		zend_call_method_with_0_params(&zvalue, ce, &ce->constructor, "__construct", NULL);
	} else {
		php_midgard_init_properties_objects(zvalue);
		/* php_error(E_NOTICE, "IMPLICIT CONSTRUCTOR %s (%p)", ce->name, gobject); */
	}
}

/* Other routines */
zend_class_entry *php_midgard_get_baseclass_ptr(zend_class_entry *ce)
{
	g_assert(ce);

	if (ce->parent == NULL) {

		return ce;

	} else {

		if (ce->parent == php_midgard_dbobject_class
				|| ce->parent == php_midgard_object_class
				|| ce->parent == php_midgard_view_class)
			return ce;

		ce = php_midgard_get_baseclass_ptr(ce->parent);
	}

	return ce;
}

zend_class_entry *php_midgard_get_mgdschema_class_ptr(zend_class_entry *ce)
{
	g_assert(ce != NULL);

	zend_class_entry *tmp = ce;

	while (
		tmp->parent
		&& tmp->parent != php_midgard_object_class
		&& tmp->parent != php_midgard_dbobject_class
	) {
		tmp = ce->parent;
	}

	return tmp;
}

zend_class_entry *php_midgard_get_mgdschema_class_ptr_by_name(const char *name)
{
	g_assert(name != NULL);

	zend_class_entry *ce;

	ce = php_midgard_get_class_ptr_by_name(name);

	if (ce == NULL)
		return NULL;

	return php_midgard_get_mgdschema_class_ptr(ce);
}

zend_class_entry *php_midgard_get_baseclass_ptr_by_name(const char *name)
{
	g_assert(name != NULL);

	zend_class_entry *ce, *rce;

	ce = php_midgard_get_class_ptr_by_name(name);

	if (NULL == ce) {
		rce = NULL;
	} else {
		rce = php_midgard_get_baseclass_ptr(ce);
	}

	return rce;
}

zend_class_entry *php_midgard_get_class_ptr_by_name(const char *name)
{
	g_assert(name != NULL);

	size_t name_length = strlen(name);
	TSRMLS_FETCH();

	if (PG(modules_activated) == 1 && EG(class_table)) {
		// request is started
		return zend_fetch_class((char *)name, name_length, ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);
	}

	// request is not started. we have to act on our own
	gchar *lower_class_name = g_ascii_strdown(name, name_length);
	zend_class_entry **ce;

	if (zend_hash_find(CG(class_table), (char *)lower_class_name, name_length + 1, (void **) &ce)  != SUCCESS) {
		*ce = NULL;
	}

	g_free(lower_class_name);

	return *ce;
}

gboolean php_midgard_is_derived_from_class(const gchar *classname, GType basetype,
		gboolean check_parent, zend_class_entry **base_class TSRMLS_DC)
{
	if (classname == NULL || *classname == '\0')
		return FALSE;

	gboolean isderived = FALSE;

	zend_class_entry *ce = php_midgard_get_baseclass_ptr_by_name(classname);

	if (ce == NULL) {

		php_error(E_WARNING, "Can not find zend class pointer for given %s class name", classname);
		return isderived;
	}

	*base_class = ce;

	GType classtype = g_type_from_name((const gchar *)ce->name);

	if (classtype == basetype)
		isderived = TRUE;

	if (check_parent == TRUE)
		isderived = g_type_is_a(classtype, basetype);

	return isderived;
}

void php_midgard_array_from_objects(GObject **objects, const gchar *class_name, zval *zarray TSRMLS_DC)
{
	if (!objects)
		return;

	zend_class_entry *ce = zend_fetch_class((char *)class_name, strlen(class_name), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
	size_t i = 0;

	while (objects[i] != NULL) {
		zval *zobject;
		MAKE_STD_ZVAL(zobject);

		object_init_ex(zobject, ce);
		MGD_PHP_SET_GOBJECT(zobject, G_OBJECT(objects[i]));
		zend_call_method_with_0_params(&zobject, ce, &ce->constructor, "__construct", NULL);

		zend_hash_next_index_insert(HASH_OF(zarray), &zobject, sizeof(zval *), NULL);

		i++;
	}

	return;
}

/* SIGNALS */

typedef struct {
	GClosure closure;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	zval *args;
	zval *zval_array;
	guint argc;
	guint type;
	zval *zobject;
	zval *connected;
} php_mgd_closure;

static void php_midgard_closure_invalidate(gpointer data, GClosure *closure)
{
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_closure_invalidate(data = %p)\n", closure, data);
	}

	php_mgd_closure *mgdclosure = (php_mgd_closure *) closure;

	zval_ptr_dtor(&(mgdclosure->fci.function_name));

	if (mgdclosure->args != NULL) {
		zval_ptr_dtor(&mgdclosure->args);
		mgdclosure->args = NULL;
	}

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] ----> done with args\n", closure);
	}

	mgdclosure->zobject = NULL;
	mgdclosure->connected = NULL;

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= php_midgard_closure_invalidate()\n", closure);
	}
}

void php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAMETERS)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	char *sname = NULL;
	uint sname_length;
	zval *zval_object = getThis();
	zval *zval_array = NULL;
	GClosure *closure = NULL;

	/* Keep '!' as passed object parameter ( or params array ) can be NULL */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  "sf|a!",
				&sname, &sname_length,
				&fci, &fci_cache, &zval_array) == FAILURE)
	{
		return;
	}

	/* Get underlying GObject instance */
	GObject *object = G_OBJECT(__php_gobject_ptr(zval_object));

	guint signal_id;
	GQuark signal_detail;
	if (!g_signal_parse_name(sname, G_OBJECT_TYPE(object), &signal_id, &signal_detail, TRUE)) {
		php_error(E_WARNING, "%s signal name is invalid", sname);
		RETURN_NULL();
		/* TODO , should we handle exception here? */
	}

	closure = php_midgard_closure_new_default(fci, fci_cache, zval_object, zval_array TSRMLS_CC);

	if (!closure) {
		php_error(E_WARNING, "Can not create new closure");
		RETURN_NULL();
	}

	g_signal_connect_closure_by_id(object, signal_id, signal_detail, closure, FALSE);
}

/* CLASS CLOSURES */

/* Workaround, as there's no GLib API to get class default closure by signal name or id */

static GHashTable *__classes_hash = NULL;

void __destroy_hash(gpointer data)
{
	if (!data)
		return;

	GHashTable *hash = (GHashTable*) data;

	g_hash_table_destroy(hash);
}

void php_midgard_gobject_closure_hash_new()
{
	if (__classes_hash == NULL)
		__classes_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static php_mgd_closure * __class_closure_lookup(GType class_type, guint signal_id)
{
	if (signal_id == 0)
		return NULL;

	if (__classes_hash == NULL)
		return NULL;

	GHashTable *closures_hash;
	closures_hash = g_hash_table_lookup(__classes_hash, g_type_name(class_type));

	if (!closures_hash)
		return NULL;

	gchar *sname = g_strdup(g_signal_name(signal_id));
	g_strdelimit (sname, G_STR_DELIMITERS ":^", '_');

	php_mgd_closure *pmc = g_hash_table_lookup(closures_hash, sname);
	g_free(sname);

	return pmc;
}

void __php_midgard_closure_free(gpointer data)
{
	TSRMLS_FETCH();
	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] __php_midgard_closure_free()\n", data);
	}

	GClosure *closure = (GClosure *) data;

	if (closure) {
		g_closure_unref(closure);
	}
}

void __free_hash_foreach(gpointer key, gpointer val, gpointer ud)
{
	GHashTable *hash = (GHashTable *) val;

	if (hash)
		g_hash_table_destroy(hash);
}

void php_midgard_gobject_closure_hash_free()
{
	if (__classes_hash != NULL) {
		g_hash_table_foreach(__classes_hash, __free_hash_foreach, NULL);
		g_hash_table_destroy(__classes_hash);
		__classes_hash = NULL;
	}

	return;
}

static void __register_class_closure(const gchar *class_name, const gchar *signal, php_mgd_closure *closure)
{
	if (__classes_hash == NULL)
		return;

	gchar *sname = g_strdup(signal);
	g_strdelimit (sname, G_STR_DELIMITERS ":^", '_'); /* FIXME, it should be fast, so no conversion here */

	guint signal_id = g_signal_lookup(sname, g_type_from_name(class_name));

	if (signal_id == 0) {
		g_warning("'%s' is not registered as event for '%s'", sname, class_name);
		g_free(sname);
		return;
	}

	GHashTable *closures_hash =
		g_hash_table_lookup(__classes_hash, class_name);

	if (!closures_hash) {
		closures_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, __php_midgard_closure_free);
	}

	g_hash_table_insert(closures_hash, (gpointer) sname, closure);
	g_hash_table_insert(__classes_hash, (gpointer) g_strdup(class_name), closures_hash);
}

static void php_midgard_closure_default_marshal(GClosure *closure,
		GValue *return_value, guint n_param_values,
		const GValue *param_values, gpointer invocation_hint,
		gpointer marshal_data)
{
	php_mgd_closure *mgdclosure = (php_mgd_closure *) closure;

	zval *params = NULL;
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_closure_default_marshal(args = %p)\n", closure, mgdclosure->args);
	}

	MAKE_STD_ZVAL(params);
	array_init(params);

	if (mgdclosure->zobject != NULL) {
		zval_add_ref(&(mgdclosure->zobject));
		zend_hash_next_index_insert(Z_ARRVAL_P(params), &(mgdclosure->zobject), sizeof(zval *), NULL);
	} else {
		zval *dummy = NULL;
		MAKE_STD_ZVAL(dummy);
		zend_hash_next_index_insert(Z_ARRVAL_P(params), &dummy, sizeof(zval *), NULL);
	}

	if (mgdclosure->args != NULL) {
		// + params
		HashTable *args_hash = Z_ARRVAL_P(mgdclosure->args);
		zend_uint argc = 1 + zend_hash_num_elements(args_hash);

		zend_hash_internal_pointer_reset(args_hash);

		size_t i;
		for (i = 1; i < argc; i++) {
			zval **ptr;
			zend_hash_get_current_data(args_hash, (void **)&ptr);
			zend_hash_move_forward(args_hash);

			if (MGDG(midgard_memory_debug)) {
				php_printf("[%p] ----> got ptr = %p, *ptr = %p [refcount = %d]\n", closure, ptr, *ptr, Z_REFCOUNT_P(*ptr));
			}

			zval_add_ref(ptr);
			zend_hash_next_index_insert(Z_ARRVAL_P(params), ptr, sizeof(zval *), NULL);
		}
	}

	zval *retval = NULL;

	zend_fcall_info_args(&(mgdclosure->fci), params TSRMLS_CC);
	mgdclosure->fci.retval_ptr_ptr = &retval;

	zend_call_function(&(mgdclosure->fci), &(mgdclosure->fci_cache) TSRMLS_CC);
	zend_fcall_info_args_clear(&(mgdclosure->fci), 1);

	zval_ptr_dtor(&retval);
	zval_ptr_dtor(&params);
}

static GClosure *php_midgard_closure_new_default(zend_fcall_info fci, zend_fcall_info_cache fci_cache, zval *zobject, zval *zval_array TSRMLS_DC)
{
	GClosure *closure;

	if (zobject == NULL || !g_type_from_name(Z_OBJCE_P(zobject)->name)) {
		closure = g_closure_new_simple(sizeof(php_mgd_closure), NULL);
	} else {
		GObject *object = G_OBJECT(__php_gobject_ptr(zobject));
		closure = g_closure_new_object(sizeof(php_mgd_closure), object);
	}

	if (!closure) {
		php_error(E_ERROR, "Couldn't create new closure");
		return NULL;
	}

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_closure_new_default(zobject = %p, zval_array = %p)\n", closure, zobject, zval_array);
	}

	php_mgd_closure *mgdclosure = (php_mgd_closure*) closure;
	zval_add_ref(&fci.function_name);
	mgdclosure->fci = fci;
	mgdclosure->fci_cache = fci_cache;
	mgdclosure->zobject = zobject; // we do not add reference here, as closure would be destroyed when object destroyed

	mgdclosure->args = NULL;

	if (zval_array) {
		zval_add_ref(&zval_array);
		mgdclosure->args = zval_array;
	}

	g_closure_add_invalidate_notifier(closure, NULL, php_midgard_closure_invalidate);
	g_closure_set_marshal((GClosure *)mgdclosure, php_midgard_closure_default_marshal);

	return (GClosure *)mgdclosure;
}

void php_midgard_object_class_connect_default(INTERNAL_FUNCTION_PARAMETERS)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	gchar *sname = NULL;
	gchar *class_name = NULL;
	guint sname_length;
	guint class_name_length;
	guint signal_id;
	zval *zobject = NULL;
	zval *zval_array = NULL;
	GQuark signal_detail;
	GClosure *closure = NULL;

	/* Keep '!' as passed object parameter ( or params array ) can be NULL */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  "ssf|a!",
				&class_name, &class_name_length,
				&sname, &sname_length,
				&fci, &fci_cache, &zval_array)
			== FAILURE)
	{
		return;
	}

	GType class_type = g_type_from_name((const gchar *)class_name);

	if (class_type == 0) {
		g_warning("Class %s is not registered in GType system", class_name);
		return;
	}

	if (!g_signal_parse_name(sname, class_type, &signal_id, &signal_detail, TRUE)) {
		php_error(E_WARNING, "%s signal name is invalid", sname);
		return;
		/* TODO , should we handle exception here? */
	}

	closure = php_midgard_closure_new_default(fci, fci_cache, zobject, zval_array TSRMLS_CC);

	if (!closure) {
		php_error(E_WARNING, "Can not create new closure");
		return;
	}

	php_mgd_closure *dclosure = (php_mgd_closure *) closure;
	dclosure->zval_array = zval_array;
	__register_class_closure(class_name, sname, (php_mgd_closure *)dclosure);
}

void php_midgard_object_connect_class_closures(GObject *object, zval *zobject)
{
	php_mgd_closure *closure = NULL;

	/* TODO, add error handling , IS_OBJECT , etc */
	if (zobject == NULL) {
		g_warning("Connect to class closure: failed to get zend object");
		return;
	}

	if (object == NULL) {
		g_warning("Connect to class closure: failed to get underlying object");
		return;
	}

	guint i = 0;
	guint n_ids, *ids;
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] php_midgard_object_connect_class_closures(zobject = %p)\n", object, zobject);
	}

	/* Use MIDGARD_TYPE_OBJECT type explicitly!
	 * Ancestor type is not taken into account in GLib's list_ids! */
	ids = g_signal_list_ids(MIDGARD_TYPE_OBJECT, &n_ids);

	if (n_ids == 0)
		return;

	for (i = 0; i < n_ids; i++) {
		closure = __class_closure_lookup(G_OBJECT_TYPE(object), ids[i]);

		if (closure) {
			if (MGDG(midgard_memory_debug)) {
				php_printf("[%p] ----> found \"default\" closure = %p\n", object, closure);
			}

			php_mgd_closure *dclosure = (php_mgd_closure *)
				php_midgard_closure_new_default(closure->fci, closure->fci_cache, zobject, closure->zval_array TSRMLS_CC);

			if (MGDG(midgard_memory_debug)) {
				php_printf("[%p] ----> created closure = %p\n", object, dclosure);
			}

			g_signal_connect_closure(object, g_signal_name(ids[i]), (GClosure *)dclosure, FALSE);
		}
	}

	g_free(ids);

	if (MGDG(midgard_memory_debug)) {
		php_printf("[%p] <= php_midgard_object_connect_class_closures(zobject = %p)\n", object, zobject);
	}

	return;
}

GParameter *php_midgard_array_to_gparameter(zval *params, guint *n_params)
{
	if (params == NULL)
		return NULL;

	HashTable *zht = Z_ARRVAL_P(params);

	/* count hash elements to set number of parameters */
	*n_params = (guint)zend_hash_num_elements(zht);

	if (*n_params == 0)
		return NULL;

	/* Initialize parameters vector */
	GParameter *parameters = g_new0(GParameter, *n_params);

	HashPosition pos;
	zval **value;
	char *key;
	uint key_len, i = 0, k;
	ulong num_index;

	/* reset array and set pointer at first position */
	zend_hash_internal_pointer_reset_ex(zht, &pos);

	/* iterate over array and set parameters' names and values */
	while (zend_hash_get_current_data_ex(zht, (void **) &value, &pos) == SUCCESS) {
		if (zend_hash_get_current_key_ex(zht, &key, &key_len, &num_index, 0, &pos) == HASH_KEY_IS_STRING) {
			parameters[i].name = (const gchar *)key;

			GValue gval = {0, };
			if (!php_midgard_gvalue_from_zval(*value, &gval))
				goto CLEAN_AND_RETURN_NULL;

			parameters[i].value = gval;
		} else {
			g_warning("Parameter key must be valid string!");
			goto CLEAN_AND_RETURN_NULL;
		}

		i++;
		zend_hash_move_forward_ex(zht, &pos);
	}

	return parameters;

CLEAN_AND_RETURN_NULL:

	for (k = i ; k > -1; k--) {
		g_value_unset(&parameters[i].value);
	}
	g_free(parameters);

	return NULL;
}
