--TEST--
test for bug#1460
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
$login = 'bug1460';
$authtype = 'Plaintext';

$user = new midgard_user();
$user->login = $login;
$user->authtype = $authtype;
$user->create();

var_dump($user->login === $login);
var_dump($user->authtype === $authtype);

$params = array("login" => $login, "authtype" => $authtype);
$user2 = new midgard_user($params);
$user2->login();

var_dump($user2->guid === $user->guid);

$mgd = midgard_connection::get_instance();
$u = $mgd->get_user();
$str1 = var_export($u, true);
var_dump($u instanceof midgard_user);

$u = $mgd->get_user();
$str2 = var_export($u, true);
var_dump($u instanceof midgard_user);

var_dump($str1 === $str2);
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
