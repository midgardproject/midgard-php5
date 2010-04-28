/*
 * Copyright (C) 2007 Piotr Pokora <piotrek.pokora@gmail.com>
 *
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

PHP_FUNCTION(_midgard_php_object_constructor);
/* FIXME , change name */
PHP_FUNCTION(_midgard_php_object_get_by_id);
PHP_FUNCTION(_midgard_php_object_get_by_guid);
PHP_FUNCTION(_midgard_php_object_update);
PHP_FUNCTION(_midgard_php_object_create);
PHP_FUNCTION(_midgard_php_object_is_in_parent_tree);
PHP_FUNCTION(_midgard_php_object_is_in_tree);
PHP_FUNCTION(_midgard_php_object_get_tree);
PHP_FUNCTION(_midgard_php_object_delete);
PHP_FUNCTION(_midgard_php_object_get_parent);
PHP_FUNCTION(_midgard_php_object_list);
PHP_FUNCTION(_midgard_php_object_list_children);
PHP_FUNCTION(_midgard_php_object_get_by_path);
PHP_FUNCTION(_midgard_php_object_parent);
PHP_FUNCTION(_php_midgard_object_get_languages);
PHP_FUNCTION(_php_midgard_object_purge);
PHP_FUNCTION(_php_midgard_object_undelete);
PHP_FUNCTION(_php_midgard_object_export);
PHP_FUNCTION(_php_midgard_object_parameter);

/* parameters */
PHP_FUNCTION(php_midgard_object_has_parameters);
PHP_FUNCTION(_php_midgard_object_parameter);
PHP_FUNCTION(_php_midgard_object_get_parameter);
PHP_FUNCTION(_php_midgard_object_set_parameter);
PHP_FUNCTION(_php_midgard_object_list_parameters);
PHP_FUNCTION(_php_midgard_object_delete_parameters);
PHP_FUNCTION(_php_midgard_object_purge_parameters);
PHP_FUNCTION(_php_midgard_object_find_parameters);

/* attachments */
PHP_FUNCTION(php_midgard_object_has_attachments);
PHP_FUNCTION(_php_midgard_object_create_attachment);
PHP_FUNCTION(_php_midgard_object_serve_attachment);
PHP_FUNCTION(_php_midgard_object_list_attachments);
PHP_FUNCTION(_php_midgard_object_delete_attachments);
PHP_FUNCTION(_php_midgard_object_purge_attachments);
PHP_FUNCTION(_php_midgard_object_find_attachments);
