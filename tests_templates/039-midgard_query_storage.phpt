--TEST--
tests for midgard_query_storage class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--FILE--
<?php
try {
    // non existant class
    $storage = new midgard_query_storage('non_existant_class');
    echo "error\n";
} catch (midgard_error_exception $e) {
    echo "ok\n";
}

try {
    // bad class
    $storage = new midgard_query_storage('midgard_connection');
    echo "error\n";
} catch (midgard_error_exception $e) {
    echo "ok\n";
}

// good classes
$storage = new midgard_query_storage('atype');
var_dump($storage->dbclass);

$storage2 = new midgard_query_storage('xtype');
var_dump($storage2->dbclass);

// view
$storage3 = new midgard_query_storage('vview');
var_dump($storage3->dbclass);

?>
===DONE===
--EXPECTF--
Warning: GLib: %s in %s
ok

Warning: GLib: %s in %s
ok
string(5) "atype"
string(5) "xtype"
string(5) "vview"
===DONE===
