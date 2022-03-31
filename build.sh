#!/usr/bin/env bash
pushd test
time ./test.sh
testResult=$?
popd

if [ $testResult -ne 0 ]
then
    echo "unit test failures- build halted"
    exit 0
fi

make
make docs