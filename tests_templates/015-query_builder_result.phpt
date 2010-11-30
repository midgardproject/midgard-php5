--TEST--
test that query_builder really returns the proper object
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
// =========================================================================================
// = Order of fields might differ, so we compare sorted arrays of fiels instead of objects =
// =========================================================================================
$obj = new atype();
$obj->a = 'example.com';
$obj->create();

$arr = (array)$obj;
ksort($arr);
$string1 = var_export($arr, true);

$obj = new atype($obj->id);
$arr = (array)$obj;
ksort($arr);
$string2 = var_export($arr, true);

$query = new midgard_query_builder('atype');
var_dump($query->count() == 1);
$res = $query->execute();

$arr = (array)$res[0];
ksort($arr);
$string3 = var_export($arr, true);

$query2 = atype::new_query_builder();
var_dump($query2->count() == 1);
$res = $query2->execute();

$arr = (array)$res[0];
ksort($arr);
$string4 = var_export($arr, true);

var_dump($obj->delete());

var_dump($string1 === $string2);
var_dump($string2 === $string3);
var_dump($string3 === $string4);

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
