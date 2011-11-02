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
#include "php_midgard_gobject_closures.h"
#include <TSRM.h>
#include <zend_interfaces.h>
#include <locale.h>
#include "php_midgard_timestamp.h"

#include "php_midgard__helpers.h"

/* GVALUE ROUTINES */

static zend_bool php_midgard_gvalue_from_zval(const zval *zvalue, GValue *gvalue TSRMLS_DC)
{
	g_assert(zvalue != NULL);

	if (MGDG(midgard_memory_debug)) {
		printf("php_midgard_gvalue_from_zval(z=%p [refcount=%d], g=%p)\n", zvalue, Z_REFCOUNT_P((zval *)zvalue), gvalue);
	}

	HashTable *zhash;
	GValueArray *array;
	zval **value;
	HashPosition iterator;
	GValue *tmpval;
	zval*date_zval;
	gchar *lstring;

	switch (Z_TYPE_P(zvalue)) {
		case IS_ARRAY:
			zhash = Z_ARRVAL_P(zvalue);
			array =	g_value_array_new(zend_hash_num_elements(zhash));

			zend_hash_internal_pointer_reset_ex(zhash, &iterator);
			while (zend_hash_get_current_data_ex(
						zhash, (void **)&value, &iterator) == SUCCESS)
			{

				tmpval = php_midgard_zval2gvalue(*value TSRMLS_CC);
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
				date_zval = php_midgard_datetime_get_timestamp(zvalue TSRMLS_CC);
				GValue str_val = {0, };
				g_value_init(&str_val, G_TYPE_STRING);
				g_value_set_string(&str_val, Z_STRVAL_P(date_zval));

				g_value_init(gvalue, MGD_TYPE_TIMESTAMP);
				g_value_transform(&str_val, gvalue);
				g_value_unset(&str_val);
				zval_dtor(date_zval);
			} else {
				php_midgard_gobject *php_gobject = __php_objstore_object(zvalue);

				if (php_gobject && php_gobject->magic == PHP_MIDGARD_GOBJ_MAGIC && php_gobject->gobject) {
					g_value_init(gvalue, G_TYPE_OBJECT);
					g_value_set_object(gvalue, php_gobject->gobject);
				} else {
					// NOT GOBJECT. Don't know what to do with it
					return FALSE;
				}
			}
			break;

		case IS_RESOURCE:
			/* There's no way to handle resource gracefully */
			php_error(E_WARNING, "Got resource variable. Can not convert to glib-type");
			return FALSE;
			break;

		case IS_NULL:
			/* FIXME, we can not fallback to string type */
			g_value_init(gvalue, G_TYPE_STRING);
            g_value_set_string(gvalue, "");
			break;

		default:
			php_error(E_WARNING, "Got variable of unknown type. Can not convert to glib-type");
			return FALSE;
	}

	return TRUE;
}

GValue *php_midgard_zval2gvalue(const zval *zvalue TSRMLS_DC)
{
	g_assert(zvalue != NULL);

	GValue *gvalue = g_new0(GValue, 1);

	if (!php_midgard_gvalue_from_zval(zvalue, gvalue TSRMLS_CC)) {
		g_free(gvalue);
		return NULL;
	}

	return gvalue;
}

