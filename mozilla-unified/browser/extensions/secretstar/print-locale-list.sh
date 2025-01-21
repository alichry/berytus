#!/bin/sh

set -e

printLocales() {
    local localeAP
    local local
    for localeAP in /usr/share/locale/*
    do
        if [ ! -d "$localeAP" ]; then
            continue
        fi
        locale="$(basename "$localeAP")"
        case "$locale" in
            "UTF-8")
                continue
                ;;
            *.*)
                # strings like en_US.UTF-8 are skipped, for now.
                continue
                ;;
        esac
        echo "$locale"
    done
}

printLocalesAsJSONArray() {
    local res
    res="["
    res+=$(printLocales | sed -E 's|(.*)|\t"\1", |g')
    # remove last comma:
    res="${res%%, }"
    res+="]"
    echo $res
}

printLocalesAsJSONArray

