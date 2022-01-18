#!/usr/bin/env bash

set -e

git diff --cached --name-only --diff-filter=ACMRT |
  grep "\.[cmh]$" |
  xargs -n1 clang-format --dry-run -Werror
  >/dev/null

if [ $? -ne 0 ]; then
    echo "Commit did not match clang-format"
    exit 1
fi

exit 0
