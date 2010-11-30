--TEST--
test of query_builder class
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file = [[CFG_FILE]]
report_memleaks = On
--FILE--
<?php
class extend_atype extends atype
{
    public function __construct($id = null)
    {
        parent::__construct($id);
        $this->connect('action-update', array($this, 'test_callback'), array('update'));
        $this->connect('action-update-hook', array($this, 'test_callback'), array('update-hook'));
        $this->connect('action-updated', array($this, 'test_callback'), array('updated'));
        $this->connect('action-create', array($this, 'test_callback'), array('create'));
        $this->connect('action-create-hook', array($this, 'test_callback'), array('create-hook'));
        $this->connect('action-created', array($this, 'test_callback'), array('created'));
    
    }
    
    public function test_callback($obj, $action)
    {
        global $signals;
        $signals[$action] = true;
    }
}


$signals = array (
    'update' => false,
    'update-hook' => false,
    'updated' => false,
    'create' => false,
    'create-hook' => false,
    'created' => false
);


// This should turn all create hook flags to true
$t = new extend_atype();
$t->create();

$qb = new midgard_query_builder('extend_atype');
$res = $qb->execute();
foreach ($res as $r)
{
    $r->update(); // This should turn all update flags to true
}

foreach ($signals as $s)
{
    var_dump($s);
}

var_dump($t->delete()); // don't leave traces!

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
===DONE===
