--TEST--
test form midgard_dbobject::is_in_parent_tree()
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php

$obj1 = new atype();
$obj1->a = '1';
$obj1->create();

$obj2 = new xtype();
$obj2->x = '2';
$obj2->owner = $obj1->id;
$obj2->create();

var_dump($obj1, $obj2);

$dummy = new xtype();
var_dump($dummy->is_in_parent_tree($obj1->id, $obj2->id));

// cleanup
$obj1->delete();
$obj2->delete();

?>
===DONE===
--EXPECTF--
bool(true)
===DONE===
