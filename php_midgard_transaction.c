/* Copyright (C) 2009 Piotr Pokora <piotrek.pokora@gmail.com>
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

#include "php_midgard__helpers.h"

static zend_class_entry *php_midgard_transaction_class;

#define _GET_TRNS_OBJECT \
	zval *zval_object = getThis(); \
	if (!zval_object) \
		php_error(E_ERROR, "NULL this_ptr for given midgard_transaction"); \
	php_midgard_gobject *php_gobject = __php_objstore_object(zval_object); \
	if (!php_gobject && !php_gobject->gobject) \
		php_error(E_ERROR, "Can not find underlying zend object structure"); \
	MidgardTransaction *trns = MIDGARD_TRANSACTION(php_gobject->gobject); \
	if (!trns) \
		php_error(E_ERROR, "Can not find underlying midgard_transaction instance");

/* Object constructor */
static PHP_METHOD(midgard_transaction, __construct)
{
	RETVAL_FALSE;
	CHECK_MGD;
	MidgardTransaction *trns;
	zval *zval_object = getThis();

	if (zend_parse_parameters_none() == FAILURE)
		return;

	trns = midgard_transaction_new(mgd_handle());

	if (!trns) {

		php_midgard_error_exception_throw(mgd_handle());
		return;
	}

	MGD_PHP_SET_GOBJECT(zval_object, trns);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_transaction___construct, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_transaction, begin)
{
	CHECK_MGD;
	gboolean rv;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_TRNS_OBJECT
	rv = midgard_transaction_begin(trns);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_transaction_begin, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_transaction, commit)
{
	CHECK_MGD;
	gboolean rv;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_TRNS_OBJECT
	rv = midgard_transaction_commit(trns);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_transaction_commit, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_transaction, rollback)
{
	CHECK_MGD;
	gboolean rv;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_TRNS_OBJECT
	rv = midgard_transaction_rollback(trns);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_transaction_rollback, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_transaction, get_status)
{
	CHECK_MGD;
	gboolean rv;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_TRNS_OBJECT
	rv = midgard_transaction_get_status(trns);

	RETURN_BOOL(rv);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_transaction_get_status, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_transaction, get_name)
{
	CHECK_MGD;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_TRNS_OBJECT
	const gchar *name = midgard_transaction_get_name(trns);

	RETURN_STRING((gchar *)name, 1);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_transaction_get_name, 0)
ZEND_END_ARG_INFO()


void php_midgard_transaction_init(int module_number)
{
	static function_entry transaction_methods[] = {
		PHP_ME(midgard_transaction, __construct, arginfo_midgard_transaction___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_transaction, begin,       arginfo_midgard_transaction_begin,       ZEND_ACC_PUBLIC)
		PHP_ME(midgard_transaction, commit,      arginfo_midgard_transaction_commit,      ZEND_ACC_PUBLIC)
		PHP_ME(midgard_transaction, rollback,    arginfo_midgard_transaction_rollback,    ZEND_ACC_PUBLIC)
		PHP_ME(midgard_transaction, get_status,  arginfo_midgard_transaction_get_status,  ZEND_ACC_PUBLIC)
		PHP_ME(midgard_transaction, get_name,    arginfo_midgard_transaction_get_name,    ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_transaction_class_entry;
	TSRMLS_FETCH();

	INIT_CLASS_ENTRY(
			php_midgard_transaction_class_entry,
			"midgard_transaction", transaction_methods);

	php_midgard_transaction_class =
		zend_register_internal_class(&php_midgard_transaction_class_entry TSRMLS_CC);

	/* Set function to initialize underlying data */
	php_midgard_transaction_class->create_object = php_midgard_gobject_new;
	php_midgard_transaction_class->serialize = NULL; /* FIXME, set (un)serialize for some explicit error if needed */
	php_midgard_transaction_class->unserialize = NULL;
}
