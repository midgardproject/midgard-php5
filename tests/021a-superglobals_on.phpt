--TEST--
midgard_datetime tests
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
midgard.superglobals_compat = On
display_startup_errors = On
report_memleaks = On
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
%s midgard.superglobals_compat option is deprecated and might be removed in next release in %s
bool(true)
bool(true)
bool(true)
===DONE===
