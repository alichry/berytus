#!/bin/sh

set -e

printZones() {
    local areaAP
    local locationAP
    local area
    local location
    for areaAP in /usr/share/zoneinfo/*
    do
        if [ ! -d "$areaAP" ]; then
            continue
        fi
        area="$(basename "$areaAP")"
        for locationAP in "$areaAP"/*
        do
            location="$(basename "$locationAP")"
            echo "$area/$location"
        done
    done
}

printZonesAsJSONArray() {
    local res
    res="["
    res+=$(printZones | sed -E 's|(.*)|\t"\1", |g')
    # remove last comma:
    res="${res%%, }"
    res+="]"
    echo $res
}

printZonesAsJSONArray

