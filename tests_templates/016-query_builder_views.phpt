--TEST--
test that query_builder returns proper objects, when query is executed against view
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
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

$query = new midgard_query_builder('vview');

var_dump($query->count());

$res = $query->execute();
var_dump($res[0]->a === $obj1->a);
var_dump($res[0]->b === $obj1->b);
var_dump($res[0]->x === $obj2->x);
var_dump($res[0]->y === $obj2->y);
var_dump($res[0]->guid === $obj1->guid);

// do not leave traces!
var_dump($obj2->delete());
var_dump($obj1->delete());
?>
===DONE===
--EXPECTF--
int(1)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
