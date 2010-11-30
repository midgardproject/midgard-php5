--TEST--
test for gh-bug #14
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
// shouldn't echo any warning
$qb = new midgard_query_builder('midgard_user');
?>
===DONE===
--EXPECTF--
===DONE===
