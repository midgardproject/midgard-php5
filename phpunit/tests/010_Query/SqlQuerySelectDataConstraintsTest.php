<?php

class SqlQuerySelectDataConstraintsTest extends MidgardTest
{
    protected $select = null;
    protected $default_snippet_storage = null;
    protected $default_sdir_storage = null;
    protected $mgd = null;

    const SNIPPET_QUALIFIER = "snippet_q";
    const SNIPPET_NAME = "sname";
    const SNIPPET_NAME_A = "Snippet A";
    const SNIPPET_NAME_B = "Snippet B";
    const SNIPPET_NAME_C = "Snippet C";
    const SNIPPET_NAME_COLUMN = "snamecolumn";
    const SNIPPET_ID = "snippet_id";
    const SNIPPET_SDIR_ID = "snippets_sdir_id";

    const SNIPPETDIR_QUALIFIER = "snippetdir_q";
    const SNIPPETDIR_NAME = "snippetdir_name";

    private function createSnippetDir()
    {
        $sdirA = new midgard_snippetdir();
        $sdirA->name = "Snippetdir";
        $this->assertTrue($sdirA->create());
        $this->assertEquals($this->mgd->get_error_string(), "MGD_ERR_OK");

        $sA = new midgard_snippet();
        $sA->name = self::SNIPPET_NAME_A;
        $sA->doc = "Doc A";
        $sA->snippetdir = $sdirA->id;
        $this->assertTrue($sA->create());
        $this->assertEquals($this->mgd->get_error_string(), "MGD_ERR_OK");

        $sB = new midgard_snippet();
        $sB->name = self::SNIPPET_NAME_B;
        $sB->doc = "Doc B";
        $sB->snippetdir = $sdirA->id;
        $this->assertTrue($sB->create());
        $this->assertEquals($this->mgd->get_error_string(), "MGD_ERR_OK");

        $sC = new midgard_snippet();
        $sC->name = self::SNIPPET_NAME_C;
        $sC->doc = "Doc C";
        $sC->snippetdir = $sdirA->id;
        $this->assertTrue($sC->create());
        $this->assertEquals($this->mgd->get_error_string(), "MGD_ERR_OK");
    }

    public function setUp()
    {
        $this->mgd = midgard_connection::get_instance();

        if ($this->select === null) {
            $this->select = new MidgardSqlQuerySelectData(midgard_connection::get_instance());
        }

        if ($this->default_snippet_storage === null) {
            $this->default_snippet_storage = new MidgardQueryStorage("midgard_snippet");
        }

        if ($this->default_sdir_storage === null) {
            $this->default_sdir_storage = new MidgardQueryStorage("midgard_snippetdir");
        }

        $this->createSnippetDir();
    }

    public function tearDown()
    {
        $q = new MidgardQuerySelect($this->default_snippet_storage);
        $q->execute();
        foreach ($q->list_objects() as $s) {
            $s->purge(false);
        }

        $q = new MidgardQuerySelect($this->default_sdir_storage);
        $q->execute();
        foreach ($q->list_objects() as $s) {
            $s->purge(false);
        }
        unset($this->select);
    }

    private function addColumns()
    {
        $storage = $this->default_snippet_storage;
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("name", $storage),
            self::SNIPPET_QUALIFIER,
            self::SNIPPET_NAME
        );
        $this->select->add_column($column);

        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("id", $storage),
            self::SNIPPET_QUALIFIER,
            self::SNIPPET_ID
        );
        $this->select->add_column($column);
    }

    private function addSdirColumns()
    {
        $this->addColumns();

        $storage = $this->default_sdir_storage;
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("name", $storage),
            self::SNIPPETDIR_QUALIFIER,
            self::SNIPPETDIR_NAME            
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

    public function testExecuteInvalidQuery()
    {
        $this->addColumns();
        $this->select->set_constraint(
            new MidgardSqlQueryConstraint(
                new MidgardSqlQueryColumn(
                    new MidgardQueryProperty("title"),
                    self::SNIPPET_QUALIFIER
                ),
                "=",
                new MidgardQueryValue("A")
            )
        );

        try {
            $this->select->execute();
        } catch (Exception $e) {

        }
    }

    public function testGetRowsLimit()
    {
        $this->addColumns();
        $this->select->set_limit(1);
        $this->select->execute();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        // There should be one snippet
        $this->assertCount(1, $rows);
    }

    public function testGetRowsOffset()
    {
        $this->addColumns();
        $this->select->set_limit(1);
        $this->select->set_offset(1);
        $this->select->execute();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        // There should be one snippet
        $this->assertCount(1, $rows);
    }

    public function testGetRowsWithSnippetA()
    {
        $this->addColumns();
        $this->select->set_constraint(
            new MidgardSqlQueryConstraint(
                new MidgardSqlQueryColumn(
                    new MidgardQueryProperty("name", $this->default_snippet_storage),
                    self::SNIPPET_QUALIFIER
                ),
                "=",
                new MidgardQueryValue(self::SNIPPET_NAME_A)
            )
        );

        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertCount(1, $rows);
    }

    public function testGetRowsWithAllSnippets()
    {
        $this->addColumns();
        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertCount(3, $rows);
    }

    public function testGetColumns()
    {
        $this->addColumns();
        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertCount(2, $columns);
    }

    public function testGetColumnNamesWithSnippets()
    {
        $this->addColumns();
        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertCount(2, $columns);
        $names = $result->get_column_names();
        $this->assertCount(2, $names);
        $colnames = array(self::SNIPPET_NAME, self::SNIPPET_ID);
        foreach ($names as $name) {
            $this->assertContains($names[0], $colnames);
            $this->assertContains($names[1], $colnames);
        }
    }

    public function testGetColumnNamesWithSnippetsAndSdir()
    {
        $this->addSdirColumns();
        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertCount(3, $columns);
        $names = $result->get_column_names();
        $this->assertCount(3, $names);
        $colnames = array(self::SNIPPET_NAME, self::SNIPPET_ID, self::SNIPPETDIR_NAME);
        foreach ($names as $name) {
            $this->assertContains($names[0], $colnames);
            $this->assertContains($names[1], $colnames);
            $this->assertContains($names[2], $colnames);
        }
    }

    public function testGetColumnQualifiers()
    {
        $this->addColumns();
        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertCount(2, $columns);
        foreach ($columns as $column) {
            $this->assertEquals($column->get_qualifier(), self::SNIPPET_QUALIFIER);
        }
    }

    public function testGetColumnPropertyName()
    {
        $this->addColumns();
        $this->select->execute(); 
        $result = $this->select->get_query_result();
        $columns = $result->get_columns();
        $this->assertCount(2, $columns);
        $names = array("name", "id");
        foreach ($columns as $column) {
            $this->assertContains($column->get_query_property()->property, $names);
        }
    }

}

?>
