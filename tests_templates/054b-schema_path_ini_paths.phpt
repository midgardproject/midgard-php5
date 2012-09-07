--TEST--
test for schema_path ini-setting
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
midgard.schema_path = [[PATH]]/test_data/schemas/a;[[PATH]]/test_data/schemas/b
display_startup_errors = On
report_memleaks = On
--FILE--
<?php
var_dump(class_exists('test_a_type'));
var_dump(class_exists('test_b_type'));
var_dump(ini_get('midgard.schema_path'));
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
string(%d) "[[PATH]]/test_data/schemas/a;[[PATH]]/test_data/schemas/b"
===DONE===
