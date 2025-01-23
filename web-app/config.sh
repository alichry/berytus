#!/bin/sh

set -e
set -o noclobber # avoid overwriting files with the redirect, i.e. '>'

# generate db user and root pass

if [ -f "./dbpass.txt" ]; then
    echo "./dbpass.txt already exists. Refusing to overwrite."  >&2
    exit 1
fi

if [ -f "./dbrootpass.txt" ]; then
    echo "./dbrootpass.txt already exists. Refusing to overwrite."  >&2
    exit 1
fi

LC_ALL=C tr -dc A-Za-z0-9 </dev/urandom | head -c 16 > ./dbpass.txt
LC_ALL=C tr -dc A-Za-z0-9 </dev/urandom | head -c 16 > ./dbrootpass.txt

echo "DB passwords generated."