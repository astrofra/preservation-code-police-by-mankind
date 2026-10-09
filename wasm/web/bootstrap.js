// Browser presentation only. The artwork runs in the shared C++11 engine.
const $ = id => document.getElementById(id);
const base = new URL('./', import.meta.url);
const source = $('canvas'), screen = $('screen');
$('composition').appendChild(screen);
const context = screen.getContext('2d', {alpha: false});
let runtime, failed = false, playing = false;
let requested = {width: 1040, height: 600};
const fit = (w, h, bw = 520, bh = 300) => {
  const scale = Math.min(w / bw, h / bh);
  return {width: Math.max(1, Math.round(bw * scale)), height: Math.max(1, Math.round(bh * scale))};
};
function error(message) {
  if (failed) return; // Preserve the first, filename-bearing error from onAbort.
  failed = true;
  $('status').textContent = `Unable to start the demo: ${message}`;
  $('player-status').textContent = $('status').textContent;
  $('start').disabled = true;
  document.documentElement.dataset.state = 'error';
}
function readSize() {
  const custom = $('resolution').value === 'custom';
  $('custom-size').hidden = !custom;
  $('width').required = $('height').required = custom;
  $('width').disabled = $('height').disabled = !custom;
  const values = custom ? [Number($('width').value), Number($('height').value)] : $('resolution').value.split('x').map(Number);
  if (values.some(n => !Number.isInteger(n) || n < 1 || n > 16384)) return false;
  requested = fit(...values);
  $('size-note').textContent = `Image: ${requested.width} × ${requested.height}. Aspect ratio preserved; scaled to fit the available space.`;
  return true;
}
function present() {
  context.imageSmoothingEnabled = false;
  context.drawImage(source, 0, 0, screen.width, screen.height);
}
function layout() {
  if (!playing) return;
  const decorated = $('decor').checked;
  const fullscreen = Boolean(document.fullscreenElement);
  $('composition').classList.toggle('bare', !decorated);
  $('toggle-decor').setAttribute('aria-pressed', String(decorated));
  // Fullscreen fits the chosen presentation. Bare mode maximizes the 520:300
  // image; cinema mode keeps all four original border images visible.
  const bw = decorated ? 640 : 520, bh = decorated ? 480 : 300;
  const availableWidth = $('stage').clientWidth;
  const availableHeight = fullscreen ? $('stage').clientHeight : Math.max(1, innerHeight - 70);
  const limit = fullscreen ? Infinity : Math.min(requested.width / 520, requested.height / 300);
  const scale = Math.min(limit, availableWidth / bw, availableHeight / bh);
  const dpr = devicePixelRatio || 1;
  const align = value => Math.round(value * dpr) / dpr;
  const w = align(520 * scale), h = align(300 * scale);
  const compositionWidth = align(bw * scale), compositionHeight = align(bh * scale);
  $('composition').style.width = `${compositionWidth}px`;
  $('composition').style.height = `${compositionHeight}px`;
  $('composition').style.left = `${align((availableWidth - compositionWidth) / 2)}px`;
  $('composition').style.top = `${align((availableHeight - compositionHeight) / 2)}px`;
  $('cinema').style.transform = `scale(${scale})`;
  screen.style.left = `${decorated ? align(65 * scale) : 0}px`;
  screen.style.top = `${decorated ? align(13 * scale) : 0}px`;
  screen.style.width = `${w}px`; screen.style.height = `${h}px`;
  screen.width = Math.max(1, Math.round(w * dpr)); screen.height = Math.max(1, Math.round(h * dpr));
  // Use layout coordinates, not a fractional compositor transform: the latter
  // can filter a canvas even when getBoundingClientRect reports integer pixels.
  present();
}
async function enterFullscreen() {
  if (!$('stage').requestFullscreen) {
    $('player-status').textContent = 'Fullscreen is not available in this browser.';
    return;
  }
  try { await $('stage').requestFullscreen(); }
  catch { $('player-status').textContent = 'The browser did not allow fullscreen. You can keep watching in this page.'; }
  layout();
}
function start(event) {
  event.preventDefault();
  if (!runtime || failed || playing || !readSize()) return;
  playing = true;
  $('home').hidden = true; $('player').hidden = false;
  document.body.classList.add('playing');
  window.scrollTo(0, 0);
  $('player-status').textContent = '';
  layout();
  // Both operations occur inside the single user gesture, with no awaited fetch.
  if ($('fullscreen').checked) enterFullscreen();
  if (!runtime._cp_start($('sound').checked ? 0 : 1)) { home(); return; }
  screen.focus({preventScroll: true});
  document.documentElement.dataset.state = 'playing';
}
function home() {
  if (!playing) return;
  runtime._cp_stop(); playing = false;
  if (document.fullscreenElement) document.exitFullscreen().catch(() => {});
  $('home').hidden = false; $('player').hidden = true;
  document.body.classList.remove('playing');
  document.documentElement.dataset.state = failed ? 'error' : 'ready';
  $('start').focus({preventScroll: true});
}
$('launch').addEventListener('submit', start);
$('resolution').addEventListener('change', readSize);
for (const id of ['width', 'height']) $(id).addEventListener('input', readSize);
for (const id of ['close', 'back']) $(id).addEventListener('click', home);
$('toggle-decor').addEventListener('click', () => { $('decor').checked = !$('decor').checked; layout(); });
$('toggle-fullscreen').addEventListener('click', () => {
  if (document.fullscreenElement) document.exitFullscreen(); else enterFullscreen();
});
window.addEventListener('resize', layout);
document.addEventListener('fullscreenchange', layout);
document.addEventListener('keydown', event => {
  if (event.key === 'Escape' && playing && !document.fullscreenElement) home();
});
document.addEventListener('visibilitychange', () => {
  if (runtime) runtime._cp_suspend(document.hidden ? 1 : 0);
});
window.addEventListener('pagehide', () => { if (runtime) runtime._cp_stop(); });
readSize();

