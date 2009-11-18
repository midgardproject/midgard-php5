<?php
ini_set('display_errors', 'On');

/* Create default configuration for SQLite provider */
$cfg = new midgard_config();
$cfg->dbtype = "SQLite";
$cfg->save_file("midgard_example", FALSE);

/* Open connection */
$mgd = midgard_connection::get_instance();
$mgd->open_config($cfg);

/* Create blobdir */
/* midgard_config::create_blobdir(); */

/* Create midgard tables */
midgard_config::create_midgard_tables();

/* Create user defined tables */
foreach($_MIDGARD['schema']['types'] as $name => $v) 
{
    echo "Creating table for $name class \n";
    midgard_config::create_class_table($name);
    midgard_config::update_class_table($name);
}

?>
