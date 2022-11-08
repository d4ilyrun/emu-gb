#!/usr/bin/env bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No color

EXECUTABLE="$1"
TEST_ROM_DIR="./tests/blargg/cpu_instrs/individual"
OUT_FILE="/tmp/blargg.out"

if [[ -z "$EXECUTABLE" ]]; then
    echo -e "${RED}USAGE$NC: ./run-blargg-tests.sh <emu-gb>"
    exit 127
fi

if [[ ! -d $TEST_ROM_DIR ]]; then
    echo -e "$RED<!> Couldn't find test binaries (in $TEST_ROM_DIR) <!>$NC"
    exit 127
fi

[[ -d /tmp ]] || mkdir -p /tmp

run_test_rom() {
    local FILENAME

    FILENAME=$(basename "$1")

    printf "%-30s" "${FILENAME}"

    timeout 5 "$EXECUTABLE" --blargg --exit-infinite-loop "$1" &> "$OUT_FILE"

    if [ $? == 124 ]; then
        echo -e "${RED}Inconclusive${NC}"
        return 1
    else
        if grep 'Passed' "$OUT_FILE" &> /dev/null; then
            echo -e "${GREEN}Passed${NC}"
            return 0
        else
            echo -e "${RED}Failed${NC}"
            return 1
        fi
    fi
}

main() {
    local failed_test

    failed_test=0

    for test_rom in "${TEST_ROM_DIR}"/*.gb; do
        if run_test_rom "$test_rom"; then
            failed_test=1
        fi
    done

    return $failed_test
}

main
exit $?
