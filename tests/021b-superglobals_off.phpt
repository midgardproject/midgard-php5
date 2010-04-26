--TEST--
midgard_datetime tests
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
midgard.superglobals_compat = Off
display_startup_errors = On
--FILE--
<?php
function mytest()
{
    $_MIDGARD = array();
    $_MIDCOM = array();
    $_MIDGARD_CONNECTION = array();
}
mytest();
var_dump(isset($_MIDGARD));
var_dump(isset($_MIDCOM));
var_dump(isset($_MIDGARD_CONNECTION));
?>
===DONE===
--EXPECTF--
bool(false)
bool(false)
bool(false)
===DONE===
