--TEST--
test midgard_object_class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--FILE--
<?php
function __autoload($classname)
{
    if ($classname == 'myclass') {
        echo "Autoloading!\n";
        class myclass extends atype {}
    } elseif ($classname == 'myclass2') {
        echo "Autoloading!\n";
        class myclass2 extends xtype {}
    } else {
        echo "No class :(\n";
    }
}

$qb = new midgard_query_builder('myclass');
var_dump($qb instanceof midgard_query_builder);

$coll = new midgard_collector('myclass2', 'x', 'x');
var_dump($coll instanceof midgard_collector);

$qb = new midgard_query_builder('myclass3');
?>
===DONE===
--EXPECTF--
Autoloading!
bool(true)
Autoloading!
bool(true)
No class :(

Fatal error: Class 'myclass3' not found in %s on line %d
