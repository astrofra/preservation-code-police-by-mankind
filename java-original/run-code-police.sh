#!/bin/sh
# Runs the original applet; all paths are relative to this script, not the caller.
set -eu

PACKAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ "${1:-}" = '--help' ] || [ "${1:-}" = '-h' ]; then
    echo 'Usage: ./run-code-police.sh [--resolution WIDTHxHEIGHT] [--scale 1..4] [--fullscreen] [--mute] [--smoke-test]'
    echo 'Fits the 520:300 artwork inside the requested window size, with nearest-neighbour scaling.'
    echo 'Fullscreen fills the current monitor with black bars; monitor resolution is unchanged.'
    echo 'Escape: quit. F11: toggle fullscreen.'
    echo 'Requires Java 17 through 25 (Java 21 or 25 recommended). Set JAVA_HOME to select Java.'
    exit 0
fi
if [ -n "${JAVA_HOME:-}" ]; then
    JAVA_CMD="$JAVA_HOME/bin/java"
else
    JAVA_CMD=$(command -v java || true)
fi
if [ ! -x "$JAVA_CMD" ]; then
    echo 'Java 17 through 25 is required. Set JAVA_HOME or add java to PATH.' >&2
    exit 1
fi
if ! JAVA_VERSION_OUTPUT=$("$JAVA_CMD" -version 2>&1); then
    printf '%s\n' "$JAVA_VERSION_OUTPUT" >&2
    echo 'Cannot start Java. Install Java 21 or 25, or set JAVA_HOME.' >&2
    exit 1
fi
JAVA_MAJOR=$(printf '%s\n' "$JAVA_VERSION_OUTPUT" | sed -n 's/.*version "\([0-9][0-9]*\).*/\1/p' | head -n 1)
case "$JAVA_MAJOR" in
    ''|*[!0-9]*)
        echo 'Cannot determine the Java version:' >&2
        printf '%s\n' "$JAVA_VERSION_OUTPUT" >&2
        exit 1 ;;
esac
if [ "$JAVA_MAJOR" -lt 17 ] || [ "$JAVA_MAJOR" -gt 25 ]; then
    echo "Java $JAVA_MAJOR is not supported by this original-applet launcher. Use Java 17 through 25." >&2
    echo 'The original demo requires the Applet API, removed in Java 26.' >&2
    exit 1
fi
if [ ! -f "$PACKAGE_DIR/lib/code-police-launcher.jar" ] || [ ! -f "$PACKAGE_DIR/original/mkd_codepolice/page.html" ]; then
    echo 'Incomplete package: keep run-code-police.sh, lib/ and original/ together.' >&2
    exit 1
fi
exec "$JAVA_CMD" -jar "$PACKAGE_DIR/lib/code-police-launcher.jar" "$PACKAGE_DIR" "$@"
