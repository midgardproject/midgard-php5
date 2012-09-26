--TEST--
test if it's possible to set usertype only to valid values
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
$user = new midgard_user();
$user->login = 'hi';
$user->authtype = 'Plaintext';
$user->active = true;
var_dump($user->usertype = 0);
var_dump($user->usertype = 1);
var_dump($user->usertype = 2);
var_dump($user->usertype = 3);

?>
===DONE===
--EXPECTF--
int(0)
int(1)
int(2)

Warning: GLib: Invalid user type (3). in %s on line %d
int(3)
===DONE===
