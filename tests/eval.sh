#!/bin/sh
set -e

exe="$1"
[ -f "$exe" ] && [ -x "$exe" ] || {
    printf "%s isn't executable\n" "$exe"
    exit 1
}

err=0
for f in ./*.out; do
    p="${f%.out}.bl"
    printf '%s ' "${p#./}"
    output="$($exe "$p" 2> /dev/null)" || {
        printf "\e[31merror: couldn't execute\e[m\n"
        err=1
        continue
    }
    [ "$output" = "$(cat "$f")" ] || {
        printf "\e[31merror: output differs\e[m\n"
        err=1
        continue
    }
    printf "\e[32mok\e[m\n"
done

exit $err
