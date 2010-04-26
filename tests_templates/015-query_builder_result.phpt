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
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
$obj = new atype();
$obj->a = 'example.com';
$obj->create();

$string1 = var_export($obj, true);

$obj = new atype($obj->id);
$string2 = var_export($obj, true);

$query = new midgard_query_builder('atype');
var_dump($query->count() == 1);
$res = $query->execute();

$string3 = var_export($res[0], true);

$query2 = atype::new_query_builder();
var_dump($query2->count() == 1);
$res = $query2->execute();

$string4 = var_export($res[0], true);

$obj->delete();

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
===DONE===
