#!/bin/bash
valgrind -v --log-file=segfault.log php `pyrus get php_dir|tail -1`/pake.php test
cat segfault.log
