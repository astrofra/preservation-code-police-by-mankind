# preservation-code-police-by-mankind

Preservation and study of **Code Police**, a Java demo by **Mankind**, coded by
Krabob. The supplied Pouët snapshot records a February 2001 release and second
place in the PC demo competition at Synthesis Party 2001.

The initial static diagnosis finds an intact distribution, a complete dependency
graph from the applet entry point, and decodable media. No restored application
or playback validation has been produced yet.

- `original/`: unchanged user-supplied ZIP, extracted release and saved Pouët page.
- `documentation/quick-diagnosis.md`: findings, limitations and suggested next steps.
- `documentation/diagnostic-log.md`: dated investigation log.
- `evidence/`: SHA-256 manifest, class/media/script inventories, original bytecode
  disassembly and a reproducible inspection script.

See the [quick diagnosis](documentation/quick-diagnosis.md) for verification
commands and the distinction between original bytecode and future reconstructions.
