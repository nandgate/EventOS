#!/bin/bash
#set -x

# SPDX-License-Identifier: 0BSD
# Copyright (C) 2016-2026 NAND Gate Technologies, LLC

set -o pipefail

CC_NO_WARN="-Wno-int-conversion -Wno-pointer-to-int-cast -Wno-unknown-pragmas "
CC_WITH_GCOV="" # set below when coverage is enabled via the command line
CC_FLAGS="-std=gnu11 -Wall ${CC_NO_WARN}  -I./include -I../app/include"

runCoverage() {
    mainFilePath=$1
    testFilePath=$2
    mainFile=${mainFilePath##*/}
    testFile=${testFilePath##*/}
    # generate coverage data for just this test
    lcov --quiet --capture --branch-coverage --exclude test_* --directory . --output-file c.info | tee -a runlog.txt

    # append this test to the entire suit of test
    lcov --quiet --add-tracefile "*.info" --branch-coverage --output-file cc.info | tee -a runlog.txt

    # cleanup garbage
    rm c.info
    rm *.gcda
    rm *.gcno
}

runTest() {
    echo | tee -a runlog.txt
    echo "gcc -o test.exe $CC_FLAGS ${CC_WITH_GCOV} $*" | tee -a runlog.txt
    gcc -o test.exe $CC_FLAGS ${CC_WITH_GCOV} $* | tee -a runlog.txt
    ./test.exe | tee -a runlog.txt
    testResult=$?

    if [ ${CC_WITH_GCOV} ]; then
        runCoverage $* | tee -a runlog.txt
    fi

    rm test.exe
    if [ $testResult -ne 0 ]; then
        exit -1
    fi
}

runAutoTests() {
    appPath="../app/source"${1:+/$1}
    for sourceFilePath in $(find $appPath -name '*.c'); do
        # TODO: black list manually specified source files.
        sourceFileName=${sourceFilePath##*/}
        sourcePath=${sourceFilePath%/*}
        modulePath=${sourcePath#*app/}
        testFileName="test_${sourceFileName}"
        testFilePath="${modulePath}/$testFileName"

        if [ -e "$testFilePath" ]; then
            runTest $sourceFilePath $testFilePath
        else
            echo "No unit test for source file: " ${sourceFilePath}
            echo "See this source file for an explanation for why this module has no unit tests."
        fi
    done

    if [ ${CC_WITH_GCOV} ] ; then
      genhtml --quiet  cc.info --output-directory html/
      rm -f cc.info
    fi

#    zip -r9q gcov.zip gcov
}

# Clean up garbage from last time, if any
rm -f assertions.txt
rm -f runlog.txt
rm -f c.info
rm -f cc.info
rm -f *.gcda
rm -f *.gcno

# Mode of operation
# test.sh : Run all tests
# test.sh module : Run tests for module
# test.sh --coverage : Run all tests and produce HTML coverage report

if [ "$1" == "--coverage" ]; then
  rm -drf html/
  CC_WITH_GCOV="--coverage"
  runAutoTests
else
  runAutoTests $1
fi

if [ -e "assertions.txt" ]
then
    echo | tee -a runlog.txt
    echo -n Total Assertions: | tee -a runlog.txt
    cat assertions.txt | tee -a runlog.txt
fi


