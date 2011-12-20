#!/bin/bash
valgrind -v --log-file=segfault.log php -r "dl('midgard2.so'); print_r(new midgard_query_storage('midgard_person'));"
#valgrind -v --log-file=segfault.log php `pyrus get php_dir|tail -1`/pake.php test
cat segfault.log
