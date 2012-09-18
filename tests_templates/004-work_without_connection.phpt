--TEST--
test for an error, which should happen in absence of connection.
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
try {
    $obj = new atype();
} catch (midgard_error_exception $e) {
    echo $e->getMessage()."\n";
}
?>
===DONE===
--EXPECTF--
CHECK_MGD - Failed to get connection
===DONE===
