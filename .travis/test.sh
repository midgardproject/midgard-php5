#!/bin/bash

# PHPUnit tests
sudo /usr/bin/pear install -f pear
sudo /usr/bin/pear channel-discover pear.phpunit.de
sudo /usr/bin/pear channel-discover pear.symfony-project.com
sudo /usr/bin/pear install --alldeps phpunit/PHPUnit
sh -c "cd phpunit&&php -c midgard2.ini /usr/bin/phpunit -c phpunit_sqlite.xml.dist"

php -c midgard2.ini `pyrus get php_dir|tail -1`/pake.php test
