--TEST--
workspaces tests
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
var_dump(class_exists('midgard_workspace_storage'));
var_dump(class_exists('midgard_workspace'));
var_dump(class_exists('midgard_workspace_context'));
var_dump(class_exists('midgard_workspace_manager'));
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===