#include "php_midgard.h"
#include "php_midgard_gobject.h"

typedef struct {
	GClosure closure;
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	zval *args;
	zval *zval_array;
	guint argc;
	guint type;
	zval *zobject;
	zval *connected;
} php_mgd_closure;

static void php_midgard_closure_invalidate(gpointer data, GClosure *closure)
{
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_closure_invalidate(data = %p)\n", closure, data);
	}

	php_mgd_closure *mgdclosure = (php_mgd_closure *) closure;

	zval_ptr_dtor(&(mgdclosure->fci.function_name));

	if (mgdclosure->args != NULL) {
		zval_ptr_dtor(&mgdclosure->args);
		mgdclosure->args = NULL;
	}

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] ----> done with args\n", closure);
	}

	mgdclosure->zobject = NULL;
	mgdclosure->connected = NULL;

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] <= php_midgard_closure_invalidate()\n", closure);
	}
}

/* CLASS CLOSURES */

static GHashTable *__classes_hash = NULL;

static void __destroy_hash(gpointer data)
{
	if (!data)
		return;

	GHashTable *hash = (GHashTable*) data;

	g_hash_table_destroy(hash);
}

void php_midgard_gobject_closure_hash_new()
{
	if (__classes_hash == NULL)
		__classes_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static php_mgd_closure * __class_closure_lookup(GType class_type, guint signal_id)
{
	if (signal_id == 0)
		return NULL;

	if (__classes_hash == NULL)
		return NULL;

	GHashTable *closures_hash;
	closures_hash = g_hash_table_lookup(__classes_hash, g_type_name(class_type));

	if (!closures_hash)
		return NULL;

	gchar *sname = g_strdup(g_signal_name(signal_id));
	g_strdelimit (sname, G_STR_DELIMITERS ":^", '_');

	php_mgd_closure *pmc = g_hash_table_lookup(closures_hash, sname);
	g_free(sname);

	return pmc;
}

void __php_midgard_closure_free(gpointer data)
{
	TSRMLS_FETCH();
	if (MGDG(midgard_memory_debug)) {
		printf("[%p] __php_midgard_closure_free()\n", data);
	}

	GClosure *closure = (GClosure *) data;

	if (closure) {
		g_closure_unref(closure);
	}
}

void __free_hash_foreach(gpointer key, gpointer val, gpointer ud)
{
	GHashTable *hash = (GHashTable *) val;

	if (hash)
		g_hash_table_destroy(hash);
}

void php_midgard_gobject_closure_hash_free()
{
	if (__classes_hash != NULL) {
		g_hash_table_foreach(__classes_hash, __free_hash_foreach, NULL);
		g_hash_table_destroy(__classes_hash);
		__classes_hash = NULL;
	}

	return;
}

static void __register_class_closure(const gchar *class_name, const gchar *signal, php_mgd_closure *closure)
{
	if (__classes_hash == NULL)
		return;

	gchar *sname = g_strdup(signal);
	g_strdelimit (sname, G_STR_DELIMITERS ":^", '_'); /* FIXME, it should be fast, so no conversion here */

	guint signal_id = g_signal_lookup(sname, g_type_from_name(class_name));

	if (signal_id == 0) {
		php_error(E_WARNING, "'%s' is not registered as event for '%s'", sname, class_name);
		g_free(sname);
		return;
	}

	GHashTable *closures_hash =
		g_hash_table_lookup(__classes_hash, class_name);

	if (!closures_hash) {
		closures_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, __php_midgard_closure_free);
	}

	g_hash_table_insert(closures_hash, (gpointer) sname, closure);
	g_hash_table_insert(__classes_hash, (gpointer) g_strdup(class_name), closures_hash);
}

static void php_midgard_closure_default_marshal(GClosure *closure,
		GValue *return_value, guint n_param_values,
		const GValue *param_values, gpointer invocation_hint,
		gpointer marshal_data)
{
	php_mgd_closure *mgdclosure = (php_mgd_closure *) closure;

	zval *params = NULL;
	TSRMLS_FETCH();

	if (MGDG(can_deliver_signals) == 0)
		return; // engine is not in sane state

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_closure_default_marshal(args = %p)\n", closure, mgdclosure->args);
	}

	MAKE_STD_ZVAL(params);
	array_init(params);

	// "10000" on next line is a HACK. Just took a number which is slightly above normal, but still doesn't look like invalid
	if (mgdclosure->zobject != NULL && Z_REFCOUNT_P(mgdclosure->zobject) < 10000) {
		if (MGDG(midgard_memory_debug)) {
			printf("[%p] ---> zobject refcount = %d\n", closure, Z_REFCOUNT_P(mgdclosure->zobject));
		}
		zval_add_ref(&(mgdclosure->zobject));
		zend_hash_next_index_insert(Z_ARRVAL_P(params), &(mgdclosure->zobject), sizeof(zval *), NULL);
	} else {
		zval *dummy = NULL;
		MAKE_STD_ZVAL(dummy);
		ZVAL_NULL(dummy);
		zend_hash_next_index_insert(Z_ARRVAL_P(params), &dummy, sizeof(zval *), NULL);
	}

	if (mgdclosure->args != NULL) {
		// + params
		HashTable *args_hash = Z_ARRVAL_P(mgdclosure->args);
		zend_uint argc = 1 + zend_hash_num_elements(args_hash);

		zend_hash_internal_pointer_reset(args_hash);

		size_t i;
		for (i = 1; i < argc; i++) {
			zval **ptr;
			zend_hash_get_current_data(args_hash, (void **)&ptr);
			zend_hash_move_forward(args_hash);

			if (MGDG(midgard_memory_debug)) {
				printf("[%p] ----> got ptr = %p, *ptr = %p [refcount = %d]\n", closure, ptr, *ptr, Z_REFCOUNT_P(*ptr));
			}

			zval_add_ref(ptr);
			zend_hash_next_index_insert(Z_ARRVAL_P(params), ptr, sizeof(zval *), NULL);
		}
	}

	zval *retval = NULL;

	zend_fcall_info_args(&(mgdclosure->fci), params TSRMLS_CC);
	mgdclosure->fci.retval_ptr_ptr = &retval;

	zend_call_function(&(mgdclosure->fci), &(mgdclosure->fci_cache) TSRMLS_CC);
	zend_fcall_info_args_clear(&(mgdclosure->fci), 1);

	if (retval)
		zval_ptr_dtor(&retval);

	zval_ptr_dtor(&params);
}

