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
$manager = $mgd->get_content_manager();
$content = new atype();
$reference = new MidgardObjectReference('123', 'id');
echo "test JobLoad\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_LOAD, $content, $reference);
var_dump($job->get_connection() instanceof MidgardConnection);
var_dump($job->get_content_object() instanceof MidgardObject);
var_dump($job->get_reference() instanceof MidgardObjectReference);
var_dump($job->get_model());
echo "test JobCreate\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_CREATE, $content, $reference);
var_dump($job->get_connection() instanceof MidgardConnection);
var_dump($job->get_content_object() instanceof MidgardObject);
var_dump($job->get_reference() instanceof MidgardObjectReference);
var_dump($job->get_model());
echo "test JobUpdate\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_UPDATE, $content, $reference);
var_dump($job->get_connection() instanceof MidgardConnection);
var_dump($job->get_content_object() instanceof MidgardObject);
var_dump($job->get_reference() instanceof MidgardObjectReference);
var_dump($job->get_model());
echo "test JobDelete\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_DELETE, $content, $reference);
var_dump($job->get_connection() instanceof MidgardConnection);
var_dump($job->get_content_object() instanceof MidgardObject);
var_dump($job->get_reference() instanceof MidgardObjectReference);
var_dump($job->get_model());
echo "test JobPurge\n";
$job = $manager->create_job(MIDGARD_CONTENT_MANAGER_JOB_PURGE, $content, $reference);
var_dump($job->get_connection() instanceof MidgardConnection);
var_dump($job->get_content_object() instanceof MidgardObject);
var_dump($job->get_reference() instanceof MidgardObjectReference);
var_dump($job->get_model());
?>
===DONE===
--EXPECTF--
test JobLoad
bool(true)
bool(true)
bool(true)
NULL
test JobCreate
bool(true)
bool(true)
bool(true)
NULL
test JobUpdate
bool(true)
bool(true)
bool(true)
NULL
test JobDelete
bool(true)
bool(true)
bool(true)
NULL
test JobPurge
bool(true)
bool(true)
bool(true)
NULL
===DONE===
