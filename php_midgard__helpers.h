#include <php.h>

#ifndef zend_call_method_with_3_params
ZEND_API zval* zend_call_method__mgd(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval* arg1, zval* arg2, zval* arg3, zval* arg4 TSRMLS_DC);

# define zend_call_method_with_3_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3) \
	zend_call_method__mgd(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 3, arg1, arg2, arg3, NULL TSRMLS_CC)

# define zend_call_method_with_4_params(obj, obj_ce, fn_proxy, function_name, retval, arg1, arg2, arg3, arg4) \
	zend_call_method__mgd(obj, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval, 3, arg1, arg2, arg3, arg4 TSRMLS_CC)
#endif // zend_call_method_with_3_params

/* These macros are available since 5.3, so we add them in 5.2 */
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 2
# define E_DEPRECATED E_NOTICE
# define Z_SET_ISREF_P(ptr) (ptr)->is_ref = 1
# define Z_REFCOUNT_P(ptr) (ptr)->refcount
# define Z_ADDREF_P(ptr) (ptr)->refcount++
# define Z_DELREF_P(ptr) (ptr)->refcount--
# define Z_DELREF_PP(pptr) Z_DELREF_P(*(pptr))
# define zend_parse_parameters_none() zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")

ZEND_API void zend_fcall_info_args_clear(zend_fcall_info *fci, int free_mem);

zend_class_entry *php_date_get_date_ce(void);
zend_class_entry *php_date_get_timezone_ce(void);
#endif

/* This macro frees GParameter structure */
#define PHP_MGD_FREE_GPARAMETERS(parameters, n_params) \
{ \
	guint _i; \
	for (_i = 0; _i < n_params; _i++) { \
		g_value_unset(&parameters[_i].value); \
	} \
	g_free(parameters); \
}

void 		php_midgard_docs_add_class 		(const gchar *classname);
void		php_midgard_docs_add_method_comment 	(const char *classname, const char *method, const char *comment);
const gchar 	*php_midgard_docs_get_method_comment 	(const gchar *classname, const gchar *method);

#define php_mgd_register_auto_global(name) \
		zend_register_auto_global(#name, sizeof(#name)-1, NULL TSRMLS_CC);
