--TEST--
test of midgard_connection in CLI-mode
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
report_memleaks = On
--FILE--
<?php
$cfg = new midgard_config();
$cfg->read_file_at_path(dirname(__FILE__).'/test.cfg');

$mgd = midgard_connection::get_instance();
var_dump($mgd instanceof midgard_connection);
var_dump($mgd->config === null);

$str1 = var_export($mgd, true);
$mgd = midgard_connection::get_instance();
$str2 = var_export($mgd, true);
var_dump($str1 === $str2);

var_dump($mgd->open_config($cfg));

$manager = $mgd->get_content_manager();
var_dump(is_object($manager));
var_dump($manager instanceof MidgardSqlContentManager);
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
<<<<<<< HEAD
=======
bool(true)
bool(true)
>>>>>>> gjallarhorn
bool(true)
bool(true)
===DONE===
