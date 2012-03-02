<?php

class SqlQuerySelectDataTest extends MidgardTest
{
    protected $select = null;

    public function setUp()
    {
        if ($this->select === null) {
            $this->select = new MidgardSqlQuerySelectData(midgard_connection::get_instance());
        }
    }

    public function testInheritance()
    {
        $this->assertInstanceOf("MidgardSqlQuerySelectData", $this->select);
        $this->assertInstanceOf("MidgardQueryExecutor", $this->select);
    }

    public function testAddColumn()
    {
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("title"), 
            "t1", 
            "title_a"
        );
        $this->select->add_column($column);
    }

    public function testGetColumns()
    {
        // t1.property_a AS name_a
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("property_a"), 
            "t1", 
            "name_a"
        );
        $this->select->add_column($column);

        // t2.property_b AS name_b
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("property_b"), 
            "t2", 
            "name_b"
        );
        $this->select->add_column($column);

        // t3.property_c AS name_c
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("property_c"), 
            "t3", 
            "name_c"
        );
        $this->select->add_column($column);
    
        $columns = $this->select->get_columns();
        $this->assertInternalType('array', $columns);
        $this->assertCount(3, $columns);

        $names = array("name_a", "name_b","name_c");
        $qualifiers = array("t1", "t2", "t3");

        foreach ($columns as $column) {
            $this->assertNotNull($column->get_name());
            $this->assertContains($column->get_name(), $names);
            $this->assertNotNull($column->get_qualifier());
            $this->assertContains($column->get_qualifier(), $qualifiers); 
        }
    }

    private function addColumns()
    {
        $storage = new MidgardQueryStorage("midgard_person");
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("firstname", $storage), 
            "p", 
            "fname"
        );
        $this->select->add_column($column);

        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("lastname", $storage), 
            "p", 
            "lname"
        );
        $this->select->add_column($column);
    }

    public function testExecute()
    {
        $this->addColumns();
        $this->select->execute();
    }

    public function testGetQueryResult()
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $this->assertInstanceOf("MidgardQueryResult", $result);
        $this->assertInstanceOf("MidgardSqlQueryResult", $result);
    }
}

?>
