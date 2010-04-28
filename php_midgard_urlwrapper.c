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
	php_midgard2stream_write,
	php_midgard2stream_read,
	php_midgard2stream_closer,
	php_midgard2stream_flush,
	PHP_MIDGARD2_STREAMTYPE,
	php_midgard2stream_seek,
	NULL,
	NULL, // php_midgard2stream_stat,
	NULL,
};

static php_stream_wrapper_ops php_midgard2stream_wrapper_ops = {
	php_midgard2stream_opener,
	NULL, /* will call underlying closer */
	NULL,
	NULL,
	NULL,
	PHP_MIDGARD2_WRAPPER,
	NULL,
	NULL,
	NULL,
	NULL,
};

static php_stream_wrapper php_midgard2stream_wrapper = {
	&php_midgard2stream_wrapper_ops,
	NULL,
	0,
};

php_stream * php_midgard2stream_opener(php_stream_wrapper *wrapper, char *filename, char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	if (strncmp(filename, PHP_MIDGARD2_WRAPPER "://", strlen(PHP_MIDGARD2_WRAPPER) + 3) != 0) {
		php_stream_wrapper_log_error(wrapper, options TSRMLS_CC, "Invalid schema. midgard:// expected");
		return NULL;
	}

	if (options & STREAM_OPEN_PERSISTENT) {
		php_stream_wrapper_log_error(wrapper, options TSRMLS_CC, "Unable to open %s persistently", filename);
		return NULL;
	}

	const char *path = filename + (strlen(PHP_MIDGARD2_WRAPPER) + 3);

	php_midgard2stream_data *data = emalloc(sizeof(php_midgard2stream_data));
	data->obj = midgard_schema_object_factory_get_object_by_path(mgd_handle(), "midgard_snippet", path);
	data->position = 0;

	if (data->obj == NULL) {
		efree(data);
		php_stream_wrapper_log_error(wrapper, options TSRMLS_CC, "Couldn't find midgard_snippet object using %s path", path);
		return NULL;
	}

	if (mode[0] == 'w') {
		data->buffer = NULL;
		data->size = 0;
	} else {
		GValue pval = {0, };
		g_value_init(&pval, G_TYPE_STRING);

		g_object_get_property(G_OBJECT(data->obj), "code", &pval);
		const gchar *tmp_string = g_value_get_string(&pval);

		data->buffer = estrdup(tmp_string);
		data->size = strlen(tmp_string);

		if (mode[0] == 'a') {
			data->position = data->size;
		}
	}

	return php_stream_alloc(&php_midgard2stream_ops, data, 0, mode);
}

int php_midgard2stream_closer(php_stream *stream, int close_handle TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;

	if (data->obj) {
		g_object_unref(data->obj);
	}

	if (data->buffer) {
		efree(data->buffer);
	}

	efree(data);

	return 0;
}

size_t php_midgard2stream_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;
	size_t new_size = data->position + count + 1;

	if (data->size < new_size) {
		data->buffer = erealloc(data->buffer, new_size);
		data->size = new_size;
	}

	memcpy(data->buffer + data->position, buf, count);
	data->position += count;
	data->buffer[data->position] = '\0';

	return count;
}

size_t php_midgard2stream_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;
	const char *tmp = data->buffer;

	size_t to_read = count;

	if (data->position + count > data->size) {
		to_read = data->size - data->position;
	}

	memcpy(buf, tmp + data->position, to_read);
	data->position += to_read;

	return to_read;
}

int php_midgard2stream_flush(php_stream *stream TSRMLS_DC)
{
	php_midgard2stream_data *data = stream->abstract;

	{
		GValue pval = {0, };
		g_value_init(&pval, G_TYPE_STRING);
		g_value_set_string(&pval, data->buffer);

		g_object_set_property(G_OBJECT(data->obj), "code", &pval);
		g_value_unset(&pval);
	}

	g_signal_emit(data->obj, MIDGARD_OBJECT_GET_CLASS(data->obj)->signal_action_update_hook, 0);
	if (!midgard_object_update(data->obj)) {
		// FIXME: do something more meaningful
		php_error(E_WARNING, "failed to flush stream");
	}

	return 0;
}

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

// int php_midgard2stream_stat(php_stream *stream, php_stream_statbuf *ssb TSRMLS_DC)
// {
// 	return 0;
// }


PHP_MINIT_FUNCTION(midgard2_urlwrapper)
{
	return php_register_url_stream_wrapper(PHP_MIDGARD2_WRAPPER, &php_midgard2stream_wrapper TSRMLS_CC);
}

PHP_MSHUTDOWN_FUNCTION(midgard2_urlwrapper)
{
	return php_unregister_url_stream_wrapper(PHP_MIDGARD2_WRAPPER TSRMLS_CC);
}
