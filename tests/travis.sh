#!/bin/bash

# Install Pake
pyrus channel-discover pear.indeyets.ru
pyrus install -f http://pear.indeyets.ru/get/pake-1.6.3.tgz
echo "Pake is in `pyrus get bin_dir|tail -1`"

# Install Midgard from OBS
sudo apt-get install -y dbus libgda-4.0-4 libgda-4.0-dev
wget http://download.opensuse.org/repositories/home:/midgardproject:/ratatoskr/xUbuntu_10.04/i386/libmidgard2-2010_10.05.5.1-1_i386.deb 
wget http://download.opensuse.org/repositories/home:/midgardproject:/ratatoskr/xUbuntu_10.04/i386/midgard2-common_10.05.5.1-1_i386.deb
wget http://download.opensuse.org/repositories/home:/midgardproject:/ratatoskr/xUbuntu_10.04/i386/libmidgard2-dev_10.05.5.1-1_i386.deb 
sudo dpkg -i --force-depends libmidgard2-2010_10.05.5.1-1_i386.deb  
sudo dpkg -i midgard2-common_10.05.5.1-1_i386.deb
sudo dpkg -i libmidgard2-dev_10.05.5.1-1_i386.deb

# Enable dynamic loading of extensions in PHP
# FIXME: get php.ini path in a non-hardcoded way
echo "enable_dl=On" >> ~/.phpfarm/inst/php-5.3.8/lib/php.ini
