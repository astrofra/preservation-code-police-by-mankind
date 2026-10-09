#!/bin/sh
set -eu
project=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root=$(CDPATH= cd -- "$project/.." && pwd)
"$root/java-desktop/gradlew" -p "$root/java-desktop" --console=plain referenceClasses
classpath="$root/java-desktop/build/classes/java/main:$root/java-desktop/build/classes/java/reference"
mkdir -p "$project/build/reference-tools"
javac --release 17 -cp "$classpath" -d "$project/build/reference-tools" "$project/tools/ExportNative.java"
classpath="$project/build/reference-tools:$classpath"
java -cp "$classpath" ExportNative assets "$root" "$project/assets"
java --add-opens=java.base/java.lang=ALL-UNNAMED -cp "$classpath" ExportNative reference "$root" "$project/build/reference.bin"
"$project/build.sh"
"$project/build/verify-native" "$project/assets" "$project/build/reference.bin" "$project/build/pcm"
