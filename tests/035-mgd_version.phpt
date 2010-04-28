--TEST--
mgd_version tests
--SKIPIF--
<?php
if (!extension_loaded("midgard2")) die("skip midgard2 extension is required");
?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
--FILE--
<?php
var_dump(is_string(mgd_version()));
var_dump(mgd_version(null));
var_dump(mgd_version('hi'));

?>
===DONE===
--EXPECTF--
bool(true)

Warning: %s in %s on line %d
NULL

Warning: %s in %s on line %d
NULL
===DONE===
