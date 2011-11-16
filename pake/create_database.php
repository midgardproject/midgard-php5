<?php

if (!isset($_SERVER['MIDGARD_ENV_GLOBAL_SHAREDIR']))
    throw new Exception('MIDGARD_ENV_GLOBAL_SHAREDIR environment variable is not set');

if (!isset($_SERVER['PAKE_MIDGARD_CFG']))
    throw new Exception('PAKE_MIDGARD_CFG environment variable is not set');

if (extension_loaded('midgard2')) {
    throw new LogicException('Please disable midgard2-extension in php.ini. test-suite will enable it automatically');
}

if (ini_get('enable_dl') != 1) {
    throw new LogicException('Please enable "enable_dl" setting in php.ini. it is required for proper work of tests');
}

ini_set('midgard.http', 'Off');
dl('midgard2.so');

$cfg = new midgard_config();
$cfg->read_file_at_path($_SERVER['PAKE_MIDGARD_CFG']);
$cfg->create_blobdir();

$mgd = midgard_connection::get_instance();
$res = $mgd->open_config($cfg);

if (false === $res) {
    throw new RuntimeException('connection failed: '.$mgd->get_error_string());
}

echo "midgard_storage: create_base_storage\n";
midgard_storage::create_base_storage();

$re = new ReflectionExtension("midgard2");
foreach ($re->getClasses() as $class_ref) {
    $class_mgd_ref = new midgard_reflection_class($class_ref->getName());
    $parent_class = $class_mgd_ref->getParentClass();

    $name = $class_mgd_ref->getName();
    if (!is_subclass_of ($name, 'MidgardDBObject')
        || $class_mgd_ref->isAbstract()) {
            continue;
    }

    echo 'midgard_storage: create_class_storage('.$name.")\n";
    if (true !== midgard_storage::create_class_storage($name)) {
        throw new Exception('Failed to create storage for "'.$name.': "'.midgard_connection::get_instance()->get_error_string());
    }
}

exit(0);
