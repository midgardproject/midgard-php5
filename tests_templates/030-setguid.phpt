--TEST--
test for midgard_dbobject::set_guid()
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--FILE--
<?php

$test_guid = 'f0000000000000000000000000000000000f';

$obj1 = new atype();
$obj1->a = '1';

var_dump($obj1->set_guid($test_guid));
var_dump($obj1->guid === $test_guid);

var_dump($obj1->create());

var_dump($obj1->guid === $test_guid);

// cleanup
var_dump($obj1->delete());
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
