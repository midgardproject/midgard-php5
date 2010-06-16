--TEST--
test of query_builder class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file = [[CFG_FILE]]
report_memleaks = Off
--FILE--
<?php
$signals = array('update' => false,
                 'update-hook' => false,
                 'updated' => false,
                 'create' => false,
                 'create-hook' => false,
                 'created' => false);

class callback_holder
{
    public function test_callback($obj, $action)
    {
        global $signals;
        $signals[$action] = true;
    }

    public static function test_global_callback($a, $b, $c)
    {
        var_dump(is_object($a) and $a instanceof atype);
        var_dump($b);
        var_dump($c === 'test');
    }
}

$clbk = new callback_holder();

midgard_object_class::connect_default('atype', 'action-create', array('callback_holder', 'test_global_callback'), array('create', 'test'));
midgard_object_class::connect_default('atype', 'action-update', array('callback_holder', 'test_global_callback'), array('update', 'test'));

$t = new atype();
$t->connect('action-update',      array($clbk, 'test_callback'), array('update'));
$t->connect('action-update-hook', array($clbk, 'test_callback'), array('update-hook'));
$t->connect('action-updated',     array($clbk, 'test_callback'), array('updated'));
$t->connect('action-create',      array($clbk, 'test_callback'), array('create'));
$t->connect('action-create-hook', array($clbk, 'test_callback'), array('create-hook'));
$t->connect('action-created',     array($clbk, 'test_callback'), array('created'));

// This should turn all create hook flags to true
echo "== create() ==\n";
$t->create();

$qb = new midgard_query_builder('atype');
$res = $qb->execute();

foreach ($res as $r)
{
    // This should not turn all update flags to true, as signal is connected to specific object
    // But global-callback still should be called
    echo "== qb:update() ==\n";
    $r->update();
}

foreach ($signals as $s)
{
    var_dump($s);
}

// This should turn all update flags to true, as signal is connected to this object
// And global-callback still should be called
// That means, that 2 different callbacks will be fired here
echo "== update() ==\n";
$t->update();

echo "== results: ==\n";
foreach ($signals as $s)
{
    var_dump($s);
}

echo "== delete() ==\n";
var_dump($t->delete()); // don't leave traces!
?>
===DONE===
--EXPECTF--
== create() ==
bool(true)
string(6) "create"
bool(true)
== qb:update() ==
bool(true)
string(6) "update"
bool(true)
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
== update() ==
bool(true)
string(6) "update"
bool(true)
== results: ==
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
== delete() ==
bool(true)
===DONE===
