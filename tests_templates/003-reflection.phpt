--TEST--
reflection tests
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
--ENV--
MIDGARD_ENV_GLOBAL_SHAREDIR=[[SHARE_PATH]]
--FILE--
<?php
$cls = new midgard_reflection_class('midgard_reflection_class');
var_dump($cls->getDocComment());

$cls = new midgard_reflection_class('midgard_reflection_method');
var_dump($cls->getDocComment());

/**
 * Hi, class
 */
class MyTest
{
    /**
     * Hi, method
     */
    public static function myMethod()
    {
    }
}

$cls = new midgard_reflection_class('MyTest');
var_dump($cls->getDocComment());
var_dump($cls->listSignals());

$cls = new midgard_reflection_method('MyTest', 'myMethod');
//var_dump($cls->getDocComment());

$cls = new midgard_reflection_method($cls, 'getDocComment');
var_dump($cls->getDocComment());

$cls = new midgard_reflection_class('midgard_connection');
var_dump($cls->listSignals());

$cls = new midgard_reflection_class('midgard_object');
var_dump(count($cls->listSignals()) > 0);

$cls = new midgard_reflection_class('atype');
var_dump(count($cls->listSignals()) == 0);

$parent = $cls->getParentClass();
var_dump($parent instanceof midgard_reflection_class);

$methods = $cls->getMethods();
var_dump($methods[0] instanceof midgard_reflection_method);

var_dump($cls->get_user_value('meta1') == 'value1');
var_dump($cls->get_user_value('meta2') == 'value2');

?>
===DONE===
--EXPECTF--
string(%d) "Helps Midgard to show doc_comments of internal classes"
string(%d) "Helps Midgard to show doc_comments of methods of internal classes"
string(%d) "/**
 * Hi, class
 */"
array(0) {
}
string(%d) "returns doc_comment of method"
array(5) {
  [0]=>
  string(5) "error"
  [1]=>
  string(12) "auth-changed"
  [2]=>
  string(13) "lost-provider"
  [3]=>
  string(9) "connected"
  [4]=>
  string(12) "disconnected"
}
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
