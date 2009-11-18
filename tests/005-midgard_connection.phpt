--TEST--
test of midgard_connection in CLI-mode
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
--FILE--
<?php
$cfg = new midgard_config();
$cfg->read_file_at_path(dirname(__FILE__).'/test.cfg');

$mgd = midgard_connection::get_instance();
var_dump($mgd);

$str1 = var_export($mgd, true);
$mgd = midgard_connection::get_instance();
$str2 = var_export($mgd, true);
var_dump($str1 === $str2);

var_dump($mgd->open_config($cfg));

?>
===DONE===
--EXPECTF--
object(midgard_connection)#%d (0) {
}
bool(true)
bool(true)
===DONE===
