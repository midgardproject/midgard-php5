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

php_stream * php_midgard2stream_opener(php_stream_wrapper *wrapper, char *filename, char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	return NULL;
}

int php_midgard2stream_closer(php_stream *stream, int close_handle TSRMLS_DC)
{
	return 0;
}

size_t php_midgard2stream_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
	return 0;
}

size_t php_midgard2stream_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	return 0;
}

int php_midgard2stream_flush(php_stream *stream TSRMLS_DC)
{
	return 0;
}

int php_midgard2stream_seek(php_stream *stream, off_t offset, int whence, off_t *newoffset TSRMLS_DC)
{
	return 0;
}
