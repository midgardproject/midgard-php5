#!/bin/bash
valgrind -v php `pyrus get php_dir|tail -1`/pake.php test
