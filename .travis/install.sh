#!/bin/bash

# Install Pake
pyrus channel-discover pear.indeyets.ru
pyrus install -f http://pear.indeyets.ru/get/pake-1.6.3.tgz

# Install Midgard from OBS
sudo apt-get install -y dbus libgda-4.0-4 libgda-4.0-dev
wget http://download.opensuse.org/repositories/home:/midgardproject:/ratatoskr/xUbuntu_10.04/i386/libmidgard2-2010_10.05.5.1-1_i386.deb 
wget http://download.opensuse.org/repositories/home:/midgardproject:/ratatoskr/xUbuntu_10.04/i386/midgard2-common_10.05.5.1-1_i386.deb
wget http://download.opensuse.org/repositories/home:/midgardproject:/ratatoskr/xUbuntu_10.04/i386/libmidgard2-dev_10.05.5.1-1_i386.deb 
sudo dpkg -i --force-depends libmidgard2-2010_10.05.5.1-1_i386.deb  
sudo dpkg -i midgard2-common_10.05.5.1-1_i386.deb
sudo dpkg -i libmidgard2-dev_10.05.5.1-1_i386.deb

# Build and install the Midgard2 extension
php `pyrus get php_dir|tail -1`/pake.php install

# Enable dynamic loading of extensions in PHP
echo "enable_dl=On" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`