zend_bool php_midgard_gvalue2zval(const GValue *gvalue, zval *zvalue TSRMLS_DC)
{
	g_assert(gvalue);
	g_assert(zvalue);

	gchar *tmpstr;
	double f, dpval, tmp_val;
	GValueArray *array;
	GValue *arr_val;
	zval *zarr_val;

	/* Generic GValue */
	GType g_gtype = G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(gvalue));
	switch (g_gtype) {
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

		case G_TYPE_DOUBLE:
			/* I follow code from PHP_ROUND_WITH_FUZZ macro.
			 * If You find better way to add double property value,
			 * fell free to change this code. We do not have to worry
			 * about locale settings at this point ( I hope so ) */
			dpval = g_value_get_double(gvalue);
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
		case G_TYPE_INTERFACE:
			{
				GObject *gobject_property = g_value_get_object(gvalue);

				if (gobject_property) {
					const gchar *gclass_name = G_OBJECT_TYPE_NAME(gobject_property);

					if (!gclass_name)
						return FALSE;

					g_object_ref(gobject_property);
					php_midgard_gobject_init(zvalue, gclass_name, gobject_property, TRUE TSRMLS_CC);

					if (MGDG(midgard_memory_debug)) {
						printf("php_midgard_gvalue2zval: [%p] refcount=%d, gobj=%p, glib refcount=%d\n", zvalue, Z_REFCOUNT_P(zvalue), gobject_property, gobject_property->ref_count);
					}

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
					php_midgard_gvalue2zval(arr_val, zarr_val TSRMLS_CC);
					add_index_zval(zvalue, i, zarr_val);
				}
			}

			if (G_VALUE_TYPE(gvalue) == MGD_TYPE_TIMESTAMP) {
				php_midgard_datetime_from_gvalue(gvalue, zvalue TSRMLS_CC);
				return TRUE;
			}

			break;

		default:
			php_error(E_WARNING, "Don't know how to handle '%s' type. returning NULL instead", g_type_name(g_gtype));
			ZVAL_NULL(zvalue);
			break;
	}

	return FALSE;
}

/* OBJECTS ROUTINES */

/* check if there is glib-property of object linked to zval */
int php_midgard_gobject_has_property(zval *zobject, zval *prop, int check_type TSRMLS_DC)
{
	php_midgard_gobject *php_gobject = __php_objstore_object(zobject);
	char *prop_name = Z_STRVAL_P(prop);

	if (prop_name == NULL) {
		php_error(E_WARNING, "Can not check property with NULL name");
		return 0;
	}

	if (g_str_equal(prop_name, "")) {
		php_error(E_WARNING, "Can not check property with empty name");
		return 0;
	}

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_gobject_has_property(%s, check_type=%d). object's refcount=%d\n", zobject, prop_name, check_type, Z_REFCOUNT_P(zobject));
		printf("[%p] ----> gobject: %p, ref_count = %d\n", zobject, php_gobject, php_gobject->gobject->ref_count);
	}

	GObjectClass *klass = G_OBJECT_GET_CLASS(php_gobject->gobject);
	GParamSpec *pspec = g_object_class_find_property(klass, prop_name);

	int result = -1;

	if (check_type == 2) {
		// key exists, value is not important
		if (pspec != NULL) {
			return TRUE;
		}
	} else {
		zval *value = php_midgard_gobject_read_property(zobject, prop, BP_VAR_IS TSRMLS_CC);
		Z_ADDREF_P(value);

		if (MGDG(midgard_memory_debug)) {
			printf("[%p] ----> property: %p, ref_count = %d\n", zobject, value, Z_REFCOUNT_P(value));
		}

		if (check_type == 0) {
			// value is not null
			result = (Z_TYPE_P(value) != IS_NULL);
		} else {
			// value can be casted to TRUE
			result = zend_is_true(value);
		}

		zval_ptr_dtor(&value);
	}

	if (-1 == result) {
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		result = std_hnd->has_property(zobject, prop, check_type TSRMLS_CC);
	}

	return result;
}

