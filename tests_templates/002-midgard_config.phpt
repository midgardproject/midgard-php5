--TEST--
midgard_config tests
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
--FILE--
<?php
$cfg = new midgard_config();
var_dump($cfg->read_file_at_path('[[CFG_FILE]]'));

var_dump($cfg->dbtype === '[[DB_TYPE]]');
var_dump($cfg->database === '[[DB_NAME]]');
var_dump($cfg->logfilename === '[[PATH]]/midgard.log');
var_dump($cfg->loglevel === 'warn');
var_dump($cfg->tablecreate === true);
var_dump($cfg->tableupdate === true);
var_dump($cfg->testunit === false);
var_dump($cfg->blobdir);
var_dump($cfg->sharedir);
var_dump($cfg->vardir === 'test');
var_dump($cfg->cachedir);
var_dump($cfg->midgardusername === 'root');
var_dump($cfg->midgardpassword === 'password');
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
string(%d) "%s"
string(%d) "%s"
bool(true)
string(%d) "%s"
bool(true)
bool(true)
===DONE===
