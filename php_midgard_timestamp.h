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

#ifndef PHP_MIDGARD_TIMESAMP_H
#define PHP_MIDGARD_TIMESTAMP_H

#include "php_midgard.h"

/* GLOBALS */
extern zend_class_entry *zend_datetime_class_ptr;
extern zend_class_entry *zend_date_timezone_class_ptr;

/* ROUTINES */
gboolean php_midgard_is_property_timestamp(MidgardDBObjectClass *klass, const gchar *property);

void php_midgard_datetime_from_gvalue(const GValue *gval, zval *zvalue);

zval *php_midgard_datetime_object_from_property(zval *object, const gchar *property);
zval *php_midgard_datetime_get_timestamp(zval *object);

#endif /* PHP_MIDGARD_TIMESTAMP_H */
