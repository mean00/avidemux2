#! /bin/sh

if [[ -d config ]]; then
	rm -rf config
fi
mkdir -p config
if [[ -d m4 ]]; then
	rm -rf m4
fi
mkdir -p m4

#aclocal \
#&& libtoolize --force --copy \
#&& aclocal \
#&& autoheader \
#&& automake --foreign  --add-missing --copy \
#&& autoconf

autoreconf --force --install
