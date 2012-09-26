--TEST--
midgard_datetime tests
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
report_memleaks = On
--FILE--
<?php
$date = new midgard_datetime();
$date->setDate(2009, 06, 10);
echo $date->format("Y-m-d")."\n";
$date->setTime(12, 41);
echo $date->format("Y-m-d H:i:s")."\n";
$date->setTime(12, 41, 10);
echo $date->format("Y-m-d H:i:s")."\n";
$date->setISODate(2009, 05, 3);
echo $date->format("Y-m-d H:i:s")."\n";
$date->modify('+1 day');
echo $date->format("Y-m-d H:i:s")."\n";

echo ((string)$date)."\n";
?>
===DONE===
--EXPECTF--
2009-06-10
2009-06-10 12:41:00
2009-06-10 12:41:10
2009-01-28 12:41:10
2009-01-29 12:41:10
2009-01-29T12:41:10+00:00
===DONE===
