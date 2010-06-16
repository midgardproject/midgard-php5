--TEST--
test that midgard doesn't do any unexpected escaping
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--FILE--
<?php

$test_strings = array(
    "Hello, world\n",
    'Hello, world\n',
    "Hello, \"world\"",
    'Hello, \"world\"',
);

foreach ($test_strings as $str) {
    $obj = new atype();
    $obj->a = $str;
    $obj->t = $str;
    $obj->create();

    var_dump($obj->a === $str); // instant check
    var_dump($obj->t === $str); // instant check

    $obj2 = new atype($obj->id);

    var_dump($obj2->a === $str); // check after load from database
    var_dump($obj2->t === $str); // check after load from database

    // cleanup
    var_dump($obj->delete());
    unset($obj2, $obj);
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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
