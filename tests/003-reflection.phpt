--TEST--
reflection tests
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = Off
midgard.memory_debug = Off
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
var_dump($cls->getDocComment());

$cls = new midgard_reflection_method($cls, 'getDocComment');
var_dump($cls->getDocComment());

$cls = new midgard_reflection_class('midgard_connection');
var_dump($cls->listSignals());

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
string(%d) "/**
     * Hi, method
     */"
string(%d) "returns doc_comment of method"
array(3) {
  [0]=>
  string(5) "error"
  [1]=>
  string(12) "auth-changed"
  [2]=>
  string(13) "lost-provider"
}
===DONE===
