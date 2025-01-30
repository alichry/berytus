#!/bin/sh

set -e

npm run build

cd ..

basedir="secretstar"

tar -c \
    -zvf $basedir/secretstar.tar.gz \
    $basedir/manifest.json \
    $basedir/public \
    $basedir/build \
    $basedir/build.worker

