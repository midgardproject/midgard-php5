--TEST--
test for bug#1763
--SKIPIF--
<?php if (!extension_loaded("midgard2")) print "skip"; ?>
--INI--
midgard.engine = On
midgard.http = On
midgard.memory_debug = Off
midgard.configuration=
midgard.configuration_file=[[CFG_FILE]]
report_memleaks = Off
--FILE--
<?php
class company_person extends midgard_person
{
    public function __construct($id=null)
    {
        echo(__METHOD__."\n");
        parent::__construct($id);
        $this->email = 'mailaddress@company.com';
    }

    public function create()
    {
        echo(__METHOD__."\n");
        parent::create();
    }
}

class inherited_person extends company_person
{
}

$person = new inherited_person();
$person->firstname = 'Test';
$person->lastname = 'User';
$person->create();

var_dump(!empty($person->id));

// cleanup
$person->delete();

?>
===DONE===
--EXPECTF--
company_person::__construct
company_person::create
bool(true)
===DONE===
