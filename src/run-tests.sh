#! /bin/bash
#
# run-tests.sh
# Copyright (C) 2014 evilissimo <evilissimo@gmail.com>
#
# Distributed under terms of the MIT license.
#


mkdir -p results
for file in `find ~/devel/apps/cpython/ -name "*.py"`;
do
    echo "Parsing $file";
    ./parser-test $file > ./results/`basename $file`.ast.out && rm -f ./results/`basename $file`.ast.out;
done

