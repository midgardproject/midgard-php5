<?php

if (version_compare(pakeApp::VERSION, '1.4.0', '<='))
    throw new pakeException('Pake 1.4.1 or newer is required');

ini_set('display_errors', 'On');
define('_SRC_DIR_', dirname(__FILE__));

// "public" tasks

pake_desc('Init tests');
pake_task('init_tests', 'clean_tests');

pake_desc('Init test-database');
pake_task('init_test_db', 'init_tests', 'clean_test_db', 'enable_midgard');

// "private" tasks

pake_task('clean_tests');
pake_task('clean_test_db', 'enable_midgard');
pake_task('enable_midgard');

// Adding standard extension-building tasks
pake_import('phpExtension', false);
// -> injecting dependencies
pakePhpExtensionTask::$tasks['test'][1][] = 'init_tests';
pakePhpExtensionTask::$tasks['test'][1][] = 'init_test_db';
pakePhpExtensionTask::$tasks['clean'][1][] = 'clean_tests';
pakePhpExtensionTask::$tasks['clean'][1][] = 'clean_test_db';
// -> registering tasks
pakePhpExtensionTask::import_default_tasks();
// DONE


function run_init_test_db()
{
    $cfg = _get_midgard_config();

    pake_echo_comment('Creating empty database for tests… (be patient, it takes time)');

    putenv('MIDGARD_ENV_GLOBAL_SHAREDIR='._SRC_DIR_.'/tests/share'.'');
    putenv('PAKE_MIDGARD_CFG='._SRC_DIR_.'/tests/test.cfg');

    pake_sh(_get_php_executable().' '._SRC_DIR_.'/pake/create_database.php');
}

function run_init_tests()
{
    pake_echo_comment('Creating tests from templates');
    $path = _SRC_DIR_.'/tests';
    $src_path = _SRC_DIR_.'/tests_templates';

    $tokens = array(
        'PATH'       => $path,
        'SHARE_PATH' => $path.'/share',
        'BLOB_PATH'  => $path.'/blobs',
        'CFG_FILE'   => $path.'/test.cfg'
    );

    $files = pakeFinder::type('file')->ignore_version_control()->relative()->in($src_path);
    pake_replace_tokens_to_dir($files, $src_path, $path, '[[', ']]', $tokens);
}

function run_clean_tests()
{
    $path = _SRC_DIR_.'/tests';
    $src_path = _SRC_DIR_.'/tests_templates';

    // clean runtime-generated files
    $finder = pakeFinder::type('file')->ignore_version_control()->name('*.php', '*.exp', '*.out', '*.log', '*.diff');
    pake_remove($finder, $path);

    // clean "generated" tests
    $_files = pakeFinder::type('file')->ignore_version_control()->relative()->in($src_path);

    $finder = pakeFinder::type('file')->ignore_version_control();
    call_user_func_array(array($finder, 'name'), $_files);

    pake_remove($finder, $path);
}

function run_clean_test_db()
{
    $file = _SRC_DIR_.'/tests/'._get_midgard_config()->database.'.db';

    if (file_exists($file))
        pake_remove($file, '');
}

function run_enable_midgard()
{
    pake_echo_comment('Enabling midgard2 extension');
    if (extension_loaded('midgard2')) {
        throw new LogicException('Please disable midgard2-extension in php.ini. test-suite will enable it automatically');
    }

    if (ini_get('enable_dl') != 1) {
        throw new LogicException('Please enable "enable_dl" setting in php.ini. it is required for proper work of tests');
    }

    ini_set('midgard.http', 'Off');
    dl('midgard2.so');
}

// Support tools

function _get_midgard_config()
{
    static $cfg = null;

    if (null === $cfg) {
        $cfg = new midgard_config();
        $cfg->read_file_at_path(_SRC_DIR_.'/tests/test.cfg');
    }

    return $cfg;
}

function _get_php_executable()
{
    static $php_exec = null;

    if (null === $php_exec) {
        if (!file_exists(_SRC_DIR_.'/Makefile')) {
            throw new LogicException("Makefile is missing. You have to build extension before testing it!");
        }

        $makefile = file(_SRC_DIR_.'/Makefile');
        foreach ($makefile as $row) {
            if (strpos($row, 'PHP_EXECUTABLE = ') !== 0)
                continue;

            $row = rtrim($row);
            $parts = explode(' = ', $row);
            if (!isset($parts[1]) or strlen($parts[1]) == 0)
                continue;

            $php_exec = $parts[1];
            break;
        }
        unset($makefile);
    }
    
    if (!$php_exec)
    {
        // Fallback
        $php_exec = 'php';
    }
    
    return $php_exec;
}
