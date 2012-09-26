--TEST--
test of midgard_user
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
$user1 = new midgard_user();
var_dump($user1);
?>
===DONE===
--EXPECTF--
<<<<<<< HEAD
object(midgard_user)#%d (9) {
  ["connection"]=>
  NULL
=======
object(MidgardUser)#%d (8) {
>>>>>>> gjallarhorn
  ["guid"]=>
  string(0) ""
  ["login"]=>
  string(0) ""
  ["password"]=>
  string(0) ""
  ["active"]=>
  bool(false)
  ["authtype"]=>
  string(0) ""
  ["authtypeid"]=>
  int(0)
  ["usertype"]=>
  int(0)
  ["person"]=>
  string(0) ""
}
===DONE===
