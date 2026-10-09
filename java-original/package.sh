#!/bin/sh
# Build the original-bytecode distribution. Requires a JDK 17+ and zip.
set -eu

PACKAGE_TOOLS_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPOSITORY_DIR=$(CDPATH= cd -- "$PACKAGE_TOOLS_DIR/.." && pwd)

if [ -n "${JAVA_HOME:-}" ]; then
    JAVAC_CMD="$JAVA_HOME/bin/javac"
    JAR_CMD="$JAVA_HOME/bin/jar"
else
    JAVAC_CMD=$(command -v javac || true)
    JAR_CMD=$(command -v jar || true)
fi
if [ ! -x "$JAVAC_CMD" ] || [ ! -x "$JAR_CMD" ]; then
    echo 'A JDK 17 or newer is required to build. Set JAVA_HOME or add javac and jar to PATH.' >&2
    exit 1
fi
if ! command -v zip >/dev/null 2>&1; then
    echo 'The zip command is required to build the distribution.' >&2
    exit 1
fi

mkdir -p "$PACKAGE_TOOLS_DIR/build" "$REPOSITORY_DIR/dist"
PACKAGE_STAGE=$(mktemp -d "$PACKAGE_TOOLS_DIR/build/package.XXXXXX")
trap 'rm -rf "$PACKAGE_STAGE"' 0
trap 'exit 1' HUP INT TERM
PACKAGE_NAME=code-police-java-original
PACKAGE_DIR="$PACKAGE_STAGE/$PACKAGE_NAME"
mkdir -p "$PACKAGE_STAGE/classes" "$PACKAGE_DIR/lib" "$PACKAGE_DIR/original/mkd_codepolice"

# Only the reference host and its support types enter the JAR, never the restored engine.
"$JAVAC_CMD" --release 17 -encoding UTF-8 -d "$PACKAGE_STAGE/classes" \
    "$REPOSITORY_DIR/java-desktop/src/main/java/SceneAssets.java" \
    "$REPOSITORY_DIR/java-desktop/src/main/java/DemoAudio.java" \
    "$REPOSITORY_DIR/java-desktop/src/main/java/DisplayOptions.java" \
    "$REPOSITORY_DIR/java-desktop/src/main/java/DemoWindow.java" \
    "$REPOSITORY_DIR/java-desktop/tools/ReferenceSupport.java" \
    "$REPOSITORY_DIR/java-desktop/tools/OriginalHost.java"
"$JAR_CMD" --create --file "$PACKAGE_DIR/lib/code-police-launcher.jar" \
    --main-class OriginalHost -C "$PACKAGE_STAGE/classes" .
cp -R "$REPOSITORY_DIR/original/mkd_codepolice/." "$PACKAGE_DIR/original/mkd_codepolice/"
find "$PACKAGE_DIR" -type f -name .DS_Store -exec rm -f {} +
cp "$PACKAGE_TOOLS_DIR/run-code-police.sh" "$PACKAGE_DIR/run-code-police.sh"
cp "$PACKAGE_TOOLS_DIR/DISTRIBUTION-README.md" "$PACKAGE_DIR/README.md"
chmod 755 "$PACKAGE_DIR/run-code-police.sh"

(cd "$PACKAGE_STAGE" && zip -X -q -r "$PACKAGE_NAME.zip" "$PACKAGE_NAME")

# Replace only these generated deliverables; unrelated dist/ applications are untouched.
if [ -e "$REPOSITORY_DIR/dist/$PACKAGE_NAME" ]; then
    mv "$REPOSITORY_DIR/dist/$PACKAGE_NAME" "$PACKAGE_STAGE/previous-package"
fi
mv "$PACKAGE_DIR" "$REPOSITORY_DIR/dist/$PACKAGE_NAME"
mv -f "$PACKAGE_STAGE/$PACKAGE_NAME.zip" "$REPOSITORY_DIR/dist/$PACKAGE_NAME.zip"
printf 'Package: %s\nArchive: %s\n' \
    "$REPOSITORY_DIR/dist/$PACKAGE_NAME" "$REPOSITORY_DIR/dist/$PACKAGE_NAME.zip"
