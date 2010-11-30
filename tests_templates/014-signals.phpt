--TEST--
checking which callbacks are ok and which are not
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file = [[CFG_FILE]]
report_memleaks = On
--FILE--
<?php

function fun_callback()
{
}

class dumb
{
    public function my_callback()
    {
        # code...
    }
    public static function test_callback()
    {
    }
}

$dumb_obj = new dumb();

$obj = new atype();
echo "1\n";
$obj->connect('action-update', array(null, 'fun_callback'), array());
echo "2\n";
$obj->connect('action-update', array('dumb', null), array());
echo "3\n";
$obj->connect('action-update', array('fun_callback'), array());
echo "4\n";
$obj->connect('action-update', 'fun_callback', array());
echo "5\n";
$obj->connect('action-update', array('dumb', 'test_callback'), array('update'));
echo "6\n";
$obj->connect('action-update', array($dumb_obj, 'my_callback'));

echo "7\n";
midgard_object_class::connect_default('atype', 'action-update', array(null, 'test_callback'), array('test'));
echo "8\n";
midgard_object_class::connect_default('atype', 'action-update', array('dumb', null));
echo "9\n";
midgard_object_class::connect_default('atype', 'action-update', array('fun_callback'), array('test'));
echo "10\n";
midgard_object_class::connect_default('atype', 'action-update', 'fun_callback', array('test'));
echo "11\n";
midgard_object_class::connect_default('atype', 'action-update', array('dumb', 'test_callback'), array('test'));
echo "12\n";
midgard_object_class::connect_default('atype', 'action-update', array($dumb_obj, 'my_callback'), array('test'));

?>
===DONE===
--EXPECTF--
1

Warning: atype::connect() %s in %s on line %d
2

Warning: atype::connect() %s in %s on line %d
3

Warning: atype::connect() %s in %s on line %d
4
5
6
7

Warning: midgard_object_class::connect_default() %s in %s on line %d
8

Warning: midgard_object_class::connect_default() %s in %s on line %d
9

Warning: midgard_object_class::connect_default() %s in %s on line %d
10
11
12
===DONE===
