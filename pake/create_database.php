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
$mgd->open_config($cfg);

echo "midgard_storage: create_base_storage\n";
midgard_storage::create_base_storage();

$re = new ReflectionExtension("midgard2");
foreach ($re->getClasses() as $class_ref) {
    $class_mgd_ref = new midgard_reflection_class($class_ref->getName());
    $parent_class = $class_mgd_ref->getParentClass();

    if (!$parent_class)
        continue;

    if (!in_array($parent_class->getName(), array("midgard_dbobject", "midgard_object", "midgard_view")))
        continue;

    $name = $class_mgd_ref->getName();
    echo 'midgard_storage: create_class_storage('.$name.")\n";
    midgard_storage::create_class_storage($name);
}

exit(0);