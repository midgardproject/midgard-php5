--TEST--
test for proper memory management of $user->get_person/set_person
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
$person = new midgard_person();
$person->create();

$user = new midgard_user();

$user->login = 'hi';
$user->authtype = 'Plaintext';
$user->active = true;
$user->set_person($person);
unset($person);

var_dump($user->create());

$person = $user->get_person();
var_dump($person instanceof midgard_person);

$person2 = $user->get_person();
var_dump($person2 instanceof midgard_person);

$person3 = $user->get_person();
var_dump($person3 instanceof midgard_person);

unset($person2, $person3);

// cleanup
$user->delete();
$person->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
