--TEST--
test of midgard_dbus class
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

try {
    $dbus = new midgard_dbus('/test');
    echo "MISSED ERROR\n";
} catch (Exception $e) {
}

try {
    $mgd = midgard_connection::get_instance();
    $dbus = new midgard_dbus('/test');
} catch (Exception $e) {
    echo "UNEXPECTED ERROR\n";
}

?>
===DONE===
--EXPECTF--
===DONE===
