/* Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
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
#include <zend_exceptions.h>

/* GObject wrapper for GMainLoop */
#define MIDGARD_TYPE_G_MAIN_LOOP            (midgard_g_main_loop_get_type())
#define MIDGARD_G_MAIN_LOOP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), MIDGARD_TYPE_G_MAIN_LOOP, MidgardGMainLoop))
#define MIDGARD_G_MAIN_LOOP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), MIDGARD_TYPE_G_MAIN_LOOP, MidgardGMainLoopClass))
#define MIDGARD_IS_G_MAIN_LOOP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), MIDGARD_TYPE_G_MAIN_LOOP))
#define MIDGARD_IS_G_MAIN_LOOP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MIDGARD_TYPE_G_MAIN_LOOP))
#define MIDGARD_G_MAIN_LOOP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), MIDGARD_TYPE_G_MAIN_LOOP, MidgardGMainLoopClass))

typedef struct _MidgardGMainLoop MidgardGMainLoop;
typedef struct _MidgardGMainLoopClass MidgardGMainLoopClass;

struct _MidgardGMainLoop{
	GObject parent;
        /* private field */
	GMainLoop *g_m_loop;
};

struct _MidgardGMainLoopClass{
	GObjectClass parent;
};

GType midgard_g_main_loop_get_type(void);

MidgardGMainLoop *midgard_g_main_loop_new(void)
{
	MidgardGMainLoop *self = g_object_new(MIDGARD_TYPE_G_MAIN_LOOP, NULL);
	self->g_m_loop = g_main_loop_new(NULL, FALSE);
	return self;
}

static void _midgard_g_main_loop_finalize(GObject *object) 
{
	MidgardGMainLoop *self = (MidgardGMainLoop *) object;
	if (self->g_m_loop)
		g_main_loop_unref (self->g_m_loop);
	self->g_m_loop = NULL;
}

static void _midgard_g_main_loop_class_init(gpointer g_class, gpointer g_class_data)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(g_class);
	gobject_class->finalize = _midgard_g_main_loop_finalize;
}

GType midgard_g_main_loop_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (MidgardGMainLoopClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) _midgard_g_main_loop_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (MidgardDbus),
			0,              /* n_preallocs */
			NULL		/* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT, "midgard_g_main_loop", &info, 0);
	}
	return type;
}


zend_class_entry *php_midgard_g_mainloop_class;

#define _GET_MAINLOOP_OBJECT \
	zval *zval_object = getThis(); \
	MidgardGMainLoop *mainloop = MIDGARD_G_MAIN_LOOP(__php_gobject_ptr(zval_object)); \
	if (!mainloop) \
		php_error(E_ERROR, "Can not find underlying main loop instance");

/* Object constructor */
static PHP_METHOD(midgard_g_mainloop, __construct)
{
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	zval *zval_object = getThis();
	GObject *gobject;

	gobject = __php_gobject_ptr(zval_object);

	if (!gobject) {
		MidgardGMainLoop *mainloop = midgard_g_main_loop_new();
		MGD_PHP_SET_GOBJECT(zval_object, mainloop);
	} 
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_g_mainloop___construct, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_g_mainloop, run)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MAINLOOP_OBJECT;
	g_main_loop_run(mainloop->g_m_loop);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_g_mainloop_run, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_g_mainloop, is_running)
{
	RETVAL_FALSE;

	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MAINLOOP_OBJECT;
	RETURN_BOOL(g_main_loop_is_running(mainloop->g_m_loop));
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_g_mainloop_is_running, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(midgard_g_mainloop, quit)
{
	if (zend_parse_parameters_none() == FAILURE)
		return;

	_GET_MAINLOOP_OBJECT;
	g_main_loop_quit(mainloop->g_m_loop);
}

ZEND_BEGIN_ARG_INFO(arginfo_midgard_g_mainloop_quit, 0)
ZEND_END_ARG_INFO()

/* Initialize ZEND&PHP class */
PHP_MINIT_FUNCTION(midgard2_g_mainloop)
{
	static zend_function_entry midgard_g_mainloop_methods[] = {
		PHP_ME(midgard_g_mainloop,	__construct,	arginfo_midgard_g_mainloop___construct,	ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PHP_ME(midgard_g_mainloop,	run,		arginfo_midgard_g_mainloop_run,		ZEND_ACC_PUBLIC)
		PHP_ME(midgard_g_mainloop,	is_running,	arginfo_midgard_g_mainloop_is_running,	ZEND_ACC_PUBLIC)
		PHP_ME(midgard_g_mainloop,	quit,		arginfo_midgard_g_mainloop_quit,	ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	static zend_class_entry php_midgard_g_mainloop_class_entry;
	INIT_CLASS_ENTRY(php_midgard_g_mainloop_class_entry, "midgard_g_main_loop", midgard_g_mainloop_methods);

	php_midgard_g_mainloop_class = zend_register_internal_class(&php_midgard_g_mainloop_class_entry TSRMLS_CC);
	/* Set function to initialize underlying data */
	php_midgard_g_mainloop_class->create_object = php_midgard_gobject_new;

	return SUCCESS;
}
