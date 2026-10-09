# stb_image

`stb_image.h` 2.30, upstream revision
`2c980bb59875b0d32144a71867fbdebb2f77cd20` from
https://github.com/nothings/stb . Copied from the locally preserved, pinned
Godog dependency, then verified by SHA-256:

- Header: `594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3`
- License: `bebfe904b14301657e4e5d655c811d51fd31b97c455b9cc2d8600d6bac6cff63`

Unmodified header; this application enables only the PNG decoder. Original GIF
and JPEG images are converted through Java's original decoder to external PNGs,
preserving the exact ARGB samples. See `assets/image-manifest.tsv` and
`tools/ExportNative.java`. No image is embedded in C++ source.
