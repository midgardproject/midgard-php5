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

#ifndef MGD_PREPARSER_H
#define MGD_PREPARSER_H
#include "php_midgard.h"

extern MGD_FUNCTION(ret_type, template, (type param));
extern MGD_FUNCTION(ret_type, snippet, (type param));
extern MGD_FUNCTION(ret_type, snippet_required, (type param));
extern MGD_FUNCTION(ret_type, ref, (type param));
extern MGD_FUNCTION(ret_type, eval, (type param));
extern MGD_FUNCTION(ret_type, is_element_loaded, (type param));

#if MIDGARD_142MOD

extern MGD_FUNCTION(ret_type, variable, (type param));

char * php_midgard_variable(midgard_pool * pool, char * name, char * member, char * type);
char * php_eval_midgard(midgard_pool * pool, const char *name, char *value, int exit_php);
int mgdparse(void);

extern GByteArray *mgd_output_buffer;
#define mgd_append_byte(buffer, str) if((*str) != '\0' )\
	g_byte_array_append((buffer), (str), 1);
#define mgd_set_buffer(buffer, str) { \
	(buffer) = g_byte_array_new(); \
	if(strlen(str) > 0 ) { \
		g_byte_array_append((buffer), (str), strlen(str)); \
	} \
}

#define mgd_free_buffer(buffer) {if(buffer) g_byte_array_free((buffer), TRUE);}
#define mgd_append_buffer(buffer, str) { if(str && strlen(str) > 0 )\
	g_byte_array_append((buffer), (str), strlen(str)); }
#define mgd_append_output_buffer_const(str) { if(sizeof(str) > 0 )\
	g_byte_array_append(mgd_output_buffer, (str), (sizeof(str) == 1) ? 1 : sizeof(str)-1 ); }
#define mgd_append_output_buffer(str) { if(str && strlen(str) > 0 )\
	g_byte_array_append(mgd_output_buffer, (str), strlen(str)); }
#define mgd_append_output_buffer_data(gstr) { if(gstr && (gstr)->len > 0 )\
	g_byte_array_append(mgd_output_buffer, (gstr)->data, (gstr)->len); }

#endif /* MIDGARD_142MOD */

//#define MGD_PREPARSER_LOG

#endif
