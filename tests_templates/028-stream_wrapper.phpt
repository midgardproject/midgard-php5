--TEST--
test form "midgard://" stream wrapper
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
$obj = new midgard_snippetdir();
$obj->name = 'directory';
$obj->create();

$obj2 = new midgard_snippet();
$obj2->snippetdir = $obj->id;
$obj2->name = 'snippet';
$obj2->code = '<?php echo "Hello, world!"; ?>';
$obj2->doc = 'This snippet says "Hello, world!"';
$obj2->create();

$fp = fopen('midgard:///directory/snippet', 'r');

var_dump(false !== $fp);
var_dump(fgets($fp) === $obj2->code);
var_dump(fclose($fp));

$obj2->delete();
$obj->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
===DONE===
