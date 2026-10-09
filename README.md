# preservation-code-police-by-mankind

Preservation and study of **Code Police**, a Java demo by **Mankind**, coded by
Krabob. The supplied Pouët snapshot records a February 2001 release and second
place in the PC demo competition at Synthesis Party 2001.

The demo now has a Java desktop restoration with the original 520 × 300 artwork,
640 × 480 cinema framing and AU audio. The engine is reconstructed from CFR and
checked against the unchanged original classes. Original media and scene script
remain separate resource files.

On Apple Silicon macOS, open `dist/java/macos-arm64/Code Police.app`; Java is
bundled. Close the window, click the close image, or press Escape to quit.

To build/run from source with JDK 25:

```sh
./java-desktop/gradlew -p java-desktop run
```

See [Java reactivation](documentation/java-reactivation.md) for build/package
commands, preservation correspondence, validation and remaining limitations.

- `original/`: unchanged user-supplied ZIP, extracted release and saved Pouët page.
- `documentation/quick-diagnosis.md`: findings, limitations and suggested next steps.
- `documentation/diagnostic-log.md`: dated investigation log.
- `evidence/`: SHA-256 manifest, class/media/script inventories, original bytecode
  disassembly and a reproducible inspection script.
- `reverse/`: independent CFR and Procyon reconstructions, retained unchanged.
- `java-desktop/`: desktop source, complete Gradle Wrapper and validation tools.
- `reference/`: documented captures from the original-bytecode host and restoration.
- `dist/java/`: generated applications with bundled Java runtimes (not committed).

See the [quick diagnosis](documentation/quick-diagnosis.md) for verification
commands and the distinction between original bytecode and future reconstructions.
