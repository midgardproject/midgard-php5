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
#include "php_midgard_urlwrapper.h"

static php_stream_ops php_midgard2stream_ops = {
	NULL, //php_midgard2stream_write,
	php_midgard2stream_read,
	php_midgard2stream_closer,
	NULL, //php_midgard2stream_flush,
	PHP_MIDGARD2_STREAMTYPE,
	php_midgard2stream_seek,
	NULL,
	NULL,
	NULL,
};

php_stream * php_midgard2stream_opener(php_stream_wrapper *wrapper, char *filename, char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	if (strncmp(filename, PHP_MIDGARD2_WRAPPER "://", strlen(PHP_MIDGARD2_WRAPPER) + 3) != 0) {
		php_stream_wrapper_log_error(wrapper, options TSRMLS_CC, "Invalid schema. midgard:// expected");
		return NULL;
	}

	php_midgard2stream_data *data = emalloc(sizeof(php_midgard2stream_data));
	data->obj = NULL; // TODO: place proper object here
	data->position = 0;

	return php_stream_alloc(&php_midgard2stream_ops, data, 0, mode);
}

int php_midgard2stream_closer(php_stream *stream, int close_handle TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;

	g_object_unref(data->obj);
	efree(data);

	return 0;
}

// size_t php_midgard2stream_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
// {
// 	php_midgard2stream_data *data = stream->abstract;
// 
// 	return 0;
// }

size_t php_midgard2stream_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;

	GValue pval = {0, };
	g_value_init(&pval, G_TYPE_STRING);

	g_object_get_property(G_OBJECT(data->obj), "code", &pval);

	const char *tmp = g_value_get_string(&pval);
	strncpy(buf, tmp + data->position, count);

	return 0;
}

// int php_midgard2stream_flush(php_stream *stream TSRMLS_DC)
// {
// 	php_midgard2stream_data *data = stream->abstract;
// 
// 	g_signal_emit(data->obj, MIDGARD_OBJECT_GET_CLASS(data->obj)->signal_action_update_hook, 0);
// 
// 	if (!midgard_object_update(data->obj)) {
// 		// FIXME: do something more meaningful
// 		php_error(E_WARNING, "failed to flush stream");
// 	}
// 
// 	return 0;
// }

int php_midgard2stream_seek(php_stream *stream, off_t offset, int whence, off_t *newoffset TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;

	switch (whence) {
		case SEEK_SET:
			data->position = offset;
			break;

		case SEEK_CUR:
			data->position += offset;
			break;

		case SEEK_END:
		{
			GValue pval = {0, };
			g_value_init(&pval, G_TYPE_STRING);

			g_object_get_property(G_OBJECT(data->obj), "code", &pval);

			data->position = strlen(g_value_get_string(&pval)) + offset;
			break;
		}
	}

	if (data->position < 0) {
		data->position = 0;
	}

	if (newoffset) {
		*newoffset = data->position;
	}

	return 0;
}
