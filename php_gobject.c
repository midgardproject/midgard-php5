/* Copyright (C) 2012 Piotr Pokora <piotrek.pokora@gmail.com>
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

#include "php_gobject.h"
#include "php_midgard.h"
#include "php_midgard_gobject.h"
#include "php_midgard__helpers.h"

#include <Zend/zend_exceptions.h>
#include <spl/spl_exceptions.h>

void php_gobject_constructor(INTERNAL_FUNCTION_PARAMETERS)
{
	zval *zval_array = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  "a", &zval_array) == FAILURE)
		return;

	/* Get classname, so we can use correct GType */
	const char *php_classname = Z_OBJCE_P(getThis())->name;
	GType object_type = g_type_from_name (php_classname);

	/* If GType is abstract, throw exception */
	if (G_TYPE_IS_ABSTRACT(object_type)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0 TSRMLS_CC, 
				"Cannot create instance of abstract (non-instantiatable) type '%s'", php_classname);
		return;
	}

	if (!G_TYPE_IS_INSTANTIATABLE(object_type)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0 TSRMLS_CC, 
				"Cannot create instance of non-instantiatable type '%s'", php_classname);
		return;
	}

	guint n_params = 0;
	GParameter *parameters = php_midgard_array_to_gparameter(zval_array, &n_params TSRMLS_CC);

	GObject *gobject = g_object_newv(object_type, n_params, parameters);

	PHP_MGD_FREE_GPARAMETERS(parameters, n_params);

	MGD_PHP_SET_GOBJECT_G(getThis(), gobject);
}
