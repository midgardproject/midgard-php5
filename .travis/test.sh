#!/bin/bash
php `pyrus get php_dir|tail -1`/pake.php test

# PHPUnit tests
sudo pear channel-discover pear.phpunit.de
sudo pear install phpunit/PHPUnit
sh -c "cd tests/phpunit&&php -c midgard2.ini /usr/bin/phpunit -c phpunit_sqlite.xml.dist"
