/* Copyright (C) 2010 Alexey Zakhlestin <indeyets@gmail.com>
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
#include "php_midgard_query.h"

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_query)
{
	PHP_MINIT(midgard2_query_storage)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(midgard2_query_holders)(INIT_FUNC_ARGS_PASSTHRU);
	// PHP_MINIT(midgard2_query_constraints)(INIT_FUNC_ARGS_PASSTHRU);
	// PHP_MINIT(midgard2_query_executors)(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}
