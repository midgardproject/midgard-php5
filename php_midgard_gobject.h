/* Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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
#ifndef PHP_MIDGARD_GOBJECT_GENERIC
#define PHP_MIDGARD_GOBJECT_GENERIC

#include "php_midgard.h"

#define PHP_MIDGARD_GOBJ_MAGIC 0x01020E0F

extern zend_class_entry *php_midgard_dbobject_class;
extern zend_class_entry *php_midgard_object_class;
extern zend_class_entry *php_midgard_view_class;

/* Opaque struct to extend zend_object */
typedef struct _php_midgard_gobject {
	zend_object zo;
	uint32_t magic;
	GObject *gobject;
	gboolean has_properties;
	zend_class_entry *user_ce;
	gchar *user_class_name;
} php_midgard_gobject;

/* Global handlers */
extern zend_object_handlers php_midgard_gobject_handlers;

/* Generic zend_object constructor */
zend_object_value php_midgard_gobject_new(zend_class_entry *class_type TSRMLS_DC);

/* Inititlize new php object for given GObject */
void php_midgard_gobject_new_with_gobject(zval *zvalue, zend_class_entry *ce, GObject *gobject, gboolean dtor);

/* Initialize new php object by classname */
void php_midgard_gobject_init(zval *zvalue, const char *classname, GObject *gobject, gboolean dtor);

/* Get base class pointer */
zend_class_entry *php_midgard_get_baseclass_ptr(zend_class_entry *ce);
zend_class_entry *php_midgard_get_mgdschema_class_ptr(zend_class_entry *ce);
zend_class_entry *php_midgard_get_mgdschema_class_ptr_by_name(const char *name);

/* Initialize all properties of G_TYPE_OBJECT type and add it to zend's objects store */
void php_midgard_init_properties_objects(zval *zobject);

/* Routines */
int __serialize_object_hook(zval *zobject,
	unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC);

int __unserialize_object_hook(zval **zobject, zend_class_entry *ce,
	const unsigned char *buffer, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC);

void php_midgard_gobject_unset_property(zval *object, zval *member TSRMLS_DC);

/* SIGNALS */
void php_midgard_gobject_connect(INTERNAL_FUNCTION_PARAMETERS);
void php_midgard_object_class_connect_default(INTERNAL_FUNCTION_PARAMETERS);

#endif /* PHP_MIDGARD_GOBJECT_GENERIC  */
