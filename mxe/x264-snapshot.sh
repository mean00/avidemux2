#!/bin/bash
# Create a snapshot of the current head of the stable branch in the current
# directory, e.g. as x264-0.159-20200409git296494a.tar.bz2 and generate
# the corresponding patch for MXE.
# Derived from RPM Fusion x264 Fedora package.
failMsg()
{
    echo "** $1 **"
    exit 1
}

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
git clone https://code.videolan.org/videolan/x264.git -b ${branch} || failMsg "Cannot download source code"

cd ${package}
tag=$(git rev-list HEAD -n 1 | cut -c 1-7)
git checkout ${commit}
./version.sh > version.h
API="$(grep '#define X264_BUILD' < x264.h | sed 's/^.* \([1-9][0-9]*\).*$/\1/')"
date=$(git log -1 --format=%cd --date=short | tr -d \-)
version="0.$API-${date}git${tag}"
outfile="${package}-${version}.tar.bz2"
git archive --prefix="${package}-${version}/" --format=tar stable | bzip2 > "$pwd/${outfile}" || failMsg "Cannot create archive"
popd >/dev/null

# test run
sha256sum "${outfile}" >/dev/null || failMsg "Cannot calculate checksum"
# now for real
checksum=$(sha256sum "${outfile}" | cut -d \  -f 1)

cat > x264_gen.patch <<EOF
diff --git a/src/x264.mk b/src/x264.mk
index 0170301b..1189ccd0 100644
--- a/src/x264.mk
+++ b/src/x264.mk
@@ -3,20 +3,13 @@
 PKG             := x264
 \$(PKG)_WEBSITE  := https://www.videolan.org/developers/x264.html
 \$(PKG)_IGNORE   :=
-\$(PKG)_VERSION  := 20180806-2245
-\$(PKG)_CHECKSUM := 9f876c88aeb21fa9315e4a078931faf6fc0d3c3f47e05a306d2fdc62ea0afea2
-\$(PKG)_SUBDIR   := \$(PKG)-snapshot-\$(\$(PKG)_VERSION)
-\$(PKG)_FILE     := \$(PKG)-snapshot-\$(\$(PKG)_VERSION).tar.bz2
-\$(PKG)_URL      := https://download.videolan.org/pub/videolan/\$(PKG)/snapshots/\$(\$(PKG)_FILE)
+\$(PKG)_VERSION  := ${version}
+\$(PKG)_CHECKSUM := ${checksum}
+\$(PKG)_SUBDIR   := \$(PKG)-\$(\$(PKG)_VERSION)
+\$(PKG)_FILE     := \$(PKG)-\$(\$(PKG)_VERSION).tar.bz2
+\$(PKG)_URL      :=
 \$(PKG)_DEPS     := cc liblsmash \$(BUILD)~nasm
 
-define \$(PKG)_UPDATE
-    \$(WGET) -q -O- 'https://git.videolan.org/?p=x264.git;a=shortlog' | \\
-    \$(SED) -n 's,.*\([0-9]\{4\}\)-\([0-9]\{2\}\)-\([0-9]\{2\}\).*,\1\2\3-2245,p' | \\
-    \$(SORT) | \\
-    tail -1
-endef
-
 define \$(PKG)_BUILD
     cd '\$(BUILD_DIR)' && AS='\$(PREFIX)/\$(BUILD)/bin/nasm' '\$(SOURCE_DIR)/configure'\\
         \$(MXE_CONFIGURE_OPTS) \\
EOF
