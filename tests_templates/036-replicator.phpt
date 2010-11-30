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
report_memleaks = On
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

var_dump($obj1->metadata->exported->format('Y-m-d H:i:s') == '0001-01-01 00:00:00');

var_dump(midgard_replicator::export($obj1));
// should be real date, a bit in the past
var_dump($obj1->metadata->exported->format('Y-m-d H:i:s') != '0001-01-01 00:00:00');

// test blob
$att = new midgard_attachment();
$att->location = 'test.txt';
$att->name = 'test.txt';
$att->title = 'Some text-file';
$att->parentguid = $obj1->guid;
$res = $att->create();

if (false === $res) {
    var_dump(midgard_connection::get_instance()->get_error_string());
}

$blob = new midgard_blob($att);

$handler = $blob->get_handler();
fwrite($handler, 'Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.');
fclose($handler);

$xml = simplexml_load_string(midgard_replicator::serialize_blob($att));
var_dump(isset($xml->midgard_blob));

$attrs = $xml->midgard_blob->attributes();
var_dump(isset($attrs['guid']));
var_dump($attrs['guid'] == $att->guid);

echo "==delete==\n";
$blob->remove_file();
$att->delete();
var_dump($obj1->delete());
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
==delete==
bool(true)
===DONE===
