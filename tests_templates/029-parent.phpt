--TEST--
test for midgard_dbobject::parent() and midgard_dbobject::get_parent() methods
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

var_dump($obj2->parent() == 'atype');
var_dump($obj2->get_parent() instanceof atype);
var_dump($obj2->get_parent()->a == '1');

// cleanup
$obj1->delete();
$obj2->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
===DONE===
