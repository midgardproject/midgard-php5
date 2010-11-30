--TEST--
test for bug#1478
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
$login = 'bug1478';
$authtype = 'Plaintext';

$user = new midgard_user();
$user->login = $login;
$user->authtype = $authtype;
$user->create();

unset($user);

$params = array("login" => $login, "authtype" => $authtype);
$user2 = new midgard_user($params);

$mgd = midgard_connection::get_instance();

function my_callback($obj)
{
    echo "Callback called\n";
    if (null === $obj)
        echo "NULL\n";
    elseif ($obj instanceof midgard_connection)
        echo "midgard_connection\n";
    else
        echo "UNKNOWN!!\n";
}

$mgd->connect('auth-changed', 'my_callback', array());

var_dump($user2->login());
?>
===DONE===
--EXPECTF--
Callback called
midgard_connection
bool(true)
===DONE===
