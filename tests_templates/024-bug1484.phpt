--TEST--
test for bug#1484
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
$a->flt = 0.12345;
$a->create();

$a2 = new atype($a->guid);
var_dump($a2->flt);

unset($a2);

$qb = atype::new_query_builder();
$qb->add_constraint('guid', '=', $a->guid);

$items = $qb->execute();
$a3 = $items[0];
var_dump($a3->flt);

var_dump($a->delete());
?>
===DONE===
--EXPECTF--
float(0.12345)
float(0.12345)
bool(true)
===DONE===
