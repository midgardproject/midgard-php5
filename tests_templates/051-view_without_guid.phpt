--TEST--
test if views without guids work fine
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
$obj1 = new atype();
$obj1->a = 'a';
$obj1->b = 'b';
$obj1->create();

$obj2 = new xtype();
$obj2->x = 'x';
$obj2->y = 'y';
$obj2->owner = $obj1->id;
$obj2->create();


$storage = new midgard_query_storage('noguidview');
$q = new midgard_query_select($storage);
var_dump($q->execute());

$list = $q->list_objects();
var_dump(count($list) == 1);

// do not leave traces!
var_dump($obj2->delete());
var_dump($obj1->delete());
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
