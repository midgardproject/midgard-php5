--TEST--
tests for midgard_query_constraint classes
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
$storage = new midgard_query_storage('atype');
$property = new midgard_query_property('a', $storage);
$value = new midgard_query_value('abc');

$constraint = new midgard_query_constraint($property, '=', $value, $storage);

var_dump($constraint->property instanceof midgard_query_property);
var_dump($constraint->operator === '=');
var_dump($constraint->holder instanceof midgard_query_value);
var_dump($constraint->storage instanceof midgard_query_storage);

unset($storage, $property, $value);

var_dump($constraint->property instanceof midgard_query_property);
var_dump($constraint->operator === '=');
var_dump($constraint->holder instanceof midgard_query_value);
var_dump($constraint->storage instanceof midgard_query_storage);


echo "==group==\n";

$group = new midgard_query_constraint_group();
$list = $group->list_constraints();

var_dump(is_array($list));
var_dump(count($list) == 0);

$group->add_constraint($constraint);
$group->add_constraint($constraint);
$list = $group->list_constraints();

var_dump(is_array($list));
var_dump(count($list) == 2);

unset($constraint);

$list = $group->list_constraints();

var_dump(is_array($list));
var_dump(count($list) == 2);

foreach ($list as $constraint) {
    if (!($constraint instanceof midgard_query_constraint)) {
        echo 'wrong type: '.var_export($constraint, true)."\n";
    }
}

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
==group==
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
