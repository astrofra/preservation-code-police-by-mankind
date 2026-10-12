# iOS audio unlock

Date: 2026-10-11. Scope: the WebAssembly port's sound output on iOS Safari. The
shared C++11 engine, the decoded artwork and the original AU mixer are not
touched by this change.

## Reported symptom

On an iPhone 13 mini running iOS 26.6.2, the browser demo played but produced no
sound. The other browser hosts were unaffected. This matters because the
[WebAssembly notes](wasm-reactivation.md) previously stated explicitly that
physical Safari, iOS and Android devices were untested, and that the Playwright
WebKit build available for validation crashed before loading any page. Desktop
Chrome and Firefox were the only verified browsers; iOS behaviour was a known
gap rather than a regression.

## Nature of the change

Two files change, both browser-only:

| File | Change |
| --- | --- |
| `wasm/web/bootstrap.js` | Unlock Web Audio inside the Start gesture before SDL opens its device; add a tap-to-resume fallback. |
| `wasm/src/main.cpp` | Report the `AudioContext` state in `cp_status()`. No behaviour change to the engine or the mixer. |

No resource, asset, timing, PCM or engine file is modified. The original
ScriptProcessorNode backend is retained.

## Why the sound was missing

The sound path is: `cp_start()` → `openAudio()` → `SDL_OpenAudioDevice()` →
Emscripten's SDL2 port creates an `AudioContext` and a `ScriptProcessorNode`,
whose callback pulls from the shared `FilePlatform::mix`. SDL2's Emscripten
backend (`SDL_emscriptenaudio.c`, SDL 2.32.8) creates the context lazily inside
`SDL_OpenAudioDevice` and only arms its silence-timer auto-resume when
`audioContext.state === 'suspended'` **at that instant**. The context is
otherwise left alone; `resume()` is asynchronous and was not awaited.

That is adequate on desktop, where the context is frequently already running.
On iOS the context is created suspended and only becomes `running` inside a
user gesture, and `resume()` resolves later. The graph was therefore built and
connected while the context was still suspended, so the demo could run for its
whole timeline with no audible output and no error.

Two secondary factors made the symptom harder to read:

- On iOS the Web Audio output belongs to an audio session. Without a `playback`
  session the ring/silent switch and other apps can silence or duck the demo.
- The opening part of the script is intentionally silent for four seconds, so a
  short attempt can look like a total failure.

## Implementation

The unlock now happens before the device is opened, still inside the single
Start gesture, and nothing is fetched while it runs:

1. **Create and resume early.** `unlockAudio()` reuses or creates the
   `AudioContext` on the same `Module.SDL2` slot Emscripten uses, sets
   `navigator.audioSession.type = 'playback'` when the AudioSession API exists
   (guarded; absent on Chrome), awaits `context.resume()`, and plays a
   one-sample silent buffer to complete the unlock. It returns whether the
   context is `running`.
2. **Order Start around the await.** The submit handler still calls
   `preventDefault()` and requests fullscreen synchronously in the gesture, then
   waits for the unlock and calls `_cp_start` afterwards. `_cp_start` therefore
   opens SDL audio on an already running context.
3. **Fallback for stricter engines.** `watchAudio()` re-checks once after
   800 ms. If the context is still suspended it shows “Tap the picture to
   enable sound.” and retries on the first `pointerdown`/`touchstart` on the
   player. It is silent when sound is switched off or the context is running.
4. **Observable state.** `cp_status()` gains `audioState`: `0` no context,
   `1` running, `2` suspended, `3` closed, `4` other. It is reported even when
   audio is muted, which is what makes a remote diagnosis possible.

Because `_cp_start` is now reached from a promise continuation, the browser
validation scripts wait for `codePolice.SDL2.audio.scriptProcessorNode` before
wrapping `onaudioprocess`. They previously assumed startup was synchronous.

`cp_status()` remains a diagnostic string; no public host entry point changed
name or meaning, and `_cp_start`/`_cp_stop`/`_cp_suspend` keep their signatures.

## What is deliberately unchanged

- The C++ engine, the AU decoder, the mixer clamp and every PCM sample.
- The render-then-schedule-20 ms cadence and the 10 ms script ticks.
- `ScriptProcessorNode`, still deprecated but functional; an AudioWorklet
  migration would be a separate host change with its own verification.
- The original silent opening, which is part of the script, not a defect.

## Result

Confirmed working on the reporting device, an iPhone 13 mini with iOS 26.6.2.
The change was independently checked on desktop Chrome against the current
package: the context reports `running`, `audioState` is `1`, the
ScriptProcessorNode exists, the status string is well formed and the page
raises no errors. The `audioSession` guard is exercised as absent there.

## Remaining limits

- Reading `audioState` on a real device still needs a Mac and Safari's Web
  Inspector; there is no on-screen readout.
- The `navigator.audioSession` API is recent; when it is missing, the demo
  depends on the device's default session and the physical silent switch.
- Playwright WebKit still crashes at launch on the validation machine, so
  automated Safari coverage is unchanged. Desktop Chrome and Firefox remain the
  automated scope; iOS verification is manual.

## Reproduction

```sh
./build-wasm.sh
./preview-wasm.sh 8000
# A LAN address (for example http://192.168.1.13:8000/) allows a phone on the
# same network to load the extracted site. The preview script binds 127.0.0.1;
# bind 0.0.0.0 to test from another device.
```

Remote check, with the phone attached to Safari's Web Inspector:

```js
JSON.parse(codePolice.UTF8ToString(codePolice._cp_status()))
```

## References

- [WebAssembly reactivation](wasm-reactivation.md): timing, audio backend and
  previous browser coverage.
- [Native reactivation](native-reactivation.md): the shared engine and mixer.
- SDL2 2.32.8 `src/audio/emscripten/SDL_emscriptenaudio.c`, and Emscripten
  4.0.23 `$autoResumeAudioContext` in `src/lib/libcore.js`.
