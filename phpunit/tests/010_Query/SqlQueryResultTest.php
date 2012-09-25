<?php

class SqlQueryResultTest extends MidgardTest
{
    protected $select = null;

    public function setUp()
    {
        if ($this->select === null) {
            $this->select = new MidgardSqlQuerySelectData(midgard_connection::get_instance());
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

    public function testInheritance()
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $this->assertInstanceOf("MidgardSqlQueryResult", $result);
        $this->assertInstanceOf("MidgardQueryResult", $result);
    }

    public function testGetRows()
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertInstanceOf("MidgardQueryRow", $rows[0]);
        $this->assertInstanceOf("MidgardSqlQueryRow", $rows[0]);
        // Expect default admin person only 
        $this->assertCount(1, $rows);
    }

    public function testGetColumns() 
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertInstanceOf("MidgardQueryColumn", $columns[0]);
        $this->assertInstanceOf("MidgardSqlQueryColumn", $columns[0]);
        // Expect firstname AS fname, lastname AS lname
        $this->assertCount(2, $columns);
    }

    public function testGetColumnsQualifier()
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertInstanceOf("MidgardQueryColumn", $columns[0]);
        $this->assertInstanceOf("MidgardSqlQueryColumn", $columns[0]);
        foreach ($columns as $column) {
            $this->assertEquals($column->get_qualifier(), "p");
        }
    }

    public function testGetColumnPropertyName()
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertInstanceOf("MidgardQueryColumn", $columns[0]);
        $this->assertInstanceOf("MidgardSqlQueryColumn", $columns[0]);
        $names = array("firstname", "lastname");
        foreach ($columns as $column) {
            $qprop = $column->get_query_property();
            $this->assertInstanceOf("MidgardQueryProperty", $qprop);
            $this->assertContains($qprop->property, $names);
        }
    }

    public function testGetColumnNames()
    {
        $this->addColumns();
        $this->select->execute();
        $result = $this->select->get_query_result();
        $names = $result->get_column_names();
        // firstname AS fname, lastname AS lname
        $this->assertCount(2, $names);
        $columns = array("fname", "lname");
        $this->assertContains($names[0], $columns);
        $this->assertContains($names[1], $columns);
        $this->assertEquals($columns, $names);
    }
}

?>
