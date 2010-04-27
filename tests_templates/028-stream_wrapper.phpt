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
$obj2->code = '<?php echo "Hello, world!\n"; ?>';
$obj2->doc = 'This snippet says "Hello, world!"';
$obj2->create();

$fp = fopen('midgard:///directory/snippet', 'r');

var_dump(false !== $fp);
var_dump(fgets($fp) === $obj2->code);
var_dump(fclose($fp));

include 'midgard:///directory/snippet';

$fp = fopen('midgard:///directory/snippet', 'w');
var_dump(false !== $fp);

var_dump(false !== fwrite($fp, "Hello, test!\n"));
var_dump(fclose($fp));

include 'midgard:///directory/snippet';

$fp = fopen('midgard:///directory/snippet', 'a');
var_dump(false !== $fp);

var_dump(false !== fwrite($fp, "And Again!\n"));
var_dump(fclose($fp));

include 'midgard:///directory/snippet';

$obj2->delete();
$obj->delete();

?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
Hello, world!
bool(true)
bool(true)
bool(true)
Hello, test!
bool(true)
bool(true)
bool(true)
Hello, test!
And Again!
===DONE===
