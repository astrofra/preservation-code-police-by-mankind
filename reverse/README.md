# Independent reconstructions

These files are **decompiled reconstructions**, not recovered Java source.
Inputs are the unchanged 39 classes in `original/mkd_codepolice/`.

- `cfr/`: CFR 0.152 JAR-mode output. Its `kraycasting.java` and `kSound.java`
  compile together and form the desktop reconstruction baseline.
- `cfr-unreferenced/`: separate CFR output for five nested classes omitted from
  its main output. These are outside the applet's static dependency closure.
  Qualified declarations emitted by this mode are documentary, not compilable
  standalone source.
- `procyon/`: independent Procyon 0.6.0 output with foreach reconstruction
  disabled. It includes all 26 nested classes, including the five above.

Procyon's `SplitSTag` places substring assignment and array indexing in a for-loop
update clause after an empty `if (index != -1) {}`. The original bytecode branches
around those operations when no opening delimiter remains. CFR preserves that
condition. Procyon's inclusion of the unused nested `kSound` also shadows the
active top-level `kSound` in source. These outputs are preserved without correction.

The 11 other top-level classes are outside the entry-point dependency closure;
some reference missing alternate-engine types. They remain archived and
decompiled, without inventing replacements. No decompiler reported a failed
method body in the active CFR reconstruction. The original-bytecode comparison
is the behavioral check; successful compilation alone is insufficient.

Reproduce into a new directory (CFR/Procyon CLI or `CFR_JAR` / `PROCYON_JAR`):

```sh
python3 java-desktop/tools/decompile.py --output java-desktop/build/redecompile
```

The generated JAR is only a decompiler input container; the original ZIP and
individual class bytes remain unchanged. See `evidence/decompilation/` for
the original tool and baseline compilation logs.

Tool documentation: [CFR](https://www.benf.org/other/cfr/),
[Procyon](https://github.com/mstrobel/procyon/wiki/Java-Decompiler).
