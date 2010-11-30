--TEST--
test for bug#1492
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=doesnotexist
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = On
--FILE--
<?php
$mgd = midgard_connection::get_instance();
var_dump($mgd instanceof midgard_connection);
?>
===DONE===
--EXPECTF--
bool(true)
===DONE===
