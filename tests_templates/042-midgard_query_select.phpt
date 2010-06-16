--TEST--
tests for midgard_query_select class
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
$storage = new midgard_query_storage('atype');
$property = new midgard_query_property('a', $storage);
$value = new midgard_query_value('abc');

$constraint = new midgard_query_constraint($property, '=', $value);

$obj1 = new atype();
$obj1->a = 'abc';
$obj1->b = 'def';
$obj1->create();

$obj2 = new atype();
$obj2->a = 'abc';
$obj2->b = 'ghi';
$obj2->create();

$obj3 = new atype();
$obj3->a = 'jkl';
$obj3->b = 'ghi';
$obj3->create();

$q = new midgard_query_select($storage);
var_dump($q->execute());

var_dump($q->get_results_count() == 3);

$list = $q->list_objects();
var_dump(is_array($list));
var_dump($list[0] instanceof atype);
var_dump($list[1] instanceof atype);
var_dump($list[2] instanceof atype);

var_dump($q->set_constraint($constraint));
var_dump($q->execute());
var_dump($q->get_results_count() == 2);
$list = $q->list_objects();
var_dump($list[0]->a == 'abc');
var_dump($list[1]->a == 'abc');


$group = new midgard_query_constraint_group(
    'AND',
    new midgard_query_constraint(new midgard_query_property('a', $storage), '=', new midgard_query_value('abc')),
    new midgard_query_constraint(new midgard_query_property('b', $storage), '=', new midgard_query_value('ghi'))
);

var_dump($q->set_constraint($group));
var_dump($q->execute());
var_dump($q->get_results_count() == 1);
$list = $q->list_objects();
var_dump($list[0]->a == 'abc');
var_dump($list[0]->b == 'ghi');
var_dump($list[0]->guid == $obj2->guid);

unset($group);

var_dump($q->execute());
var_dump($q->get_results_count() == 1);
$list = $q->list_objects();
var_dump($list[0]->a == 'abc');
var_dump($list[0]->b == 'ghi');
var_dump($list[0]->guid == $obj2->guid);

echo "==delete==\n";
var_dump($obj1->delete());
var_dump($obj2->delete());
var_dump($obj3->delete());

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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
==delete==
bool(true)
bool(true)
bool(true)
===DONE===
