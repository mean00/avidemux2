#!/bin/bash
# Create a snapshot of the current head of the stable branch in the current
# directory, e.g. as x264-0.159-20200409git296494a.tar.bz2
# Adapted from RPM Fusion x264 Fedora package.

set -e

tmp=$(mktemp -d)

trap cleanup EXIT
cleanup() {
    set +e
    [ -z "$tmp" -o ! -d "$tmp" ] || rm -rf "$tmp"
}

unset CDPATH
pwd=$(pwd)
package=x264
branch=stable
commit=HEAD

pushd "$tmp"
git clone https://code.videolan.org/videolan/x264.git -b ${branch}

cd ${package}
tag=$(git rev-list HEAD -n 1 | cut -c 1-7)
git checkout ${commit}
./version.sh > version.h
API="$(grep '#define X264_BUILD' < x264.h | sed 's/^.* \([1-9][0-9]*\).*$/\1/')"
date=$(git log -1 --format=%cd --date=short | tr -d \-)
git archive --prefix="${package}-0.$API-${date}git${tag}/" --format=tar stable | bzip2 > "$pwd"/${package}-0.$API-${date}git${tag}.tar.bz2
popd >/dev/null
