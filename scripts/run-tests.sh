#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
GRAY='\033[0;30m'
NC='\033[0m' # No color

if [[ ! -d result/tests ]]; then
    echo -e "$RED<!> Couldn't find test binaries (in result/tests) <!>$NC"
    exit 127
fi

FAILED=0
VERBOSE=$(test "$1" = "-v"; echo $?)

cd result/tests

echo -e "====${GRAY} Running tests in result/tests $NC===="

for test in $(ls *_test); do
    if [[ $VERBOSE -eq 0 ]]; then
        echo -e "$GRAY======== Running $test ========$NC"
        "./$test"
        FAILED=$((FAILED + $?))
        echo -e "$GRAY===============================$NC"
    else
        echo -ne "$GREY- $test: $NC"
        "./$test" > /dev/null
        if [[ $? -eq 0 ]]; then
            echo -e "${GREEN}OK"
        else
            FAILED=$((FAILED + 1))
            echo -e "${RED}KO"
        fi
        echo -ne $NC
    fi
done

echo -e "====${GRAY}     Finished running tests    $NC===="

if [[ $FAILED -eq 0 ]]; then
    echo -e "${GREEN}ALL TEST SUCCESFULLY PASSED!"
else
    echo -e "${RED}FAILED $FAILED TESTS"
fi

exit $FAILED