/* Read Zend object property using underlying GObject's one */
zval *php_midgard_gobject_read_property(zval *zobject, zval *prop, int type TSRMLS_DC)
{
	zval *_retval = NULL;
	gboolean is_native_property = FALSE;
	GParamSpec *pspec = NULL;
	GObjectClass *klass = NULL;
	int silent = (type == BP_VAR_IS);

	const gchar *propname = Z_STRVAL_P(prop);
	int proplen = Z_STRLEN_P(prop) + 1;

	if (propname == NULL || *propname == '\0')
		php_error(E_ERROR, "Can not read empty property name");

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_gobject_read_property(%s)\n", zobject, propname);
	}

	GObject *gobject = __php_gobject_ptr(zobject);

	if (gobject && Z_STRVAL_P(prop) != NULL) {
		klass = G_OBJECT_GET_CLASS(gobject);

		/* Find GObject property */
		if (G_IS_OBJECT_CLASS(klass)) {
			pspec = g_object_class_find_property(klass, propname);

			if (pspec != NULL) {
				is_native_property = TRUE;

				if (!(pspec->flags & G_PARAM_READABLE)) {
					MAKE_STD_ZVAL(_retval);
					ZVAL_NULL(_retval);
					Z_DELREF_P(_retval); // we don't have local reference, so need to decrement refcount

					return _retval;
				}
			}
		}
	}

	/* If found, get property's gvalue. Create zval from it and return */
	if (is_native_property) {
		gboolean is_datetime_property = FALSE;
		if (MIDGARD_IS_DBOBJECT_CLASS(klass)) {
			is_datetime_property = php_midgard_is_property_timestamp(MIDGARD_DBOBJECT_CLASS(klass), propname);
		}

		GType prop_type = G_PARAM_SPEC_VALUE_TYPE(pspec);

		if (is_datetime_property) {
			/* Datetime property. $obj->metadata->created for example. */
			if (MGDG(midgard_memory_debug)) {
				php_printf("==========> DateTime\n");
			}

			_retval = php_midgard_datetime_object_from_property(zobject, propname TSRMLS_CC);
			Z_DELREF_P(_retval); //remove extra reference

			if (MGDG(midgard_memory_debug)) {
				printf("[%p] property's tmp-var refcount: %d [%s]\n", zobject, Z_REFCOUNT_P(_retval), propname);
			}
		} else if (G_TYPE_IS_OBJECT(prop_type) || G_TYPE_IS_INTERFACE(prop_type)) {
			/* Property of object type. $obj->metadata for example. */
			if (MGDG(midgard_memory_debug)) {
				php_printf("==========> G_TYPE_OBJECT\n");
			}

			zval **property;
			int hf_ret = zend_hash_find(Z_OBJPROP_P(zobject), propname, proplen, (void **) &property);

			if (hf_ret == SUCCESS) {
				_retval = *property;
				// zval_add_ref(property);

				if (MGDG(midgard_memory_debug)) {
					printf("==========> found\n");
					printf("==========> property's tmp-var refcount: %d [%s]\n", Z_REFCOUNT_P(_retval), propname);
					GObject *gobj = __php_gobject_ptr(_retval);
					printf("==========> property's gobject: %p [refcount: %d]\n", gobj, gobj->ref_count);
				}
			} else {
				if (MGDG(midgard_memory_debug)) {
					php_printf("==========> NOT found\n");
				}

				MAKE_STD_ZVAL(_retval);
				ZVAL_NULL(_retval);
				Z_DELREF_P(_retval); // we don't have local reference, so need to decrement refcount

				return _retval;
			}
		} else {
			/* Property of generic type. String, int, float, etc */
			if (MGDG(midgard_memory_debug)) {
				php_printf("==========> scalar\n");
			}

			GValue pval = {0, };
			g_value_init(&pval, pspec->value_type);
			g_object_get_property(gobject, propname, &pval);

			MAKE_STD_ZVAL(_retval);
			php_midgard_gvalue2zval(&pval, _retval TSRMLS_CC);
			Z_DELREF_P(_retval); // we don't have local reference, so need to decrement refcount

			g_value_unset(&pval);

			if (MGDG(midgard_memory_debug)) {
				printf("[%p] property's tmp-var refcount: %d [%s]\n", zobject, Z_REFCOUNT_P(_retval), propname);
			}
		}
	/* Fallback to zend. */
	} else {
		/* Property is not found. Fallback to zend's property handler
		 * Piotras: I have no idea what type should be passed instead
		 * of BP_VAR_NA. The point is to throw warning when property
		 * is not registered for (sub)class. */
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		_retval = std_hnd->read_property(zobject, prop, silent ? BP_VAR_IS : BP_VAR_NA TSRMLS_CC);
	}

	return _retval;
}

