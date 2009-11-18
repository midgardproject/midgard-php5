<?php

/* Create non persistent configuration */
$config = new midgard_config();
$config->dbtype = "SQLite";


/* Initialize connection */
echo "Create new database connection...";
$mgd = midgard_connection::get_instance();
if (!$mgd->open_config ($config))
{
	throw new Exception ($mgd->get_error_string());
}
echo "Done \n";


/* Create default and minimal storage*/
echo "Create default storage...";
midgard_storage::create_base_storage();
echo "Done \n";

/* Create storage for every class which uses storage */
$re = new ReflectionExtension ("midgard2");
$classes = $re->getClasses();

foreach ($classes as $refclass)
{
	$mrcs[] = new midgard_reflection_class ($refclass->getName());
}

foreach ($mrcs as $refclass)
{
	$parent_class = $refclass->getParentClass();

	if (!$parent_class)
	{
		continue;
	}

	if ($parent_class->getName() == "midgard_dbobject"
		|| $parent_class->getName() == "midgard_object")
	{
		echo "Create storage for " . $refclass->getName() . " class...";
		midgard_storage::create_class_storage ($refclass->getName());
		echo "Done \n";
	}
}

?>
