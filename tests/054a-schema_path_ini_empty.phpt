--TEST--
test for schema_path ini-setting
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
midgard.schema_path = ''
display_startup_errors = On
report_memleaks = On
--FILE--
<?php
var_dump(ini_get('midgard.schema_path'));
?>
===DONE===
--EXPECTF--
midgard-core (pid:%d):(WARNING):\'\'/schema doesn't seem to be schema directory

Warning: Failed to read schema from given '\'\'' directory. in Unknown on line %d
string(4) "\'\'"
===DONE===
