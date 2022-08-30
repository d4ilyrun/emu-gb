#!/bin/sh

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No color

TEST_ROM_DIR="./tests/blargg/cpu_instrs/individual"
OUT_FILE="/tmp/blargg.out"

if [[ ! -f build/emu-gb ]]; then
    echo -e "$RED<!> Couldn't find executable (build/emu-gb) <!>$NC"
    exit 127
fi

if [[ ! -d $TEST_ROM_DIR ]]; then
    echo -e "$RED<!> Couldn't find test binaries (in $TEST_ROM_DIR) <!>$NC"
    exit 127
fi

[[ -d /tmp ]] || mkdir -p /tmp

run_test_rom() {
    local FILENAME=$(basename "$1")

    printf "%-30s" "${FILENAME}"

    timeout 5 ./build/emu-gb --blargg --exit-infinite-loop "$1" &> "$OUT_FILE"

    if [ $? == 124 ]; then
        printf "${RED}Inconclusive${NC}\n"
        return 1
    else
        grep 'Passed' "$OUT_FILE" &> /dev/null
        if [ $? == 0 ]; then
            printf "${GREEN}Passed${NC}\n"
            return 0
        else
            printf "${RED}Failed${NC}\n"
            return 1
        fi
    fi
}

main() {
    local failed_test=0

    for test_rom in ${TEST_ROM_DIR}/*.gb; do
        run_test_rom "$test_rom"

        if [ $? != 0 ]; then
            failed_test=1
        fi
    done

    return $failed_test
}

main
exit $?
