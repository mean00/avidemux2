#!/bin/sh
# Re-sign valid Avidemux app installed to the default location.
set -eu
set -o pipefail

APP="/Applications/Avidemux-2.8.2.app"

fail()
{
    echo "** $1 **" >&2
    exit 1
}

if [ ! -d "${APP}/Contents" ]; then
    fail "directory \"${APP}/Contents\" does not exist"
fi

sign()
{
    echo "  codesign $1"
    /usr/bin/codesign --force --sign - --timestamp=none "$1" || fail "ad-hoc signing $1"
}

# Deepest path first. Bundle paths do not contain newlines or tabs.
deepest_first()
{
    awk '{ n = split($0, parts, "/"); printf "%04d\t%s\n", n, $0 }' | sort -nr | cut -f 2-
}

# Foo.framework/Versions/<ver>/Foo is sealed when the framework bundle is signed.
# Signing that Mach-O on its own makes codesign treat it as having no resources.
is_framework_main_binary()
{
    parent=$(dirname "$1")
    grand=$(dirname "$parent")
    if [ "$(basename "$grand")" != "Versions" ]; then
        return 1
    fi
    fwdir=$(dirname "$grand")
    fwname=$(basename "$fwdir" .framework)
    [ "$(basename "$1")" = "$fwname" ]
}

should_sign_file()
{
    base=$(basename "$1")
    dir=$(dirname "$1")
    if [ "$dir" = "${APP}/Contents/MacOS" ]; then
        case "$base" in
            Avidemux*) return 1 ;;
        esac
    fi
    if is_framework_main_binary "$1"; then
        return 1
    fi
    if file -b "$1" | grep -q 'Mach-O'; then
        return 0
    fi
    case "$1" in
        "${APP}/Contents/MacOS/ADM_plugins6"/*)
            case "$base" in
                *.json|*.py|*.shader) return 0 ;;
            esac
            ;;
    esac
    return 1
}

echo "Ad-hoc signing ${APP} inside-out"

# Nested Mach-O files, then presets / scripts / shaders, deepest first.
# find -type f skips symlinks. The main executable is signed with the .app.
find "${APP}" -type f -print | deepest_first | while IFS= read -r f; do
    if should_sign_file "$f"; then
        sign "$f"
    fi
done

find "${APP}" -type d -name '*.framework' -print | deepest_first | while IFS= read -r fw; do
    sign "$fw"
done

sign "${APP}"

echo "Verifying ${APP}"
/usr/bin/codesign --verify --deep --strict --verbose=2 "${APP}" || fail "codesign --verify --deep --strict"

echo "** Ad-hoc signing completed. **"
