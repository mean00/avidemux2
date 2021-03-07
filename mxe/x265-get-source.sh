#!/bin/bash
# Clone x265 Mercurial repository, update to tag and create a source archive.
fail()
{
    echo "$1"
    exit 1
}

which hg > /dev/null 2>&1 || fail "Mercurial is not in PATH, cannot proceed."

set -e

tmp=$(mktemp -d)

trap cleanup EXIT
cleanup()
{
    set +e
    [ -z "$tmp" -o ! -d "$tmp" ] || rm -rf "$tmp"
}

unset CDPATH
pwd=$(pwd)
package=x265
tag=3.4
branch=Release_${tag}

pushd "$tmp"
hg clone https://hg.videolan.org/x265#${branch} || fail "Repository cloning failed, cannot proceed."
cd "${package}" || fail "Cannot change directory to ${package} ??"
hg update "${tag}" || fail "Cannot checkout ${tag}, cannot proceed."
hg archive -p "${package}_${tag}" -t tgz "${package}_${tag}.tar.gz" || fail "Cannot create archive"
cp -vu "${package}_${tag}.tar.gz" "${pwd}/"

popd >/dev/null
