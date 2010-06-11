--TEST--
test for midgard_object lock-methods
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

$obj1 = new atype();
$obj1->a = 'a';
$obj1->create();

var_dump($obj1->is_locked()); // false
var_dump($obj1->lock());     // false (as we don't have rights)

$params = array("login" => 'admin', "password" => 'password', "authtype" => 'Plaintext');
$user = new midgard_user($params);
var_dump($user->login());       // true

var_dump($obj1->lock());     // true (admin-user)
var_dump($obj1->is_locked()); // true
var_dump($obj1->unlock());   // true (still, admin-user)
var_dump($obj1->is_locked()); // false

var_dump($obj1->delete());

?>
===DONE===
--EXPECTF--
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
===DONE===
