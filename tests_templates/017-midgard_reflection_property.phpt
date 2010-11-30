--TEST--
test midgard_reflection_property
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
// testing direct instantiation
$rp1 = new midgard_reflection_property('atype');

var_dump($rp1->description('id') == 'Local non-replication-safe database identifier');
var_dump($rp1->description('a') == 'A-field');
var_dump($rp1->description('b') == 'B-field');

var_dump($rp1->get_midgard_type('id') == MGD_TYPE_UINT);
var_dump($rp1->get_midgard_type('a') == MGD_TYPE_STRING);
var_dump($rp1->get_midgard_type('b') == MGD_TYPE_STRING);


// testing instantiation from object
$obj = new atype();
$rp2 = $obj->new_reflection_property();

var_dump($rp2->description('id') == 'Local non-replication-safe database identifier');
var_dump($rp2->description('a') == 'A-field');
var_dump($rp2->description('b') == 'B-field');

var_dump($rp2->get_midgard_type('id') == MGD_TYPE_UINT);
var_dump($rp2->get_midgard_type('a') == MGD_TYPE_STRING);
var_dump($rp2->get_midgard_type('b') == MGD_TYPE_STRING);

// testing renamed class
$rp3 = new midgard_reflection_property('midgard_metadata');

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
===DONE===
