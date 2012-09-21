--TEST--
test content manager update
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
$content->name = "Snippetdir 1 update";
$content2 = new midgard_snippetdir();
$content2->name = "Snippetdir 2";
$content2->create();
$content2->name = "Snippetdir 2 update";
$reference = new MidgardObjectReference($content->guid, 'guid');
$reference2 = new MidgardObjectReference($content2->guid, 'guid');
echo "test JobUpdate\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_UPDATE, $content, $reference);
$job2 = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_UPDATE, $content2, $reference2);
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
echo "execute\n";
$job->execute();
$job2->execute();
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
$content_copy = new midgard_snippetdir($content->guid);
var_dump($content_copy->name);
$content2_copy = new midgard_snippetdir($content2->guid);
var_dump($content2_copy->name);
$content->purge(false);
$content2->purge(false);
?>
===DONE===
--EXPECTF--
test JobUpdate
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
string(19) "Snippetdir 1 update"
string(19) "Snippetdir 2 update"
===DONE===
