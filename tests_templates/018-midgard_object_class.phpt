--TEST--
test midgard_object_class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
$obj = midgard_object_class::factory('atype');
$obj->a = 'a';
$obj->b = 'b';
$obj->create();

$obj2 = midgard_object_class::factory('atype', $obj->id);

var_dump($obj->a === $obj2->a);
var_dump($obj->b === $obj2->b);
var_dump($obj->guid === $obj2->guid);

$obj3 = midgard_object_class::get_object_by_guid($obj->guid);

var_dump($obj->a === $obj3->a);
var_dump($obj->b === $obj3->b);
var_dump($obj->guid === $obj3->guid);

// do not leave traces!
$obj->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
