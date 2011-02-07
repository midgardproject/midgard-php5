--TEST--
test if it's possible to get dbus messages
--SKIPIF--
<?php
if (!extension_loaded("midgard2"))
    die('skip'); 
if (!class_exists('midgard_dbus'))
    die("skip DBus support is not enabled");
if (!extension_loaded("pcntl"))
    die('skip PCNTL not installed');
if (version_compare(PHP_VERSION, '5.3.0') !== 1)
    die('skip php-5.3 is required');
?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = On
--FILE--
<?php
$pid = pcntl_fork();

if ($pid == -1) {
     die('could not fork');
} else if ($pid) {
     // we are the parent
     $dbus = new midgard_dbus('/midgard_foo', true);
     $dbus->connect('notified', function($dbus) {
         echo "Got notified via session bus\n";
         var_dump($dbus->get_message() == 'Hello!');
         exit(0);
     }, array());

     $loop = new midgard_g_main_loop();
     $loop->run();
} else {
    // we are the child
    sleep(1); // should be enough to init dbus-listener
    midgard_dbus::send('/midgard_foo', 'Hello!', true);
    exit(0);
}
?>
--EXPECTF--
Got notified via session bus
bool(true)
