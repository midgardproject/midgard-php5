#!/bin/bash

# Install Pake
pyrus channel-discover pear.indeyets.ru
pyrus install -f http://pear.indeyets.ru/get/pake-1.6.3.tgz

# Options
MIDGARD_LIBS_VERSION=10.05.7
MIDGARD_EXT_VERSION=gjallarhorn

# Install Midgard2 library dependencies
sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu precise-security main restricted universe multiverse"
sudo apt-get --quiet update
sudo apt-get install -y dbus libglib2.0-dev libgda-4.0-4 libgda-4.0-dev libxml2-dev libdbus-1-dev libdbus-glib-1-dev valgrind php-pear

# Build Midgard2 core from recent tarball
wget -q https://github.com/midgardproject/midgard-core/tarball/${MIDGARD_EXT_VERSION}
tar -xzf ${MIDGARD_EXT_VERSION}
sh -c "cd midgardproject-midgard-core-*&&./autogen.sh --prefix=/usr; make; sudo make install"
rm -f ${MIDGARD_EXT_VERSION}

# Build and install the Midgard2 extension
php `pyrus get php_dir|tail -1`/pake.php install

# Enable dynamic loading of extensions in PHP
echo "enable_dl=On" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`

