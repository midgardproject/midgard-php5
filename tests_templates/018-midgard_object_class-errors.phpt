--TEST--
test midgard_object_class with wrong parameters
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = On
--FILE--
<?php
var_dump(midgard_object_class::factory('non_existant_class') === NULL);
var_dump(midgard_object_class::get_property_up(null) === NULL);
var_dump(midgard_object_class::get_property_up(123)  === NULL);
var_dump(midgard_object_class::get_property_parent(null) === NULL);
var_dump(midgard_object_class::get_property_parent(123)  === NULL);
var_dump(midgard_object_class::has_metadata(null) === NULL);
var_dump(midgard_object_class::has_metadata(123)  === NULL);
var_dump(midgard_object_class::get_schema_value(null, 'meta1') === NULL);
var_dump(midgard_object_class::get_schema_value(123, 'meta2')  === NULL);

?>
===DONE===
--EXPECTF--
Warning: midgard_object_class::factory(): Can not find non_existant_class class in %s on line %d
bool(true)

Warning: midgard_object_class::get_property_up(): argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::get_property_up(): argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::get_property_parent(): argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::get_property_parent(): argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::has_metadata(): argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::has_metadata(): argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::get_schema_value(): first argument should be object or string in %s on line %d
bool(true)

Warning: midgard_object_class::get_schema_value(): first argument should be object or string in %s on line %d
bool(true)
===DONE===
