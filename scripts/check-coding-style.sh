#!/usr/bin/env bash

# Check given files against our coding style

function git_root(){
    git rev-parse --show-toplevel
}

GIT_ROOT=$(git_root)

# Check a single file
# args:
#   $1=filepath (relative to git root)
function check_file() {
    local file
    local check

    file="$GIT_ROOT/$1"
    check="clang-tidy"

    [[ $FIX_CSTYLE -eq 1 ]] && check="$check --fix" # Fix if FIX_CSTYLE=1

    eval $check --format-style=file \
        --config-file="$GIT_ROOT/.clang-tidy" \
        --quiet \
        "$file" \
    &> /dev/null

    return $?
}

FAILED=0

cd "$GIT_ROOT/scripts" || exit
source "utils/log.sh"

for file in $@
do
    if ! check_file "$file"
    then
        print_error "$file does not respect the coding style"
        FAILED=1
    fi
done

exit $FAILED
