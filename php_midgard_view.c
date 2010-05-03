/*
 * Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
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
#include "php_midgard_object.h"

#include "php_midgard__helpers.h"

#include <Zend/zend_exceptions.h>
#include <midgard/midgard_view.h>

zend_class_entry *php_midgard_view_class = NULL;

#define __THROW_EXCEPTION \
	if (EG(exception)) { \
		return_value = zend_throw_exception(Z_OBJCE_P(EG(exception)), "", 0 TSRMLS_CC); \
		return; \
	}

#define _GET_VIEW_OBJECT \
	zval *zval_object = getThis(); \
	MgdView *object = MIDGARD_VIEW(__php_gobject_ptr(zval_object)); \
	if (!object) \
		php_error(E_ERROR, "Can not find underlying midgard view instance");

/* Object constructor */
static PHP_FUNCTION(php_midgard_view_constructor)
{
	RETVAL_FALSE;
	CHECK_MGD;
	zval *zval_object = getThis();
	GObject *gobject;

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		if (zend_parse_parameters_none() == FAILURE)
			return;

		MidgardView *view = g_object_new(g_type_from_name(Z_OBJCE_P(zval_object)->name), NULL);

		if (!view) {
			php_midgard_error_exception_throw(mgd_handle(TSRMLS_C));
			return;
		}

		MGD_PHP_SET_GOBJECT(zval_object, view);
	} else {
		// we already have gobject injected
	}
}


static void __register_view_php_classes(const gchar *class_name, zend_class_entry *parent)
{
	zend_class_entry *mgdclass, *mgdclass_ptr;
	// gint j;
	// guint _am = 0;
	TSRMLS_FETCH();

	/* lcn is freed in zend_register_internal_class */
	gchar *lcn = g_ascii_strdown(class_name, strlen(class_name));

	zend_function_entry __functions[2];

	__functions[0].fname = "__construct";
	__functions[0].handler = ZEND_FN(php_midgard_view_constructor);
	__functions[0].arg_info = NULL;
	__functions[0].num_args = 0;
	__functions[0].flags = ZEND_ACC_PUBLIC | ZEND_ACC_CTOR;

	__functions[1].fname = NULL;
	__functions[1].handler = NULL;
	__functions[1].arg_info = NULL;
	__functions[1].num_args = 0;
	__functions[1].flags = 0;

	mgdclass = g_new0(zend_class_entry, 1);
	mgdclass->name = lcn;
	mgdclass->name_length = strlen(class_name);
	mgdclass->builtin_functions = __functions;

	mgdclass->constructor = NULL;
	mgdclass->destructor = NULL;
	mgdclass->clone = NULL;
	mgdclass->create_object = NULL;
	mgdclass->interface_gets_implemented = NULL;
	mgdclass->__call = NULL;
	mgdclass->__get = NULL;
	mgdclass->__set = NULL;
	mgdclass->parent = parent;
	mgdclass->num_interfaces = 0;
	mgdclass->interfaces = NULL;
	mgdclass->get_iterator = NULL;
	mgdclass->iterator_funcs.funcs = NULL;
	mgdclass->module = NULL;
	mgdclass->ce_flags = 0;

	mgdclass_ptr = zend_register_internal_class(mgdclass TSRMLS_CC);
	mgdclass_ptr->ce_flags = 0;
	mgdclass_ptr->serialize = NULL;
	mgdclass_ptr->unserialize = NULL;
	mgdclass_ptr->create_object = php_midgard_gobject_new;
	g_free(mgdclass);
}

PHP_MINIT_FUNCTION(midgard2_view)
{
	/* Register midgard_view class */
	static zend_class_entry php_midgard_view_ce;
	INIT_CLASS_ENTRY(php_midgard_view_ce, "midgard_view", NULL);

	php_midgard_view_class = zend_register_internal_class_ex(&php_midgard_view_ce, php_midgard_dbobject_class, "midgard_view" TSRMLS_CC);

	guint n_types, i;
	GType *all_types = g_type_children(MIDGARD_TYPE_VIEW, &n_types);

	for (i = 0; i < n_types; i++) {
		const gchar *typename = g_type_name(all_types[i]);
		__register_view_php_classes(typename, php_midgard_view_class);
	}

	g_free(all_types);

	return SUCCESS;
}
