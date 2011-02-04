--TEST--
test for gh-bug #26 (metadata problem, when calling isset)
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = On
--FILE--
<?php
$a = new atype();
$a->create();

$obj = new atype($a->guid);

var_dump(isset($obj->metadata));
var_dump(property_exists($obj->metadata, 'schedulestart'));
var_dump(property_exists($obj->metadata, 'does_not_exist'));

// cleanup
$a->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(false)
===DONE===
