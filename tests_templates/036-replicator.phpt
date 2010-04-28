--TEST--
test for midgard_replicator
--SKIPIF--
<?php
if (!extension_loaded('midgard2')) print "skip";
if (!extension_loaded('simplexml')) print 'skip simplexml extension is required to test replicator output';
?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php

$obj1 = new atype();
$obj1->a = 'a';
$obj1->create();

$xml = simplexml_load_string(midgard_replicator::serialize($obj1));

var_dump(isset($xml->atype));
var_dump($xml->atype->a == $obj1->a);

$attrs = $xml->atype->attributes();
var_dump($attrs['guid'] == $obj1->guid);
var_dump($attrs['action'] == 'created');

$obj1->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
