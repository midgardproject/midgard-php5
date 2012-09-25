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
    const SNIPPETDIR_NAME = "Snippetdir";
    const SNIPPETDIR_NAME_COLUMN = "snippetdir_name";
    const SNIPPETDIR_ID = "snippetdir_id";

    private function createSnippetDir()
    {
        $sdirA = new midgard_snippetdir();
        $sdirA->name = self::SNIPPETDIR_NAME;
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
            self::SNIPPETDIR_NAME_COLUMN            
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
        $colnames = array(self::SNIPPET_NAME, self::SNIPPET_ID, self::SNIPPETDIR_NAME_COLUMN);
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

    private function prepareJoin()
    {
        $this->addSdirColumns();

        // snippetdir_q.id AS snippetdir_id
        $sdir_storage = $this->default_sdir_storage;
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("id", $sdir_storage),
            self::SNIPPETDIR_QUALIFIER,
            self::SNIPPETDIR_ID            
        );
        $this->select->add_column($column);

        // snippet_q.snippetdir AS snippets_sdir_id 
        $storage = $this->default_snippet_storage;
        $column = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("snippetdir", $storage),
            self::SNIPPET_QUALIFIER,
            self::SNIPPET_SDIR_ID
        );
        $this->select->add_column($column);

        // JOIN ON (snippetdir_q.id = snippet_q.snippetdir)
        $this->select->add_join(
            "INNER", 
            new MidgardSqlQueryColumn(
                new MidgardQueryProperty("snippetdir", $storage),
                self::SNIPPET_QUALIFIER,
                self::SNIPPET_SDIR_ID
            ),
            new MidgardSqlQueryColumn(
                new MidgardQueryProperty("id", $sdir_storage),
                self::SNIPPETDIR_QUALIFIER,
                self::SNIPPETDIR_ID
            )
        );
    }

    private function addSnippetNameConstraint()
    {
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
    }

    public function testAddJoin()
    {
        $this->prepareJoin();
        $this->addSnippetNameConstraint();
        $this->select->execute();
        //echo $this->select->get_query_string();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertCount(1, $rows);

        $values = $rows[0]->get_values();
        $this->assertCount(5, $values);
        $this->assertEquals($values[0], self::SNIPPET_NAME_A);
        // DO not test ids, those depends on provider
        $this->assertEquals($values[2], self::SNIPPETDIR_NAME);
        // snippet.snippetdir = snippetdir.id
        $this->assertEquals($values[3], $values[4]);
    }

    public function testAddJoinAddOrderASC()
    {   
        /*
        SELECT 
            snippet_q.name AS sname, 
            snippet_q.id AS snippet_id, 
            snippetdir_q.name AS snippetdir_name, 
            snippetdir_q.id AS snippetdir_id, 
            snippet_q.snippetdir AS snippets_sdir_id
        FROM snippet AS snippet_q
            INNER JOIN snippetdir AS snippetdir_q ON (snippet_q.snippetdir = snippetdir_q.id)
        WHERE 1=1 AND 0<1
        ORDER BY snippet_q.name ASC
         */
        $this->prepareJoin();
        $this->select->add_order(
            new MidgardSqlQueryColumn(
                new MidgardQueryProperty("name", $this->default_snippet_storage),
                self::SNIPPET_QUALIFIER
            ),
            SORT_ASC
        );
        $this->select->execute();
        //echo $this->select->get_query_string();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertCount(3, $rows);
        $this->assertEquals($rows[0]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_A);
        $this->assertEquals($rows[0]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);
        $this->assertEquals($rows[1]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_B);
        $this->assertEquals($rows[1]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);
        $this->assertEquals($rows[2]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_C);
        $this->assertEquals($rows[2]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);

        try {
            $rows[0]->get_value("Doesntexist");
        } catch (Exception $e) {
            //expected
        }
    }

    public function testAddJoinAddOrderDESC()
    {   
        /*
        SELECT 
            snippet_q.name AS sname, 
            snippet_q.id AS snippet_id, 
            snippetdir_q.name AS snippetdir_name, 
            snippetdir_q.id AS snippetdir_id, 
            snippet_q.snippetdir AS snippets_sdir_id
        FROM snippet AS snippet_q
            INNER JOIN snippetdir AS snippetdir_q ON (snippet_q.snippetdir = snippetdir_q.id)
        WHERE 1=1 AND 0<1
        ORDER BY snippet_q.name DESC
         */
        $this->prepareJoin();
        $this->select->add_order(
            new MidgardSqlQueryColumn(
                new MidgardQueryProperty("name", $this->default_snippet_storage),
                self::SNIPPET_QUALIFIER
            ),
            SORT_DESC
        );
        $this->select->execute();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertCount(3, $rows);
        $this->assertEquals($rows[0]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_C);
        $this->assertEquals($rows[0]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);
        $this->assertEquals($rows[1]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_B);
        $this->assertEquals($rows[1]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);
        $this->assertEquals($rows[2]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_A);
        $this->assertEquals($rows[2]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);

        try {
            $rows[0]->get_value("Doesntexist");
        } catch (Exception $e) {
            //expected
        }
    }

    public function testAddJoinAddOrderDESCASC()
    {   
        /*
        SELECT 
            snippet_q.name AS sname, 
            snippet_q.id AS snippet_id, 
            snippetdir_q.name AS snippetdir_name, 
            snippetdir_q.id AS snippetdir_id, 
            snippet_q.snippetdir AS snippets_sdir_id
        FROM snippet AS snippet_q
            INNER JOIN snippetdir AS snippetdir_q ON (snippet_q.snippetdir = snippetdir_q.id)
        WHERE 1=1 AND 0<1
        ORDER BY snippet_q.name DESC, snippet_q.id ASC
         */
        $this->prepareJoin();
        $this->select->add_order(
            new MidgardSqlQueryColumn(
                new MidgardQueryProperty("name", $this->default_snippet_storage),
                self::SNIPPET_QUALIFIER
            ),
            SORT_DESC
        );
        $this->select->add_order(
            new MidgardSqlQueryColumn(
                new MidgardQueryProperty("id", $this->default_snippet_storage),
                self::SNIPPET_QUALIFIER
            ),
            SORT_ASC
        );
        $this->select->execute();
        //echo $this->select->get_query_string();
        $result = $this->select->get_query_result();
        $rows = $result->get_rows();
        $this->assertCount(3, $rows);
        $this->assertEquals($rows[0]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_C);
        $this->assertEquals($rows[0]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);
        $this->assertEquals($rows[1]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_B);
        $this->assertEquals($rows[1]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);
        $this->assertEquals($rows[2]->get_value(self::SNIPPET_NAME), self::SNIPPET_NAME_A);
        $this->assertEquals($rows[2]->get_value(self::SNIPPETDIR_NAME_COLUMN), self::SNIPPETDIR_NAME);

        try {
            $rows[0]->get_value("Doesntexist");
        } catch (Exception $e) {
            //expected
        }
    }
}

?>