zval **php_midgard_gobject_get_property_ptr_ptr(zval *object, zval *member TSRMLS_DC)
{
	if (MGDG(midgard_memory_debug)) {
		const gchar *propname = Z_STRVAL_P(member);
		printf("[%p] php_midgard_gobject_get_property_ptr_ptr(%s)\n", object, propname);
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
		case G_TYPE_DOUBLE:
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
		const gchar *propname = Z_STRVAL_P(prop);
		printf("[%p] php_midgard_gobject_write_property(%s)\n", zobject, propname);
	}

	zend_object_handlers *std_hnd = zend_get_std_object_handlers();
	GObject *gobject = __php_gobject_ptr(zobject);

	/* Find GObject property */
	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(gobject), Z_STRVAL_P(prop));

	/* If found, set property's gvalue.*/
	if (pspec) {
		/* Property type might be initialized with IS_STRING or unset type.
		 * Check property's type and convert if needed. */
		_convert_value(value, pspec->value_type);

		GValue *gvalue = php_midgard_zval2gvalue(value TSRMLS_CC);

		if (gvalue) {
			/* Transform int to uint */
			if (pspec->value_type == G_TYPE_UINT && G_VALUE_HOLDS_INT (gvalue)) {
				GValue uintval = {0, };
				g_value_init (&uintval, G_TYPE_UINT);
				g_value_transform ((const GValue *) gvalue, &uintval);
				g_object_set_property (gobject, Z_STRVAL_P (prop), &uintval);
				g_value_unset (&uintval);
			} else {
				g_object_set_property(gobject, Z_STRVAL_P(prop), gvalue);
			}
			
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

/* Get object's properties */
HashTable *php_midgard_zendobject_get_properties(zval *zobject TSRMLS_DC)
{
	if (zobject == NULL)
		return NULL;

	php_midgard_gobject *php_gobject = __php_objstore_object(zobject);

	if (!G_IS_OBJECT(php_gobject->gobject))
		php_error(E_ERROR, "Underlying object is not GObject");

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_zendobject_get_properties(%s)\n", zobject, Z_OBJCE_P(zobject)->name);
		printf("[%p] ----> gobject: %p, ref_count = %d\n", zobject, php_gobject->gobject, php_gobject->gobject->ref_count);
	}

	GObject *gobject = php_gobject->gobject;
	guint propn, i;
	GParamSpec **props = g_object_class_list_properties(G_OBJECT_GET_CLASS(gobject), &propn);

	for (i = 0; i < propn; i++) {
		if (props[i]->flags & G_PARAM_CONSTRUCT_ONLY) {
			continue;
		}

		if (!(props[i]->flags & G_PARAM_READABLE)) {
			// not readable
			continue;
		}

		if (php_gobject->has_properties) {
			if (G_TYPE_IS_OBJECT(props[i]->value_type) || G_TYPE_IS_INTERFACE(props[i]->value_type)) {
				// do not reinit objects
				continue;
			}

			if (props[i]->value_type == MGD_TYPE_TIMESTAMP) {
				// do not reinit datetime objects (to keep var_dump() happy)
				continue;
			}
		}

		GValue pval = {0, };
		g_value_init(&pval, props[i]->value_type);
		g_object_get_property(gobject, (gchar*)props[i]->name, &pval);

		zval *tmp;

		if (props[i]->value_type == MGD_TYPE_TIMESTAMP) {
			// link it to object
			tmp = php_midgard_datetime_object_from_property(zobject, props[i]->name TSRMLS_CC);
		} else {
			MAKE_STD_ZVAL(tmp);
			php_midgard_gvalue2zval(&pval, tmp TSRMLS_CC);
		}

		zend_hash_update(php_gobject->zo.properties,
				props[i]->name, strlen(props[i]->name)+1,
				(void *)&tmp, sizeof(zval *), NULL);

		g_value_unset(&pval);
	}

	g_free(props);

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] <= php_midgard_zendobject_get_properties()\n", zobject);
	}

	php_gobject->has_properties = TRUE;

	return php_gobject->zo.properties;
}

