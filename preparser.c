/* $Id$
Copyright (C) 1999 Jukka Zitting <jukka.zitting@iki.fi>
Copyright (C) 2000 The Midgard Project ry
Copyright (C) 2000 Emile Heyns, Aurora SA <emile@iris-advies.com>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "php_midgard.h"
#include "zend_highlight.h"
#include "ext/standard/basic_functions.h"
#include "mgd_preparser.h"

MGD_FUNCTION(ret_type, is_element_loaded, (type param))
{
	/* TODO, rewrite me */
}

MGD_FUNCTION(ret_type, template, (type param))
{
	char *tmp = NULL;
	int tmp_len;

	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &tmp_len, &tmp_len) == FAILURE)
		return;

	if (tmp == NULL) {
		RETVAL_STRING("",1);
	} else {
		RETVAL_STRING("",1);
	}
}

MGD_FUNCTION(ret_type, snippet, (type param))
{
	MidgardConnection *mgd = mgd_handle();
	const gchar *path;
	guint pathl;
	MgdObject *object;

	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &pathl)  == FAILURE) {
		return;
	}

	if ((object = midgard_object_class_get_object_by_path(mgd, "midgard_snippet", path)) != NULL) {
		gchar *code = NULL;
		g_object_get(G_OBJECT(object), "code", &code, NULL);

		if (!code)
			code = "";

		RETVAL_STRING(code,1);
		g_object_unref(object);
	} else {
		/* Keep it commented right now.
		 * zend_error(E_WARNING, "Could not load requested snippet://%s.", path);
		 */
		RETVAL_STRING("", 1);
	}
}

MGD_FUNCTION(ret_type, snippet_required, (type param))
{
	MidgardConnection *mgd = mgd_handle();
	const gchar *path;
	guint pathl;
	MgdObject *object;

	CHECK_MGD;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &pathl)  == FAILURE) {
		return;
	}

	if ((object = midgard_object_class_get_object_by_path(mgd, "midgard_snippet", path)) != NULL) {
		gchar *code;
		g_object_get(G_OBJECT(object), "code", &code, NULL);
		RETVAL_STRING(code,1);
		g_object_unref(object);
	} else {
		php_error(E_ERROR, "Could not load requested snippet://%s.", path);
		RETVAL_STRING("", 1);
	}
}