async function fetchFile(path, version) {
  const url = new URL(path, base);
  if (version) url.searchParams.set('v', version);
  for (let attempt = 0; attempt < 3; ++attempt) {
    const response = await fetch(url, {cache: version ? 'default' : 'no-cache', signal: AbortSignal.timeout(30000)});
    if (response.status === 429 && attempt < 2) {
      const retry = response.headers.get('Retry-After');
      let delay = 1000 * 2 ** attempt;
      if (retry !== null) {
        const parsed = /^\d+$/.test(retry) ? Number(retry) * 1000 : Date.parse(retry) - Date.now();
        if (Number.isFinite(parsed)) delay = Math.max(0, parsed);
      }
      // Respect server throttling; never retry sooner than Retry-After.
      if (delay > 30000) throw new Error(`${path} (HTTP 429: server busy; please try again later)`);
      await response.arrayBuffer();
      await new Promise(resolve => setTimeout(resolve, delay));
      continue;
    }
    if (!response.ok) throw new Error(`${path} (HTTP ${response.status})`);
    return new Uint8Array(await response.arrayBuffer());
  }
}
function restoreScriptNewlines(bytes) {
  // Some text-mode uploads convert the original CRLF script to LF. Restore
  // only missing CR bytes; the normal size/hash checks still apply afterwards.
  let missing = 0;
  for (let i = 0; i < bytes.length; ++i) if (bytes[i] === 10 && bytes[i - 1] !== 13) ++missing;
  if (!missing) return bytes;
  const restored = new Uint8Array(bytes.length + missing);
  let offset = 0;
  for (let i = 0; i < bytes.length; ++i) {
    if (bytes[i] === 10 && bytes[i - 1] !== 13) restored[offset++] = 13;
    restored[offset++] = bytes[i];
  }
  return restored;
}
async function prepare() {
  if (location.protocol === 'file:') throw new Error('use the preview-wasm.sh local server or an HTTP(S) host.');
  const manifest = JSON.parse(new TextDecoder().decode(await fetchFile('resources.json')));
  let completed = 0;
  const resources = new Map();
  const pending = [...manifest];
  await Promise.all(Array.from({length: 2}, async () => {
    while (pending.length) {
      const item = pending.shift();
      try {
        let bytes = await fetchFile(item.path, item.sha256);
        const received = bytes.length;
        if (item.path === 'assets/scene.txt') bytes = restoreScriptNewlines(bytes);
        if (bytes.length !== item.bytes) throw new Error(`size mismatch (expected ${item.bytes} bytes, received ${received})`);
        if (globalThis.crypto?.subtle) {
          const digest = await crypto.subtle.digest('SHA-256', bytes);
          const hash = [...new Uint8Array(digest)].map(n => n.toString(16).padStart(2, '0')).join('');
          if (hash !== item.sha256) throw new Error('file checksum mismatch (SHA-256)');
        }
        resources.set(item.path, bytes);
        if (!failed) $('status').textContent = `Loading the demo… ${++completed}/${manifest.length}`;
      } catch (cause) { throw new Error(`${item.path}: ${cause.message}`); }
    }
  }));
  let createCodePolice;
  try { ({default: createCodePolice} = await import('./code-police.js')); }
  catch (cause) { throw new Error(`code-police.js: ${cause.message}`); }
  runtime = await createCodePolice({
    canvas: source,
    locateFile: path => new URL(path, base).href,
    print: message => console.info(message), printErr: message => console.warn(message),
    onAbort: message => error(`code-police.wasm: ${message}`),
    onDemoError: error, onDemoFrame: present,
    preRun: [module => {
      module.FS.mkdirTree('/assets/Images'); module.FS.mkdirTree('/assets/Sounds');
      for (const [path, bytes] of resources) module.FS.writeFile('/' + path, bytes);
    }],
  });
  // Exposed for reproducible diagnostics; the normal UI uses only the host API.
  window.codePolice = runtime;
  document.title = 'CODE POLICE by Mankind';
  if (failed) return;
  $('status').textContent = 'Ready. Press Start to launch the demo.';
  $('start').disabled = false;
  document.documentElement.dataset.state = 'ready';
}
prepare().catch(cause => error(cause.message));
