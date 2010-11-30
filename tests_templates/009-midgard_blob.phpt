--TEST--
test of midgard_blob class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[PATH]]/test.cfg
report_memleaks = On
--FILE--
<?php
$cfg = new midgard_config();
$cfg->read_file_at_path(dirname(__FILE__).'/test.cfg');
$cfg->blobdir = '[[BLOB_PATH]]';

$mgd = midgard_connection::get_instance();
$mgd->open_config($cfg);

$att = new midgard_attachment();
$att->location = 'test.txt';
$att->name = 'test.txt';
$att->title = 'Some text-file';

$blob = new midgard_blob($att);

$handler = $blob->get_handler();
fwrite($handler, 'Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.');
fclose($handler);

var_dump($blob->exists());

var_dump($blob->get_path() === '[[BLOB_PATH]]/test.txt');

$txt1 = file_get_contents('[[BLOB_PATH]]/test.txt');
$txt2 = $blob->read_content();
var_dump($txt1 === $txt2);

$handler = $blob->get_handler('r');
$txt3 = stream_get_contents($handler);
var_dump($txt1 === $txt3);
fclose($handler);

$res = $blob->remove_file();
var_dump($res);

var_dump($blob->exists());

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
===DONE===
