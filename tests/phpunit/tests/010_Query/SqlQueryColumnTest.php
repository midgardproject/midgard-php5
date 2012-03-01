<?php

class SqlQueryColumnTest extends MidgardTest
{
    protected $column = null;
    protected $queryProperty = null;

    public function setUp()
    {
        if ($this->column === null) {
            $this->queryProperty = new MidgardQueryProperty("title");
            $this->column = new MidgardSqlQueryColumn($this->queryProperty, "t1", "The name");
        }
    }

    public function testInheritance()
    {
        $this->assertInstanceOf("MidgardSqlQueryColumn", $this->column);
        $this->assertInstanceOf("MidgardQueryColumn", $this->column);
    }

    public function testGetQueryProperty()
    {
        $this->assertEquals($this->column->get_query_property(), $this->queryProperty);
        $this->assertInstanceOf("MidgardQueryProperty", $this->column->get_query_property());
    }

    public function testGetName()
    {
        /* writable only property */
        $this->assertEquals($this->column->name, null);
        $this->assertEquals($this->column->get_name(), "The name");
    }

    public function testGetQualifier()
    {
        /* writable only property */
        $this->assertEquals($this->column->qualifier, null);
        $this->assertEquals($this->column->get_qualifier(), "t1");
    }
}

?>
