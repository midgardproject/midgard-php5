--TEST--
test for setting values of datetime properties
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
$a->a = 'a';
$a->create();

var_dump($a->metadata->published->format('Y-m-d') == '0001-01-01');
var_dump($a->metadata->published->format('Y-m-d') == '0001-01-01');
$a->metadata->published->setDate(2011, 01, 01);
var_dump($a->metadata->published->format('Y-m-d') == '2011-01-01');
$a->metadata->published->setTimestamp(time());
var_dump($a->metadata->published->format('Y-m-d') == gmdate('Y-m-d'));

$a->delete();
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
