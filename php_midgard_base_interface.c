/* Copyright (C) 2011 Piotr Pokora <piotrek.pokora@gmail.com>
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

zend_class_entry *php_midgard_base_interface_class;

zend_class_entry *
__get_interface_entry (const gchar *name)
{
	TSRMLS_FETCH();

	int iface_name_length = strlen(name);
       	char *lower_iface_name = g_ascii_strdown(name, iface_name_length);
	zend_class_entry **ce;

	if (zend_hash_find(CG(class_table), (char *)lower_iface_name, iface_name_length + 1, (void **) &ce) != SUCCESS) {
		return NULL;
	}
	g_free(lower_iface_name);

	return *ce;
}

/* This is done without zend_lookup_clas(_ex) because we register interfaces 
 * during MINIT phase, so EG(class_table) is not yet initialized.
 * If interfaces are loaded later, after RINIT phase, zend_lookup_class might be 
 * easier choice */
static zend_bool
__is_interface_registered (const gchar *name)
{
	if (__get_interface_entry(name) == NULL)
		return FALSE;

	return TRUE;
}

static void
__register_php_interface (const char *name)
{
	/* Silently return if interface is already registered */
	if (__is_interface_registered(name)) {
		return;
	}

	GType iface_type = g_type_from_name(name);

	/* If given type is not interface, silently gnore */
	if (!G_TYPE_IS_INTERFACE (iface_type))
		return;

	TSRMLS_FETCH();

	guint n_types;
	guint i;
	/* Get prerequisites and chain up.
	 * Gtype system doesn't provde any reflection for "derived" interfaces. */
	GType *prqs = g_type_interface_prerequisites (iface_type, &n_types);
	for (i = 0; i < n_types; i++) {
		const gchar *parent_name = g_type_name(prqs[i]);
		if (!__is_interface_registered (parent_name))
			__register_php_interface(parent_name);
	}

	zend_class_entry *php_iface, *php_iface_ptr;

	/* Create interface template */
	php_iface = g_new0(zend_class_entry, 1);
	php_iface->name = g_strdup (name);
	php_iface->name_length = strlen(name);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	php_iface->info.internal.builtin_functions = NULL;
#else
	php_iface->builtin_functions = NULL;
#endif
	php_iface->constructor = NULL;
	php_iface->destructor = NULL;
	php_iface->clone = NULL;
	php_iface->create_object = NULL;
	php_iface->interface_gets_implemented = NULL;
	php_iface->__call = NULL;
	php_iface->__get = NULL;
	php_iface->__set = NULL;
	php_iface->parent = NULL;
	php_iface->num_interfaces = 0;
	php_iface->interfaces = NULL;
	php_iface->get_iterator = NULL;
	php_iface->iterator_funcs.funcs = NULL;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 3
	php_iface->info.internal.module = NULL;
#else
	php_iface->module = NULL;
#endif
	php_iface->ce_flags = 0;

	/* Register interface */
	php_iface_ptr = zend_register_internal_interface(php_iface TSRMLS_CC);

	/* Implement prerequisites */
	for (i = 0; i < n_types; i++) {
		if (!G_TYPE_IS_INTERFACE(prqs[i])) /* GObject prerequisite, most likely */
			continue;
		zend_class_entry *prqs_iface = __get_interface_entry(g_type_name(prqs[i]));
		zend_class_implements(php_iface_ptr TSRMLS_CC, 1, prqs_iface);
	}
	g_free(prqs);


	/* freeing interface-template (it is not needed anymore) */
	g_free(php_iface);
}

PHP_MINIT_FUNCTION(midgard2_base_interface)
{
	/* TODO, if needed, base, fundamental GType interface could be registered as well */

	guint n_types, i;
	GType *all_types = g_type_children(G_TYPE_INTERFACE, &n_types);

	const gchar *typename;
	for (i = 0; i < n_types; i++) {
		typename = g_type_name(all_types[i]);
		__register_php_interface(typename);
	}

	g_free(all_types);

	return SUCCESS;
}
