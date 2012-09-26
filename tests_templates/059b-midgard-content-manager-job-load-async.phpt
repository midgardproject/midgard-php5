--TEST--
test content manager load
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

$mgd = MidgardConnection::get_instance();

$msg = 'TODO';
$msg2 = 'TODO 2';

function execute_callback($object, $args)
{
	global $msg;
	$msg = 'DONE';
	$args[0] = $msg;
}

function execute_callback2($object, $args)
{
	global $msg2;
	$msg2 = 'DONE 2';
	$args[0] = $msg2;
}

$manager = $mgd->get_content_manager();
$content = new midgard_snippetdir();
$content->name = "Snippetdir 1";
$content->create();
$content2 = new midgard_snippetdir();
$content2->name = "Snippetdir 2";
$content2->create();
$content_copy = new midgard_snippetdir();
$content2_copy = new midgard_snippetdir();
$reference = new MidgardObjectReference($content->guid, 'guid');
$reference2 = new MidgardObjectReference($content2->guid, 'guid');
echo "test JobLoad\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_LOAD, $content_copy, $reference);
$job2 = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_LOAD, $content2_copy, $reference2);
$job->connect('execution-end', 'execute_callback', array($msg));
$job2->connect('execution-end', 'execute_callback2', array($msg));
echo "instance of MidgardValidable\n";
var_dump($job instanceof MidgardValidable);
var_dump($job2 instanceof MidgardValidable);
echo "instance of MidgardExecutable\n";
var_dump($job instanceof MidgardExecutable);
var_dump($job2 instanceof MidgardExecutable);
echo "instance of MidgardJob\n";
var_dump($job instanceof MidgardJob);
var_dump($job2 instanceof MidgardJob);
echo "instance of MidgardContentManagerJob\n";
var_dump($job instanceof MidgardContentManagerJob);
var_dump($job2 instanceof MidgardContentManagerJob);
echo "instance of MidgardSqlContentManagerJob\n";
var_dump($job instanceof MidgardSqlContentManagerJob);
var_dump($job2 instanceof MidgardSqlContentManagerJob);
echo "is valid\n";
var_dump($job->is_valid());
var_dump($job2->is_valid());
echo "is running\n";
var_dump($job->is_running());
var_dump($job2->is_running());
echo "is executed\n";
var_dump($job->is_executed());
var_dump($job2->is_executed());
echo "is failed \n";
var_dump($job->is_failed());
var_dump($job2->is_failed());
echo "test invalid property\n";
try {
	$pool = new MidgardExecutionPool(array('max-n-invalid' => 2));
} catch (Exception $e) {
	// Do nothing
}
echo "execute\n";
$pool = new MidgardExecutionPool(array('max-n-threads' => 2));
$pool->push($job);
$pool->push($job2);
sleep(5);
unset($pool);
var_dump($msg);
var_dump($msg2);
echo "is valid\n";
var_dump($job->is_valid());
var_dump($job2->is_valid());
echo "is executed\n";
var_dump($job->is_executed());
var_dump($job2->is_executed());
echo "is failed\n";
var_dump($job->is_failed());
var_dump($job2->is_failed());
echo "is running\n";
var_dump($job->is_running());
var_dump($job2->is_running());
var_dump($content_copy->guid);
var_dump($content2_copy->guid);
var_dump(strlen($content_copy->guid) > 1);
var_dump(strlen($content2_copy->guid) > 1);
var_dump($content_copy->name);
var_dump($content2_copy->name);
$content->purge(false);
$content2->purge(false);
?>
===DONE===
--EXPECTF--
test JobLoad
instance of MidgardValidable
bool(true)
bool(true)
instance of MidgardExecutable
bool(true)
bool(true)
instance of MidgardJob
bool(true)
bool(true)
instance of MidgardContentManagerJob
bool(true)
bool(true)
instance of MidgardSqlContentManagerJob
bool(true)
bool(true)
is valid
bool(false)
bool(false)
is running
bool(false)
bool(false)
is executed
bool(false)
bool(false)
is failed 
bool(false)
bool(false)
test invalid property
execute
string(4) "DONE"
string(6) "DONE 2"
is valid
bool(true)
bool(true)
is executed
bool(true)
bool(true)
is failed
bool(false)
bool(false)
is running
bool(false)
bool(false)
string(%d) "%s"
string(%d) "%s"
bool(true)
bool(true)
string(12) "Snippetdir 1"
string(12) "Snippetdir 2"
===DONE===
