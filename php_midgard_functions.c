#include "php_midgard.h"
#include "php_midgard__helpers.h"

PHP_FUNCTION(mgd_version)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	const gchar *version = midgard_version();

	if (!version)
		version = "";

	RETURN_STRING((char *)version, 1);
}

PHP_FUNCTION(mgd_is_guid)
{
	zval *guid_zval = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &guid_zval) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(guid_zval) != IS_STRING) {
		// this should fail without an error
		RETURN_FALSE;
	}

	gchar *guid = (gchar *)Z_STRVAL_P(guid_zval);
	RETURN_BOOL(midgard_is_guid(guid));
}

