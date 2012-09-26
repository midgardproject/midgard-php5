--TEST--
midgard_datetime tests (additional test for php 5.3)
--SKIPIF--
<?php
    if (!extension_loaded("midgard2"))
        die("skip midgard2 extension is required");

    if (version_compare(PHP_VERSION, '5.3.0') !== 1)
        die("skip php-5.3 is required");
?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
report_memleaks = On
--FILE--
<?php
// testing chain-calls
$date = new midgard_datetime();
$date->setDate(2009, 06, 10)->setTime(12, 41)->setTime(12, 41, 10)->setISODate(2009, 05, 3)->modify('+1 day');
echo $date->format("Y-m-d H:i:s")."\n";

?>
===DONE===
--EXPECTF--
2009-01-29 12:41:10
===DONE===
