--TEST--
test for midgard_object::emit()
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

function fun_callback()
{
    echo "callback\n";
}

$obj1 = new atype();
$obj1->a = '1';
$obj1->create();

$obj1->connect('action-update', 'fun_callback', array());

$obj1->emit('action-update');

$obj1->delete();

?>
===DONE===
--EXPECTF--
callback
===DONE===
