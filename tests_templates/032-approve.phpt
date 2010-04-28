--TEST--
test for midgard_object approve-methods
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

var_dump($obj1->is_approved()); // false
var_dump($obj1->approve());     // false (as we don't have rights)

$params = array("login" => 'admin', "password" => 'password', "authtype" => 'Plaintext');
$user = new midgard_user($params);
var_dump($user->login());       // true

var_dump($obj1->approve());     // true (admin-user)
var_dump($obj1->is_approved()); // true
var_dump($obj1->unapprove());   // true (still, admin-user)
var_dump($obj1->is_approved()); // false

$obj1->delete();

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
===DONE===