static void __object_properties_dtor(zend_object *zo TSRMLS_DC)
{
	HashPosition iterator;
	zval **zvalue = NULL;
	HashTable *props = zo->properties;

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] __object_properties_dtor()\n", zo);
	}

	zend_hash_internal_pointer_reset_ex(props, &iterator);

	while (zend_hash_get_current_data_ex(props, (void **)&zvalue, &iterator) == SUCCESS) {
		char *str_index;
		uint str_len;
		ulong num_index;
		zend_hash_get_current_key_ex(props, &str_index, &str_len, &num_index, 0, &iterator);

		if (MGDG(midgard_memory_debug)) {
			printf("[%p] property's [%p] refcount: %d (before) [%s]\n", zo, *zvalue, Z_REFCOUNT_P(*zvalue), str_index);
		}

		zend_hash_del(props, str_index, str_len);

		// first element was deleted, need to start over
		zend_hash_internal_pointer_reset_ex(props, &iterator);
	}

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] <= __object_properties_dtor()\n", zo);
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
		printf("[%p] __php_midgard_gobject_dtor(%s)\n", php_gobject, obj_ce->name);
	}

	if (&php_gobject->zo == NULL)
		return;

	if (php_gobject->gobject == NULL) {
		/* This may be disabled, it's just for debugging purpose */
		/* php_error(E_NOTICE, "__php_midgard_gobject_dtor. Underlying GObject is NULL"); */
	} else if (G_IS_OBJECT(php_gobject->gobject)) {
		if (MGDG(midgard_memory_debug)) {
			printf("[%p] =========> G_IS_OBJECT\n", object);
		}

		if (G_OBJECT_TYPE_NAME(php_gobject->gobject) != NULL) {
			if (MGDG(midgard_memory_debug)) {
				printf("[%p] =========> ..._TYPE_NAME != NULL\n", object);
			}

			if (MGDG(midgard_memory_debug)) {
				printf("[%p] =========> gobject's refcount = %d (before unref)\n", object, php_gobject->gobject->ref_count);
			}

			/*php_error(E_NOTICE, "%s DTOR (%p)", G_OBJECT_TYPE_NAME(php_gobject->gobject), (void*)php_gobject->gobject); */
			/* TODO, find a way to destroy properties of object type.
			 * Memory usage will be a bit abused, but I really have no idea how it should be implemented */
			__object_properties_dtor(&php_gobject->zo TSRMLS_CC);
			g_object_unref(php_gobject->gobject);
			php_gobject->gobject = NULL;
		}
	}

	zend_object_std_dtor(&php_gobject->zo TSRMLS_CC);

	php_gobject->gobject = NULL;
	efree(php_gobject);

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] <= __php_midgard_gobject_dtor()\n", php_gobject);
	}

	php_gobject = NULL;
	object = NULL;
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
		printf("[%p] php_midgard_gobject_new(%s)\n", &php_gobject->zo, class_type->name);
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
		printf("[%p] <= php_midgard_gobject_new()\n", &php_gobject->zo);
	}

	return retval;
}

void php_midgard_gobject_init(zval *zvalue, const char *php_classname, GObject *gobject, gboolean dtor TSRMLS_DC)
{
	zend_class_entry *ce = NULL;

	if (zvalue == NULL)
		MAKE_STD_ZVAL(zvalue);

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_gobject_init(%s, %p [gobject refcount = %d])\n", zvalue, php_classname, gobject, gobject->ref_count);
	}

	ce = php_midgard_get_class_ptr_by_name(php_classname TSRMLS_CC);

	if (ce == NULL)
		php_error(E_ERROR, "Class '%s' is not registered", php_classname);

	php_midgard_gobject_new_with_gobject(zvalue, ce, gobject, dtor TSRMLS_CC);
}

void php_midgard_gobject_new_with_gobject(zval *zvalue, zend_class_entry *ce, GObject *gobject, gboolean dtor TSRMLS_DC)
{
	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_gobject_new_with_gobject(%s)\n", zvalue, ce->name);
	}

	object_init_ex(zvalue, ce);
	MGD_PHP_SET_GOBJECT_G(zvalue, gobject);

	if (MIDGARD_IS_OBJECT(gobject) && ce->constructor) {
		zend_call_method_with_0_params(&zvalue, ce, &ce->constructor, "__construct", NULL);
	}

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] <= php_midgard_gobject_new_with_gobject(%s)\n", zvalue, ce->name);
	}
}

