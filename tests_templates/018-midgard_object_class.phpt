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
report_memleaks = On
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

var_dump(midgard_object_class::get_property_up('atype') === 'up');
var_dump(midgard_object_class::get_property_up($obj)    === 'up');
var_dump(midgard_object_class::get_property_up('xtype') === NULL);

$objx = new xtype();

var_dump(midgard_object_class::get_property_parent('xtype') === 'owner');
var_dump(midgard_object_class::get_property_parent($objx)   === 'owner');
var_dump(midgard_object_class::get_property_parent('atype') === NULL);

$guid = $obj->guid;
$obj->delete();

//var_dump(midgard_object_class::undelete($guid));
//$obj = new atype($guid);

var_dump(midgard_object_class::has_metadata($obj));
var_dump(midgard_object_class::has_metadata('atype'));
var_dump(midgard_object_class::has_metadata('midgard_metadata'));

var_dump(midgard_object_class::get_schema_value('atype', 'meta1') === 'value1');
var_dump(midgard_object_class::get_schema_value('atype', 'meta2') === 'value2');
var_dump(midgard_object_class::get_schema_value('atype', 'meta3') === NULL);

// cleanup
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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
===DONE===
