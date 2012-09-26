--TEST--
test model and model reference 
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
display_startup_errors = On
report_memleaks = On
--FILE--
<?php
$id = 123;
$name = "id";
$workspace = new MidgardWorkspace();
$reference = new MidgardObjectReference($id, $name, $workspace);
echo "is object\n";
var_dump(is_object($reference));
echo "is model\n";
var_dump($reference instanceof MidgardModel);
echo "is model reference\n";
var_dump($reference instanceof MidgardModelReference);
echo "get name\n";
var_dump($reference->get_name());
echo "get id\n";
var_dump($reference->get_id());
echo "get id value\n";
var_dump($reference->get_id_value()); 
echo "get workspace\n";
var_dump (get_class($reference->get_workspace()));
?>
===DONE===
--EXPECTF--
is object
bool(true)
is model
bool(true)
is model reference
bool(true)
get name
string(2) "id"
get id
string(3) "123"
get id value
int(123)
get workspace
string(16) "MidgardWorkspace"
===DONE===
