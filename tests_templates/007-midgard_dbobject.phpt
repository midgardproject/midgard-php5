--TEST--
test of midgard_dbobject
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file = [[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
$obj = new atype();
$obj->a = 'test';
var_dump($obj->a);
var_dump($obj->guid);

$obj->create();
var_dump($obj->a);
var_dump($obj->guid);

$obj2 = new atype();
$obj2->up = $obj->id;
$obj2->a = 'kid';
$obj2->create();


$list = $obj->list();
var_dump(count($list));

var_dump($list[0]->a === $obj2->a);
var_dump($list[0]->guid === $obj2->guid);

$obj3 = new xtype();
$obj3->x = 'outer';
$obj3->owner = $obj2->id;
$obj3->create();

$list = $obj2->list_children('xtype');
var_dump(count($list));

var_dump($list[0]->x === $obj3->x);
var_dump($list[0]->guid === $obj3->guid);


// don't leave traces!
var_dump($obj->delete()); // <-- this should fail because of dependency
var_dump($obj3->delete());
var_dump($obj2->delete());
var_dump($obj->delete());
?>
===DONE===
--EXPECTF--
string(4) "test"
string(0) ""
string(4) "test"
string(36) "%s"
int(1)
bool(true)
bool(true)
int(1)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
===DONE===
