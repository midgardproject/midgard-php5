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

zend_class_entry *php_midgard_repligard_class = NULL;

PHP_MINIT_FUNCTION(midgard2_repligard)
{
	/* Register midgard_repligard class */
	static zend_class_entry php_midgard_repligard_ce;
	INIT_CLASS_ENTRY(php_midgard_repligard_ce, "MidgardRepligard", NULL);

	php_midgard_repligard_class = zend_register_internal_class_ex(&php_midgard_repligard_ce, php_midgard_dbobject_class, "MidgardDBObject" TSRMLS_CC);
	php_midgard_repligard_class->ce_flags = ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	zend_register_class_alias("midgard_repligard", php_midgard_repligard_class);

	return SUCCESS;
}