GClosure *php_midgard_closure_new_default(zend_fcall_info fci, zend_fcall_info_cache fci_cache, zval *zobject, zval *zval_array TSRMLS_DC)
{
	GClosure *closure;

	if (zobject == NULL || !g_type_from_name(Z_OBJCE_P(zobject)->name)) {
		closure = g_closure_new_simple(sizeof(php_mgd_closure), NULL);
	} else {
		GObject *object = G_OBJECT(__php_gobject_ptr(zobject));
		closure = g_closure_new_object(sizeof(php_mgd_closure), object);
	}

	if (!closure) {
		php_error(E_ERROR, "Couldn't create new closure");
		return NULL;
	}

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_closure_new_default(zobject = %p, zval_array = %p)\n", closure, zobject, zval_array);
	}

	php_mgd_closure *mgdclosure = (php_mgd_closure*) closure;
	zval_add_ref(&fci.function_name);
	mgdclosure->fci = fci;
	mgdclosure->fci_cache = fci_cache;
	mgdclosure->zobject = zobject; // we do not add reference here, as closure would be destroyed when object destroyed

	mgdclosure->args = NULL;

	if (zval_array) {
		zval_add_ref(&zval_array);
		mgdclosure->args = zval_array;
	}

	g_closure_add_invalidate_notifier(closure, NULL, php_midgard_closure_invalidate);
	g_closure_set_marshal((GClosure *)mgdclosure, php_midgard_closure_default_marshal);

	return (GClosure *)mgdclosure;
}

void php_midgard_object_class_connect_default(INTERNAL_FUNCTION_PARAMETERS)
{
	zend_fcall_info fci;
	zend_fcall_info_cache fci_cache;
	char *sname = NULL, *class_name = NULL;
	int sname_length, class_name_length;
	zval *zval_array = NULL;

	/* Keep '!' as passed object parameter ( or params array ) can be NULL */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,  "ssf|a!",
				&class_name, &class_name_length,
				&sname, &sname_length,
				&fci, &fci_cache, &zval_array)
			== FAILURE)
	{
		return;
	}

	GType class_type = g_type_from_name((const gchar *)class_name);

	if (class_type == 0) {
		php_error(E_WARNING, "Class %s is not registered in GType system", class_name);
		return;
	}

	guint signal_id;
	GQuark signal_detail;

	if (!g_signal_parse_name(sname, class_type, &signal_id, &signal_detail, TRUE)) {
		php_error(E_WARNING, "%s signal name is invalid", sname);
		return;
		/* TODO , should we handle exception here? */
	}

	GClosure *closure = php_midgard_closure_new_default(fci, fci_cache, NULL, zval_array TSRMLS_CC);

	if (!closure) {
		php_error(E_WARNING, "Can not create new closure");
		return;
	}

	php_mgd_closure *dclosure = (php_mgd_closure *) closure;
	dclosure->zval_array = zval_array;
	__register_class_closure(class_name, sname, dclosure);
}

void php_midgard_object_connect_class_closures(GObject *object, zval *zobject)
{
	php_mgd_closure *closure = NULL;

	/* TODO, add error handling , IS_OBJECT , etc */
	if (zobject == NULL) {
		php_error(E_WARNING, "Connect to class closure: failed to get zend object");
		return;
	}

	if (object == NULL) {
		php_error(E_WARNING, "Connect to class closure: failed to get underlying object");
		return;
	}

	guint i = 0;
	guint n_ids, *ids;
	TSRMLS_FETCH();

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] php_midgard_object_connect_class_closures(zobject = %p)\n", object, zobject);
	}

	/* Use MIDGARD_TYPE_OBJECT type explicitly!
	 * Ancestor type is not taken into account in GLib's list_ids! */
	ids = g_signal_list_ids(MIDGARD_TYPE_OBJECT, &n_ids);

	if (n_ids == 0)
		return;

	for (i = 0; i < n_ids; i++) {
		closure = __class_closure_lookup(G_OBJECT_TYPE(object), ids[i]);

		if (closure) {
			if (MGDG(midgard_memory_debug)) {
				printf("[%p] ----> found \"default\" closure = %p\n", object, closure);
			}

			php_mgd_closure *dclosure = (php_mgd_closure *)
				php_midgard_closure_new_default(closure->fci, closure->fci_cache, zobject, closure->zval_array TSRMLS_CC);

			if (MGDG(midgard_memory_debug)) {
				printf("[%p] ----> created closure = %p\n", object, dclosure);
			}

			g_signal_connect_closure(object, g_signal_name(ids[i]), (GClosure *)dclosure, FALSE);
		}
	}

	g_free(ids);

	if (MGDG(midgard_memory_debug)) {
		printf("[%p] <= php_midgard_object_connect_class_closures(zobject = %p)\n", object, zobject);
	}

	return;
}
