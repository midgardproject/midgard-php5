--TEST--
test content manager's create_job routine
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
$manager = $mgd->get_content_manager();
echo "test connection\n";
$mgd_ref = $manager->get_connection();
var_dump(is_object($mgd_ref));
var_dump($mgd_ref instanceof MidgardConnection);
$content = new atype();
$reference = new MidgardObjectReference('123', 'id');
echo "test JobLoad\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_LOAD, $content, $reference);
var_dump($job instanceof MidgardSqlContentManagerJobLoad);
echo "test JobCreate\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_CREATE, $content, $reference);
var_dump($job instanceof MidgardSqlContentManagerJobCreate);
echo "test JobUpdate\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_UPDATE, $content, $reference);
var_dump($job instanceof MidgardSqlContentManagerJobUpdate);
echo "test JobDelete\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_DELETE, $content, $reference);
var_dump($job instanceof MidgardSqlContentManagerJobDelete);
echo "test JobPurge\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_PURGE, $content, $reference);
var_dump($job instanceof MidgardSqlContentManagerJobPurge);
echo "test InvalidJob\n";
try {
	$job = $manager->create_job(-1, $content, $reference);
} catch (Exception $e) {
	echo "ok\n";	
}

?>
===DONE===
--EXPECTF--
test connection
bool(true)
bool(true)
test JobLoad
bool(true)
test JobCreate
bool(true)
test JobUpdate
bool(true)
test JobDelete
bool(true)
test JobPurge
bool(true)
test InvalidJob
ok
===DONE===
