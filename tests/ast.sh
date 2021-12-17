#!/bin/sh
set -e

command -v jq > /dev/null || {
    printf "jq is required\n"
    exit 1
}

exe="$1"
[ -f "$exe" ] && [ -x "$exe" ] || {
    printf "%s isn't executable\n" "$exe"
    exit 1
}

err=0
for f in ./*.json; do
    p="${f%.json}.bl"
    printf '%s ' "${p#./}"
    output="$($exe -a "$p" 2> /dev/null)" || {
        printf "\e[31merror: parsing failed\e[m\n"
        err=1
        continue
    }
    printf '%s' "$output" | jq -ne 'input == input' - "$f" > /dev/null || {
        printf "\e[31merror: ast differs\e[m\n"
        err=1
        continue
    }
    printf "\e[32mok\e[m\n"
done

exit $err
