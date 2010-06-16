--TEST--
tests for joins midgard_query_select class
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
$obj1 = new atype();
$obj1->a = 'abc';
$obj1->b = 'def';
$obj1->create();

$obj2 = new atype();
$obj2->a = 'jkl';
$obj2->b = 'mno';
$obj2->create();

$obj3 = new xtype();
$obj3->owner = $obj1->id;
$obj3->x = 'abc';
$obj3->y = 'ghi';
$obj3->create();

$obj4 = new xtype();
$obj4->owner = $obj1->id;
$obj4->x = 'jkl';
$obj4->y = 'pqr';
$obj4->create();


$storage = new midgard_query_storage('atype');
$xstorage = new midgard_query_storage('xtype');

$property = new midgard_query_property('a', $storage);
$xproperty = new midgard_query_property('x', $xstorage);
$yproperty = new midgard_query_property('y', $xstorage);

$q = new midgard_query_select($storage);
$q->add_join('left', $property, $xproperty);
$q->set_constraint(new midgard_query_constraint($yproperty, '=', new midgard_query_value('ghi')));
$q->execute();

var_dump($q->get_results_count() == 1);
$list = $q->list_objects();

var_dump($list[0]->a == 'abc');
var_dump($list[0]->b == 'def');

echo "==delete==\n";
var_dump($obj3->delete());
var_dump($obj4->delete());
var_dump($obj1->delete());
var_dump($obj2->delete());

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
==delete==
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
