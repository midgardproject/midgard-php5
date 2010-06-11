--TEST--
tests for midgard_query_holder family of classes
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php

// Checking midgard_query_value's
$values = array(
    '',
    'test',
    'Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.',
    0,
    0.1,
    -20,
    -20.1,
    true,
    false,
);

foreach ($values as $i => $value) {
    $qvalue = new midgard_query_value($value);

    if ($qvalue->get_value() !== $value) {
        echo "values are note equal: ".var_export($value, true).' vs. '.var_export($qvalue->get_value(), true)."\n";
    }

    $qvalue = new midgard_query_value();
    $qvalue->set_value($value);

    if ($qvalue->get_value() !== $value) {
        echo "#2 values are note equal: ".var_export($value, true).' vs. '.var_export($qvalue->get_value(), true)."\n";
    }

    $qvalue = new midgard_query_value('dummy');
    $qvalue->set_value($value);

    if ($qvalue->get_value() !== $value) {
        echo "#3 values are note equal: ".var_export($value, true).' vs. '.var_export($qvalue->get_value(), true)."\n";
    }
}
echo "ok\n";

// Checking midgard_query_property's

$storage = new midgard_query_storage('atype');
$property = new midgard_query_property('a');

var_dump($property->property === 'a');
var_dump($property->storage === null);

$property = new midgard_query_property('a', $storage);
var_dump($property->property === 'a');
var_dump($property->storage instanceof midgard_query_storage);

unset($storage);
var_dump($property->storage instanceof midgard_query_storage);

?>
===DONE===
--EXPECTF--
ok
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
