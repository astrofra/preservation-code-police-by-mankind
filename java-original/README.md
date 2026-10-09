# Original Java distribution

Build the package from any working directory:

```sh
./java-original/package.sh
```

Use the script's absolute path when outside the repository. Building requires
a **JDK 17 or newer** (`javac` and `jar` on PATH, or `JAVA_HOME`) and `zip`.
It does not require Gradle. Outputs:

```text
dist/code-police-java-original.zip
dist/code-police-java-original/
    run-code-police.sh
    README.md
    lib/code-police-launcher.jar
    original/mkd_codepolice/...
```

Distribute the ZIP. After extraction, recipients run `./run-code-police.sh`,
or `sh run-code-police.sh` if their extractor drops executable permissions.
The archive preserves the executable bit. The runtime requires **Java 17–25**;
Java 21 and 25 were tested. Java 26+ lacks the original applet API and the script
reports this explicitly. `JAVA_HOME` selects the runtime when set; otherwise the
launcher uses `java` from PATH. Java is not bundled.

The JAR contains only `OriginalHost`, `ReferenceSupport`, `SceneAssets`,
`DemoAudio` and their generated support classes. The last two supply existing
host utilities; live reference playback uses the original `AudioClip` API.
The restored engine is not included. The original 39 classes, all media and HTML,
and release notes are copied unchanged as 107 external files. The host reads
`page.html` only for the scene script and opens a fixed 520 × 300 applet window.

`OriginalHost` now rejects malformed command lines and supports the optional
`--smoke-test` flag. That flag checks a running worker and the 520 × 300 surface,
closes the window after eight seconds and waits for the original worker to stop.
The fixed window avoids resizing an applet whose buffers are initialized once.
No archived class or reconstructed rendering code was changed for packaging.

Validation used the extracted ZIP, an unrelated working directory (`/tmp`),
a package path containing spaces and an accented character, and a macOS sandbox
denying network access. Both Java 21 and Java 25 launch/close checks passed.
ZIP CRCs, all original bytes, launcher permissions and rejection of Java 26 were
checked. These are short packaging checks; the earlier renderer comparisons are
documented in `documentation/java-reactivation.md`. Linux/Windows playback and
Java 17 have not been tested. Results are retained under
`evidence/java-original-package/`.

The packaged user-facing instructions are sourced from `DISTRIBUTION-README.md`.
Generated output is ignored by Git; both shell scripts and this build recipe
are versioned. Re-running the builder replaces only these generated deliverables.
