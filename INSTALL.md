Prerequisites
=============

1. php 5.2 or php 5.3 (including "developer" packages, for those OSes which package those separately)

2. Pake. See http://wiki.github.com/indeyets/pake/installing-pake for instructions

3. midgard-core library installed


Installation
============

If all prerequisites are installed, just run the following command from regular user account. Pake will request superuser privileges if/when it is required:

    pake install

In case if you need to specify custom path to phpize (part of php's developer tools) use this command:

    pake install --with-phpize=/path/to/bin/phpize

If there are some problems with pake, you can do all the stuff manually:

    /path/to/bin/phpize
    ./configure --with-php-config=/path/to/bin/php-config
    make
    sudo make install


Configuration
=============

Edit your php.ini to add:

    extension=midgard2.so

and make sure that extension_dir at least contains the output of `php-config --extension-dir`

If you plan to use midgard as part in web-environment, then remember to fully restart your web-server


midgard's php.ini configuration directives
------------------------------------------

**midgard.engine** – Disable or enable midgard engine. Default is 'On'.

**midgard.http** – Disable or enable automatic connection to database. This one is recommended for web-applications. Default is 'Off'

**midgard.configuration** – Name of midgard unified configuration file. This name is looked for in default configuration files directory. Default is ''.

**midgard.configuration_file** – Full path of midgard unified configuration file. Overrides midgard.configuration key. Default is ''.

**midgard.superglobals\_compat** — Disable or enable registration of $\_MIDGARD and $\_MIDCOM superglobal variables. Deprecated. Default is 'Off'

**midgard.memory\_debug** — Dump a lot of debugging information to stdout. Useful **ONLY** to developers. Default is 'Off'
