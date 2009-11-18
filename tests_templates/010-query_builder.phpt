--TEST--
test of query_builder class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
$query = new midgard_query_builder('midgard_style');

var_dump($query->set_limit(-1));
var_dump($query->set_offset(-1));

var_dump($query->set_limit(1));
var_dump($query->set_offset(1));

var_dump($query->add_order('thereisnosuchfield', 'ASC'));
var_dump($query->add_order('name', 'asc'));
var_dump($query->add_order('name', 'ASC'));

var_dump($query->add_constraint('id', '>', new stdClass()));
var_dump($query->add_constraint('id', '>', STDOUT));
var_dump($query->add_constraint('id', '>', 2));
var_dump($query->add_constraint_with_property('id', '>', 'name'));

var_dump(gettype($query->count()));
var_dump(gettype($query->execute()));
?>
===DONE===
--EXPECTF--
Warning: Ignoring a negative query limit in %s on line %d
bool(false)

Warning: Ingoring a negative query offset in %s on line %d
bool(false)
bool(true)
bool(true)

Warning: %s in %s on line %d

Warning: %s in %s on line %d
bool(false)

Warning: %s in %s on line %d

Warning: %s in %s on line %d
bool(false)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
string(7) "integer"
string(5) "array"
===DONE===
