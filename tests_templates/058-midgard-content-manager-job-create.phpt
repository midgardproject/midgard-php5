--TEST--
test content manager job's objects
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

function execute_callback($object, $args)
{
	global $msg;
	$msg = 'DONE';
	$args[0] = $msg;
}

$manager = $mgd->get_content_manager();
$content = new midgard_snippetdir();
$reference = new MidgardObjectReference('123', 'id');
echo "test JobCreate\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_CREATE, $content, $reference);
$job->connect('execution-end', 'execute_callback', array($msg));
echo "instance of MidgardValidable\n";
var_dump($job instanceof MidgardValidable);
echo "instance of MidgardExecutable\n";
var_dump($job instanceof MidgardExecutable);
echo "instance of MidgardJob\n";
var_dump($job instanceof MidgardJob);
echo "instance of MidgardContentManagerJob\n";
var_dump($job instanceof MidgardContentManagerJob);
echo "instance of MidgardSqlContentManagerJob\n";
var_dump($job instanceof MidgardSqlContentManagerJob);
echo "is valid\n";
var_dump($job->is_valid());
echo "is running\n";
var_dump($job->is_running());
echo "is executed\n";
var_dump($job->is_executed());
echo "is failed \n";
var_dump($job->is_failed());
echo "execute\n";
$job->execute();
var_dump($msg);
echo "is valid\n";
var_dump($job->is_valid());
echo "is executed\n";
var_dump($job->is_executed());
?>
===DONE===
--EXPECTF--
test JobCreate
instance of MidgardValidable
bool(true)
instance of MidgardExecutable
bool(true)
instance of MidgardJob
bool(true)
instance of MidgardContentManagerJob
bool(true)
instance of MidgardSqlContentManagerJob
bool(true)
is valid
bool(false)
is running
bool(false)
is executed
bool(false)
is failed 
bool(false)
execute
string(4) "DONE"
is valid
bool(true)
is executed
bool(true)
===DONE===