/* Other routines */
zend_class_entry *php_midgard_get_baseclass_ptr(zend_class_entry *ce)
{
	g_assert(ce);

	if (ce->parent == NULL) {
		return ce;
	}

	GType g_class_type = g_type_from_name(ce->name);
	if (g_class_type != G_TYPE_INVALID) {
		if (g_type_is_a (g_class_type, MIDGARD_TYPE_DBOBJECT)) {
			return ce;
		}
	}

	if (ce->parent == php_midgard_dbobject_class
			|| ce->parent == php_midgard_object_class
			|| ce->parent == php_midgard_view_class
	) {
		return ce;
	}

	return php_midgard_get_baseclass_ptr(ce->parent);
}

zend_class_entry *php_midgard_get_mgdschema_class_ptr(zend_class_entry *ce)
{
	g_assert(ce != NULL);

	zend_class_entry *tmp = ce;

	GType g_class_type = g_type_from_name(ce->name);
	if (g_class_type != G_TYPE_INVALID) {
		if (g_type_is_a (g_class_type, MIDGARD_TYPE_DBOBJECT)) {
			return tmp;
		}
	}

	while (
		tmp->parent
		&& tmp->parent != php_midgard_object_class
		&& tmp->parent != php_midgard_dbobject_class
	) {
		tmp = tmp->parent;
	}

	return tmp;
}

zend_class_entry *php_midgard_get_mgdschema_class_ptr_by_name(const char *name TSRMLS_DC)
{
	g_assert(name != NULL);

	zend_class_entry *ce = php_midgard_get_class_ptr_by_name(name TSRMLS_CC);

	if (ce == NULL) {
		return NULL;
	}

	return php_midgard_get_mgdschema_class_ptr(ce);
}

zend_class_entry *php_midgard_get_baseclass_ptr_by_name(const char *name TSRMLS_DC)
{
	g_assert(name != NULL);

	zend_class_entry *ce = php_midgard_get_class_ptr_by_name(name TSRMLS_CC);

	if (NULL == ce) {
		return NULL;
	}

	return php_midgard_get_baseclass_ptr(ce);
}

// Can fetch class even if EG() is not available (before request started)
zend_class_entry *php_midgard_get_class_ptr_by_name(const char *name TSRMLS_DC)
{
	g_assert(name != NULL);

	size_t name_length = strlen(name);

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

gboolean php_midgard_is_derived_from_class(const char *classname, GType basetype,
		gboolean check_parent, zend_class_entry **base_class TSRMLS_DC)
{
	if (classname == NULL || *classname == '\0')
		return FALSE;

	zend_class_entry *ce = php_midgard_get_baseclass_ptr_by_name(classname TSRMLS_CC);

	if (ce == NULL) {
		php_error(E_WARNING, "Can not find zend class pointer for given %s class name", classname);
		return FALSE;
	}

	*base_class = ce;

	const gchar *g_classname = php_class_name_to_g_class_name(ce->name);
	GType classtype = g_type_from_name(g_classname);

	if (classtype == basetype)
		return TRUE;

	if (check_parent == TRUE)
		return g_type_is_a(classtype, basetype);

	return FALSE;
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

		php_midgard_gobject_new_with_gobject(zobject, ce, objects[i], TRUE TSRMLS_CC);

		zend_hash_next_index_insert(HASH_OF(zarray), &zobject, sizeof(zval *), NULL);

		i++;
	}

	return;
}

/* SIGNALS */
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
	GObject *object = __php_gobject_ptr(zval_object);

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

GParameter *php_midgard_array_to_gparameter(zval *params, guint *n_params TSRMLS_DC)
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
			if (!php_midgard_gvalue_from_zval(*value, &gval TSRMLS_CC))
				goto CLEAN_AND_RETURN_NULL;

			parameters[i].value = gval;
		} else {
			php_error(E_WARNING, "Parameter key must be valid string!");
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

const gchar* php_class_name_to_g_class_name(const char *php_class_name)
{
	zend_class_entry *ce = php_midgard_get_class_ptr_by_name(php_class_name TSRMLS_CC);
	if (ce)
		return ce->name;

	return php_class_name;
}
