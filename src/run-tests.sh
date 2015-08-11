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

BADSYNNOCARET=0
BADSYN3=0
BADSYN8=0
BADSYN9=0
PY3TEST=0
FAILURE=0
for result in `ls ./results`;
do
    case $result in
        badsyntax_future3.py.ast.out)
            BADSYN3=1
            continue
        ;;
        badsyntax_future8.py.ast.out)
            BADSYN8=1
            continue
        ;;
        badsyntax_future9.py.ast.out)
            BADSYN9=1
            continue
        ;;
        py3_test_grammar.py.ast.out)
            PY3TEST=1
            continue
        ;;
        badsyntax_nocaret.py.ast.out)
            BADSYNNOCARET=1
            continue
        ;;
    esac
    FAILURE=1
    echo "Failed to parse $result"
done;

SUCCESS=1
if [ $FAILURE = 0 ] && [ $BADSYN3 = 1 ] && [ $BADSYN8 = 1 ] && [ $BADSYN9 = 1 ] && [ $PY3TEST = 1 ] && [ $BADSYNNOCARET = 1 ];
then
    SUCCESS=0
fi
exit $SUCCESS
