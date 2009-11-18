--TEST--
test for an error, which should happen in absence of connection.
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
--FILE--
<?php
$host = new midgard_style();
?>
===DONE===
--EXPECTF--
Fatal error: Can not find MidgardConnection in %s/004-work_without_connection.php on line %d
