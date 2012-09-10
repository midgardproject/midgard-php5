/* 
 * Copyright (C) 2012 Piotr Pokora <piotrek.pokora@gmail.com>
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

/* midgard core interfaces */
zend_class_entry *php_midgard_model_class;
zend_class_entry *php_midgard_model_reference_class;

/* midgard core classes */
zend_class_entry *php_midgard_object_reference_class;

/*	Model	*/

static PHP_METHOD(midgard_model, get_name)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	GError *error = NULL;
	MidgardModel *model = MIDGARD_MODEL(__php_gobject_ptr(getThis()));
	const gchar *name = midgard_model_get_name(model, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get model. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	if (name == NULL)
		RETURN_NULL();
		
	RETURN_STRING(name, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_model_get_name, 0, 0, 0)
ZEND_END_ARG_INFO()


/*	ModelReference	*/

static PHP_METHOD(midgard_model_reference, get_id)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	GError *error = NULL;
	MidgardModelReference *model = MIDGARD_MODEL_REFERENCE(__php_gobject_ptr(getThis()));
	const gchar *id = midgard_model_reference_get_id(model, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get reference id. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	if (id == NULL)
		RETURN_NULL();
		
	RETURN_STRING(id, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_model_reference_get_id, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_model_reference, get_id_value)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	GError *error = NULL;
	MidgardModelReference *model = MIDGARD_MODEL_REFERENCE(__php_gobject_ptr(getThis()));
	GValue *id = midgard_model_reference_get_id_value(model, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get reference id value. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}
		
	php_midgard_gvalue2zval(id, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_model_reference_get_id_value, 0, 0, 0)
ZEND_END_ARG_INFO()

/*	ObjectReference	*/

static PHP_METHOD(midgard_object_reference, __construct)
{
	char *id = NULL;
	int id_length = 0;
	char *name = NULL;
	int name_length = 0;
	zval *z_workspace = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s!sO",
				&id, &id_length,
				&name, &name_length, 
				&z_workspace, php_midgard_workspace_class
				) == FAILURE) {
		return;
	}

	MidgardWorkspace *workspace = NULL;
	if (z_workspace != NULL)
		workspace = MIDGARD_WORKSPACE(__php_gobject_ptr(z_workspace));
	MidgardObjectReference *reference = midgard_object_reference_new (id, name, workspace);
	MGD_PHP_SET_GOBJECT(getThis(), reference);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_reference___construct, 0, 0, 3)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_OBJ_INFO(0, workspace, midgard_workspace, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_object_reference, get_workspace)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	MidgardObjectReference *reference = MIDGARD_OBJECT_REFERENCE(__php_gobject_ptr(getThis()));
	GError *error = NULL;
	MidgardWorkspace *workspace = midgard_object_reference_get_workspace(reference, &error);
	if (error) {
		zend_throw_exception_ex(ce_midgard_error_exception, 0 TSRMLS_CC,
				"Failed to get workspace. %s", error && error->message ? error->message : "Unknown reason");
		return;
	}

	if (workspace == NULL)
		RETURN_NULL();
	
	MGD_PHP_SET_GOBJECT(return_value, G_OBJECT(workspace));
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_midgard_object_reference_get_workspace, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_MINIT_FUNCTION(midgard2_model)
{
	/*	Model	*/
	static zend_function_entry midgard_model_methods[] = {
		PHP_ME(midgard_model,	get_name,	arginfo_midgard_model_get_name,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_model_class_entry;
	INIT_CLASS_ENTRY(php_midgard_model_class_entry, "MidgardModel", midgard_model_methods);

	php_midgard_model_class = zend_register_internal_class(&php_midgard_model_class_entry TSRMLS_CC);
	php_midgard_model_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_model_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_model_class, strdup("Base, abstract class for any model"));

	zend_register_class_alias("midgard_model", php_midgard_model_class);

	/*	ModelReference	*/
	static zend_function_entry midgard_model_reference_methods[] = {
		PHP_ME(midgard_model_reference,	get_id,		arginfo_midgard_model_reference_get_id,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_model_reference,	get_id_value,	arginfo_midgard_model_reference_get_id_value,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_model_reference_class_entry;
	INIT_CLASS_ENTRY(php_midgard_model_reference_class_entry, "MidgardModelRefrence", midgard_model_reference_methods);

	php_midgard_model_reference_class = zend_register_internal_class(&php_midgard_model_reference_class_entry TSRMLS_CC);
	php_midgard_model_reference_class->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	php_midgard_model_reference_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_model_reference_class, strdup("Base, abstract class for any reference model"));

	zend_register_class_alias("midgard_model_reference", php_midgard_model_reference_class);

	/*	ObjectReference	*/
	static zend_function_entry midgard_object_reference_methods[] = {
		PHP_ME(midgard_object_reference,	__construct,	arginfo_midgard_object_reference___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_object_reference,	get_workspace,	arginfo_midgard_object_reference_get_workspace,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_object_reference_class_entry;
	INIT_CLASS_ENTRY(php_midgard_object_reference_class_entry, "MidgardObjectReference", midgard_object_reference_methods);

	php_midgard_object_reference_class = zend_register_internal_class_ex(&php_midgard_object_reference_class_entry, php_midgard_model_reference_class, 
			"MidgardObjectReference" TSRMLS_CC);
	php_midgard_object_reference_class->create_object = php_midgard_gobject_new;
	CLASS_SET_DOC_COMMENT(php_midgard_model_class, strdup("Object which holds reference"));

	zend_class_implements(php_midgard_object_reference_class TSRMLS_CC, 1, php_midgard_model_class);
	zend_class_implements(php_midgard_object_reference_class TSRMLS_CC, 1, php_midgard_model_reference_class);
	zend_register_class_alias("midgard_object_reference", php_midgard_object_reference_class);

	return SUCCESS;
}
