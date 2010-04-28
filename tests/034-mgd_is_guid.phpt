--TEST--
mgd_is_guid tests
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
var_dump(mgd_is_guid());
var_dump(mgd_is_guid(123));
var_dump(mgd_is_guid(null));
var_dump(mgd_is_guid('f0000000000000000000000000000000000f'));

?>
===DONE===
--EXPECTF--
Warning: %s in %s on line %d
NULL
bool(false)
bool(false)
bool(true)
===DONE===
