<?php

class token_user extends midgard_user 
{
	private $_config = null;

	public function __construct($properties)
	{
		parent::__construct($properties);
	}

	/* Associate user with configuration file */
	public function set_config_file (midgard_key_config_file $config)
	{
		$this->_config = $config;
	}

	/* check if user is allowed to log in */
	public function login ($token = null)
	{
		$config = $this->_config;

		if ($token == null)
			return parent::login();

		$login_enabled = $config->get_value ($token, $this->login);

		if ($login_enabled == "false")
		{
			return false;
		}

		if ($this->is_admin()) 
		{
			$admin_enabled = $config->get_value ($token, "admin");

			if ($admin_enabled == "false")
			{
				return false;
			}
		}

		return parent::login();
	}
}

/* Open connection with given configuration */
$mgd = midgard_connection::get_instance();
$mcfg = new midgard_config();
$mcfg->read_file ("midgard_test", true);

if (!$mgd->open_config ($mcfg))
{
	echo "Failed to connect to midgard database \n";
	exit;
}

midgard_connection::get_instance()->set_loglevel ("debug");

/* Create user */
$user = new midgard_user();
$user->login = "John";
$user->authtype = "Plaintext";
$user->usertype = MGD_USER_TYPE_ADMIN;

if (!$user->create())
{
	echo "Failed to create user object. ";
	echo $mgd->get_error_string();
	exit;
}

/* Create context and its configuration file */
$ctx = new midgard_key_config_file_context ("/tmp/.midgard2/.my_app");
$config = new midgard_key_config_file ($ctx, "/auth/tokens");

/* Get user and try to log in */
$params = array ("login" => "John", "authtype" => "Plaintext");
$token_user = new token_user ($params);

if ($token_user) {

	$token_user->set_config_file ($config);
	$retval = $token_user->login ("Foo");

	if ($retval)
	{
		echo "Successfully logged in \n"; 
	}
}
else {

	echo "Failed to get user object. ";
	echo $mgd->get_error_string();
}

?>
