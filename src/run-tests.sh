#! /bin/bash
#
# run-tests.sh
# Copyright (C) 2014 evilissimo <evilissimo@gmail.com>
#
# Distributed under terms of the MIT license.
#

CPYTHON_SRC=${CPYTHON_SRC:-$HOME/devel/apps/cpython}
test -d $CPYTHON_SRC/Python || { echo "CPYTHON_SRC not correctly set: $CPYTHON_SOURCE_DIR/Python not found" && exit 1; }
mkdir -p results
for file in `find $CPYTHON_SRC -name "*.py"`
do
    echo "Parsing $file"
    ./parser-test $file &> ./results/`basename $file`.ast.out && rm -f ./results/`basename $file`.ast.out
done
