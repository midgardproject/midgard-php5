<?php

class SqlQuerySelectDataConstraintGroupTest extends MidgardTest
{
    protected $select = null;
    protected $mgd = null;

    public function setUp()
    {
        $this->mgd = midgard_connection::get_instance();

        if ($this->select === null) {
            $this->select = new MidgardSqlQuerySelectData(midgard_connection::get_instance());
        }
    }

    public function tearDown()
    {
        unset($this->select);
    }

    public function testInheritance()
    {
        $cg = new MidgardQueryConstraintGroup("AND");
        $this->assertInstanceOf("MidgardQueryConstraintSimple", $cg);
    }

    public function testQueryConstraintGroup()
    {
        $storage = new MidgardQueryStorage("midgard_snippet");
        $columnA = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("name", $storage),
            "tbl1",
            "a"
        );
 
        $columnB = new MidgardSqlQueryColumn(
            new MidgardQueryProperty("code", $storage),
            "tbl1",
            "b"
        );

        $this->select->add_column($columnA);
        $this->select->add_column($columnB);

        $cg = new MidgardQueryConstraintGroup("AND");

        $cg->add_constraint(
            new MidgardSqlQueryConstraint(
                $columnA,
                "<>",
                new MidgardQueryValue("")
            )
        );

        $cg->add_constraint(
            new MidgardSqlQueryConstraint(
                $columnB,
                "<>",
                new MidgardQueryValue("")
            )
        );

        $this->select->set_constraint($cg);
        $this->select->execute();
    }
}

?>
