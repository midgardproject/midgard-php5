<?php

/* Create new file based config context */
try 
{
	$ctx = new midgard_key_config_file_context ("/tmp/.midgard2/.my_app");
}
catch (Exception $e) {

	echo $e->getMessage() . "\n";
	exit;
}

/* Create new config file to store authentication tokens */
try 
{
	$config = new midgard_key_config_file ($ctx, "/auth/tokens");
}
catch (Exception $e) {

	echo $e->getMessage() . "\n";
	exit;
}

/* Set keys with values */
$config->set_value ("Foo", "admin", "false");
$config->set_value ("Bar", "John", "false");
$config->set_value ("Bar", "Alice", "false");

/* Store config */
if (!$config->store()) 
{
	echo "Failed to store authentication config \n";
}
else {

	echo "Authentication config successfully stored \n";
}

/* Create new config file to store user's preferences */
$user_config = new midgard_key_config_file ($ctx, "/config/ui");

/* Set keys */
$user_config->set_value ("Style", "color", "yellow");
$user_config->set_value ("Style", "theme", "default");

/* Store config */
if (!$user_config->store()) 
{
	echo "Failed to store preferences config \n";
}
else {

	echo "Preferences config successfully stored \n";
}

/* List all available configurations */
$cfgs = $ctx->list_key_config();

if (empty ($cfgs))
{
	echo "No single configuration found for this context \n";
}
else {

	echo "Configurations found: \n";
	foreach ($cfgs as $configpath => $v) 
	{
		echo $configpath . "\n";
		$ckf = new midgard_key_config_file ($ctx, $configpath);
		echo $ckf->to_data(); echo "\n";
	}	
}

?>
