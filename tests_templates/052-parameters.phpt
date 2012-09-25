--TEST--
test for setting/getting values of parameters
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

var_dump($a->set_parameter('foo', 'bar', 'baz'));
var_dump($a->get_parameter('foo', 'bar'));
var_dump($a->parameter('foo', 'bar'));
var_dump($a->parameter('foo', 'bar', null));
var_dump($a->get_parameter('foo', 'bar'));

$a->delete();
?>
===DONE===
--EXPECTF--
bool(true)
string(3) "baz"
string(3) "baz"
bool(true)
NULL
===DONE===
