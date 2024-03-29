#!/usr/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
GRAY='\033[0;30m'
NC='\033[0m' # No color

TEST_DIR=$1

if [[ -z "$TEST_DIR" ]]; then
    echo -e "${RED}USAGE$NC: ./run-tests.sh <test_dir>"
    exit 127
fi

if [[ ! -d "$TEST_DIR" ]]; then
    echo -e "$RED<!> Couldn't find test binaries (in result/tests) <!>$NC"
    exit 127
fi

FAILED=0
VERBOSE=$(test "$1" = "-v"; echo $?)

cd "$TEST_DIR" || exit

echo -e "====${GRAY} Running tests in result/tests $NC===="

tests=$(find -- * -type f -and -executable)

for test in $tests; do
    if [[ $VERBOSE -eq 0 ]]; then
        echo -e "$GRAY======== Running $test ========$NC"
        "./$test"
        FAILED=$((FAILED + 1))
    else
        echo -ne "- $test:"
        if "./$test" &> /dev/null; then
            echo -e "${GREEN}OK"
        else
            FAILED=$((FAILED + 1))
            echo -e "${RED}KO"
        fi
        echo -ne "$NC"
    fi
done

echo -e "====${GRAY}     Finished running tests    $NC====\n"

if [[ $FAILED -eq 0 ]]; then
    echo -e "${GREEN}ALL TESTS PASSED SUCESSFULLY!"
else
    echo -e "${RED}FAILED $FAILED TEST(S)"
fi

exit $FAILED
