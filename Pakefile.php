<?php

pake_import('phpExtension');

ini_set('display_errors', 'On');

define('_SRC_DIR_', dirname(__FILE__));

// "public" tasks

pake_desc('Init tests');
pake_task('init_tests', 'clean_tests');

pake_desc('Init test-database');
pake_task('init_test_db', 'init_tests', 'clean_test_db', 'enable_midgard');

pake_desc('Run tests');
pake_task('test', 'init_tests', 'init_test_db');

pake_desc('Cleanup after tests');
pake_task('cleanup', 'clean_test_db', 'clean_tests');

// "private" tasks

pake_task('clean_tests');
pake_task('clean_test_db', 'enable_midgard');
pake_task('enable_midgard');

function run_test()
{
    $php_cgi = '';

    $_php_cgi = _get_php_executable().'-cgi';
    if (file_exists($_php_cgi)) {
        $php_cgi = ' '.escapeshellarg('TEST_PHP_CGI_EXECUTABLE='.$_php_cgi);
    }

    pake_echo_comment('Running test-suite. This can take awhile…');
    pake_sh('make test NO_INTERACTION=1 TEST_PHP_ARGS=-n'.$php_cgi);
    pake_echo_comment('Done');

    $path = _SRC_DIR_.'/tests';
    $files = pakeFinder::type('file')->ignore_version_control()->relative()->name('*.diff')->in($path);
    if (count($files) == 0) {
        pake_echo('   All tests PASSed!');
    } else {
        pake_echo_error('Following tests FAILed:');
        foreach ($files as $file) {
            $phpt_file = mb_ereg_replace('\.diff$', '.phpt', $file);
            $_lines = file($path.'/'.$phpt_file);
            $line = $_lines[1];
            unset($_lines);

            pake_echo('     '.$phpt_file.' ('.rtrim($line).')');
        }
    }
}

function run_init_test_db()
{
    $dir = _db_dir();
    pake_mkdirs($dir);

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
    foreach ($_files as $file) {
        $finder->name($file);
    }

    pake_remove($finder, $path);
}

function run_clean_test_db()
{
    $dir = _db_dir();
    $cfg = _get_midgard_config();

    pake_remove(pakeFinder::type('file')->name($cfg->database.'.db'), $dir);
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

function run_cleanup() {}

// Support tools

function _db_dir()
{
    if (!isset($_SERVER["HOME"]))
        throw new LogicException('"HOME" environment-variable is not set');

    if (!is_dir($_SERVER["HOME"]))
        throw new LogicException('"HOME" environment-variable does not point to the valid directory');

    return $_SERVER["HOME"].'/.midgard2/data';
}

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
