--TEST--
test for gh-bug #20 (inconsistent constructor exceptions)
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
try {
    $obj = new atype(false);
    echo "ERROR\n";
} catch (InvalidArgumentException $e) {
    echo "OK\n";
}

try {
    $obj = new atype('2f');
    echo "ERROR\n";
} catch (InvalidArgumentException $e) {
    echo "OK\n";
}

$a = new atype();
$a->create();

try {
    $obj = new atype($a->id);
    $obj = null;
    echo "OK\n";
} catch (Exception $e) {
    echo "ERROR\n";
}

try {
    $obj = new atype($a->id + 1);
    echo "ERROR\n";
} catch (midgard_error_exception $e) {
    echo "OK\n";
}

// cleanup
$a->delete();

?>
===DONE===
--EXPECTF--
OK
OK
OK
OK
===DONE===
