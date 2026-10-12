// Verify the extracted ZIP in a nested, accented URL, with no external requests.
import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {createServer} from 'node:http';
import {mkdtemp, mkdir, readFile, writeFile, rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
import {chromium, firefox, webkit} from 'playwright';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const scratch = await mkdtemp(path.join(tmpdir(), 'Code Police été '));
const site = path.join(scratch, 'démos', 'Code Police été');
const zip = path.join(root, 'dist/wasm/code-police-web.zip');
const output = path.join(root, 'evidence/wasm');
const captures = path.join(root, 'reference/wasm', process.env.CODEPOLICE_CAPTURE_SET || 'navy-en');
await mkdir(site, {recursive: true});
await mkdir(output, {recursive: true});
await mkdir(captures, {recursive: true});
execFileSync('python3', ['-m', 'zipfile', '-e', zip, site]);
execFileSync('python3', [path.join(root, 'wasm/prepare-reference.py')]);
const reference = JSON.parse(await readFile(path.join(root, 'wasm/build/reference.json')));
const hash = bytes => createHash('sha256').update(bytes).digest('hex');
const report = {package: 'dist/wasm/code-police-web.zip', sha256: hash(await readFile(zip)),
  captures: path.relative(root,captures),
  platform: `${process.platform}/${process.arch}`, browsers: [], untested: ['Physical Safari', 'Physical mobile devices'],
  unavailable: ['Playwright WebKit mac14 ARM64 build 2251: bus error at browser launch before page load on macOS 14.1; see webkit-launch.txt']};
let files = 0;
for (const line of (await readFile(path.join(site, 'SHA256SUMS'), 'utf8')).trim().split('\n')) {
  const [expected, name] = line.split('  ');
  assert.equal(hash(await readFile(path.join(site, name))), expected, name); ++files;
}
const originals = JSON.parse(await readFile(path.join(root, 'evidence/original-manifest.json')));
let preserved = 0;
for (const file of originals) {
  assert.equal(hash(await readFile(path.join(root, file.path))), file.sha256, file.path);
  if (file.path.startsWith('original/mkd_codepolice/')) {
    assert.equal(hash(await readFile(path.join(site, 'original', file.path.slice('original/mkd_codepolice/'.length)))), file.sha256);
    ++preserved;
  }
}
report.integrity = {files, originalsPreserved: preserved, repositoryOriginals: originals.length};
const mime = {'.html':'text/html; charset=utf-8', '.js':'text/javascript', '.wasm':'application/wasm',
  '.css':'text/css', '.png':'image/png', '.jpg':'image/jpeg', '.gif':'image/gif', '.json':'application/json'};
const server = createServer(async (req, res) => {
  try {
    let name = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
    if (name.endsWith('/')) name += 'index.html';
    const file = path.resolve(scratch, '.' + name);
    assert(file.startsWith(scratch + path.sep));
    const bytes = await readFile(file);
    res.writeHead(200, {'Content-Type': mime[path.extname(file)] || 'application/octet-stream'});
    res.end(bytes);
  } catch {res.writeHead(404); res.end('Not found');}
});
await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
const origin = `http://127.0.0.1:${server.address().port}`;
const url = `${origin}/d%C3%A9mos/Code%20Police%20%C3%A9t%C3%A9/`;
const status = page => page.evaluate(() => JSON.parse(codePolice.UTF8ToString(codePolice._cp_status())));
async function boundedEvaluate(page, fn, arg) {
  let timer;
  try {
    return await Promise.race([page.evaluate(fn,arg),new Promise((_,reject)=>{
      timer=setTimeout(()=>reject(new Error('Browser evaluation exceeded 30 seconds')),30000);
    })]);
  } finally {clearTimeout(timer);}
}
async function ready(page) {
  await page.goto(url, {timeout: 30000});
  await page.waitForFunction(() => ['ready','error'].includes(document.documentElement.dataset.state), null, {timeout: 30000});
  assert.equal(await page.locator('html').getAttribute('data-state'), 'ready', await page.locator('#status').textContent());
}
async function geometry(page) {
  return page.evaluate(() => {
    const canvas = document.getElementById('screen'), r = canvas.getBoundingClientRect();
    const composition = document.getElementById('composition').getBoundingClientRect();
    const original = document.getElementById('canvas');
    const source = original.getContext('2d').getImageData(0,0,520,300).data;
    const pixels = canvas.getContext('2d').getImageData(0,0,canvas.width,canvas.height).data;
    const colours = new Set();
    const colour = (a,i) => (a[i]<<16)|(a[i+1]<<8)|a[i+2];
    for(let i=0;i<source.length;i+=4) colours.add(colour(source,i));
    let interpolated = 0;
    for(let i=0;i<pixels.length;i+=4) if(!colours.has(colour(pixels,i))) ++interpolated;
    return {width:r.width,height:r.height,x:r.x,y:r.y,buffer:[canvas.width,canvas.height],
      source:[original.width,original.height],composition:{width:composition.width,height:composition.height},
      dpr:devicePixelRatio,interpolated,fullscreen:!!document.fullscreenElement};
  });
}
async function capturePresentation(page,engine,label) {
  const dir=path.join(root,'wasm/build/presentation');await mkdir(dir,{recursive:true});
  const g=await geometry(page);
  const png=await page.evaluate(()=>document.getElementById('screen').toDataURL().split(',')[1]);
  await writeFile(path.join(dir,`${engine}-${label}.png`),Buffer.from(png,'base64'));
  await writeFile(path.join(dir,`${engine}-${label}.json`),JSON.stringify(g));
  await page.screenshot({path:path.join(captures,`${engine}-${label}.png`)});
  return JSON.parse(execFileSync('python3',[path.join(root,'wasm/check-presentation.py'),engine,label,captures],{encoding:'utf8'}));
}
try {
  for (const engine of (process.env.CODEPOLICE_BROWSERS || 'chromium,firefox').split(',')) {
    console.log(`Testing ${engine}…`);
    const browser = await {chromium,firefox,webkit}[engine].launch({headless:true, timeout:20000,
      ...(engine==='chromium' ? {channel:'chrome'} : {})});
    const result = {engine,version:browser.version(),checks:[],geometries:[]};
    try {
      const context = await browser.newContext({viewport:{width:1600,height:1120},deviceScaleFactor:1,timezoneId:'Pacific/Honolulu'});
      context.setDefaultTimeout(15000);
      const external = [], errors = [], bad = [];
      await context.route('**/*', route => {
        if(new URL(route.request().url()).origin===origin) return route.continue();
        external.push(route.request().url()); return route.abort();
      });
      const page = await context.newPage();
      page.on('pageerror', e=>errors.push(e.message));
      page.on('response', r=>{if(r.status()>=400)bad.push(r.url());});
      await ready(page);
      assert.equal(await page.locator('title').textContent(),'CODE POLICE by Mankind');
      assert.equal(await page.locator('html').getAttribute('lang'),'en');
      assert.equal(await page.locator('h1').innerText(),'CODE POLICE');
      assert.equal(await page.locator('.byline').innerText(),'by Mankind');
      assert.equal(await page.locator('.intro').count(),0);
      assert.equal(await page.locator('.credits').innerText(),'Code\nKrabob\nGraphics\nzaac, alexx, myke, krabob\nSound\nLeviathan, Tex');
      assert.match(await page.locator('a', {hasText:'Pouët'}).getAttribute('href'), /which=1788$/);
      assert.match(await page.locator('a', {hasText:'GitHub'}).getAttribute('href'), /astrofra\/preservation-code-police-by-mankind$/);
      await page.screenshot({path:path.join(captures, `${engine}-home.png`)});
      assert.equal((await status(page)).frames,0);
      // Mouse press alone must not start; release dispatches the single click.
      const button = await page.locator('#start').boundingBox();
      await page.mouse.move(button.x+button.width/2,button.y+button.height/2);
      await page.mouse.down(); assert.equal((await status(page)).starts,0); await page.mouse.up();
      // The host unlocks Web Audio inside the Start gesture before opening the
      // SDL device; wait for that asynchronous step before wrapping the node.
      await page.waitForFunction(()=>codePolice.SDL2?.audio?.scriptProcessorNode);
      await page.evaluate(()=>{
        window.audioProof={peak:0,callbacks:0};
        const node=codePolice.SDL2.audio.scriptProcessorNode, original=node.onaudioprocess;
        node.onaudioprocess=function(event){
          original.call(this,event); ++audioProof.callbacks;
          for(const n of event.outputBuffer.getChannelData(0)) audioProof.peak=Math.max(audioProof.peak,Math.abs(n));
        };
      });
      await page.waitForFunction(()=>JSON.parse(codePolice.UTF8ToString(codePolice._cp_status())).frames>=20);
      const before = await status(page); await page.waitForTimeout(500); const after = await status(page);
      assert(after.tick>before.tick && after.frames>before.frames); assert.equal(after.starts,1); assert.equal(after.audio,1);
      assert.equal(await page.evaluate(()=>codePolice.SDL2.audioContext.state),'running');
      // The opening scene intentionally starts/stops voices, and is silent for
      // four seconds. Check actual WebAudio output after its first looping cue.
      await page.waitForFunction(()=>audioProof.peak>0,null,{timeout:12000});
      result.audio=await page.evaluate(()=>audioProof);
      result.live = {before,after};
      result.checks.push('Start on release exactly once; live clock/frame progression; SDL audio device');
      // Freeze the reference clock and run the exact Java export schedule.
      await page.evaluate(()=>codePolice._cp_test_reset());
      const pcm = await page.evaluate(async()=>{
        const hashes={};
        for(let i=0;i<10;++i){
          const name=codePolice.UTF8ToString(codePolice._cp_test_pcm_name(i));
          const ptr=codePolice._cp_test_pcm(i), size=codePolice._cp_test_pcm_size(i)*2;
          const bytes=codePolice.HEAPU8.slice(ptr,ptr+size);
          const hash=await crypto.subtle.digest('SHA-256',bytes);
          hashes[name]=Array.from(new Uint8Array(hash),n=>n.toString(16).padStart(2,'0')).join('');
        }
        return hashes;
      });
      assert.deepEqual(pcm,reference.pcm);
      let worst=0, commands=0, captured=0;
      const capturesAt = new Set(reference.frames.filter(f=>f.state).map(f=>f.tick));
      // Chunked calls keep the browser responsive and every wait bounded.
      for(let offset=0;offset<reference.frames.length;offset+=100){
        const batch=reference.frames.slice(offset,offset+100);
        const actual=await boundedEvaluate(page,async frames=>{
          const result=[];
          for(const f of frames){
            const hash=codePolice.UTF8ToString(codePolice._cp_test_frame(f.tick));
            const row={hash,events:JSON.parse(codePolice.UTF8ToString(codePolice._cp_test_events()))};
            if(f.inspect) row.state=JSON.parse(codePolice.UTF8ToString(codePolice._cp_test_state()));
            if(f.capture){
              const c=document.getElementById('canvas');
              const bytes=c.getContext('2d').getImageData(0,0,520,300).data;
              const digest=await crypto.subtle.digest('SHA-256',bytes);
              row.rgba=Array.from(new Uint8Array(digest),n=>n.toString(16).padStart(2,'0')).join('');
            }
            result.push(row);
          }
          return result;
        },batch.map(f=>({tick:f.tick,inspect:Boolean(f.state||f.events.length),capture:capturesAt.has(f.tick)})));
        for(let i=0;i<batch.length;++i){
          const expected=batch[i],got=actual[i];
          assert.equal(got.hash,expected.hash,`${engine} pixel hash tick ${expected.tick}`);
          assert.deepEqual(got.events,expected.events,`audio events at ${expected.tick}`);commands+=expected.events.length;
          if(expected.state){
            assert.equal(got.rgba,expected.rgba_sha256,`${engine} actual SDL canvas tick ${expected.tick}`); ++captured;
            assert.equal(got.state.values.length,expected.state.length);
            for(let n=0;n<expected.state.length;++n){
              const relative=Math.abs(got.state.values[n]-expected.state[n])/Math.max(1,Math.abs(expected.state[n]));
              assert(Number.isFinite(relative)&&relative<=1e-10,`state ${expected.tick} index ${n}: ${relative}`);
              worst=Math.max(worst,relative);
            }
          }
        }
        if(offset%2000===0)console.log(`${engine}: ${Math.min(offset+100,reference.frames.length)}/11101 reference frames`);
      }
      result.comparison={frames:reference.frames.length,captured,commands,pcm:10,maxRelativeStateError:worst,timezone:'Pacific/Honolulu'};
      // An interesting frame obtained through sequential playback, never a fake seek.
      await boundedEvaluate(page,()=>{codePolice._cp_test_reset();for(let t=0;t<=4500;t+=2)codePolice._cp_test_frame(t);});
      result.presentation=[await capturePresentation(page,engine,'cinema')];
      result.geometries.push(await geometry(page));
      await page.click('#toggle-decor');
      let g=await geometry(page); assert.equal(g.width,1040);assert.equal(g.height,600);assert.equal(g.interpolated,0);
      result.geometries.push(g);
      await page.click('#toggle-fullscreen');
      await page.waitForFunction(()=>!!document.fullscreenElement);
      await page.waitForTimeout(200);
      g=await geometry(page); assert(g.fullscreen);assert.equal(g.interpolated,0);
      assert(Math.abs(g.width/g.height-520/300)<.003);result.geometries.push(g);
      result.presentation.push(await capturePresentation(page,engine,'fullscreen'));
      await page.evaluate(()=>document.exitFullscreen());
      await page.waitForFunction(()=>!document.fullscreenElement);
      await page.waitForFunction(()=>document.getElementById('screen').getBoundingClientRect().width===1040);
      assert.equal((await geometry(page)).width,1040);
      await page.click('#back');
      await page.selectOption('#resolution','1280x720');
      assert.match(await page.locator('#size-note').textContent(),/1248 × 720/);
      await page.click('#start');
      g=await geometry(page);assert.equal(g.width,1248);assert.equal(g.height,720);assert.equal(g.interpolated,0);result.geometries.push(g);
      // Exercise the visibility handler with controlled hidden/visible transitions.
      await page.evaluate(()=>{Object.defineProperty(document,'hidden',{configurable:true,get:()=>true});document.dispatchEvent(new Event('visibilitychange'));});
      const paused=await status(page);await page.waitForTimeout(350);assert.deepEqual(await status(page),paused);
      await page.evaluate(()=>{Object.defineProperty(document,'hidden',{configurable:true,get:()=>false});document.dispatchEvent(new Event('visibilitychange'));});
      await page.waitForTimeout(80);assert((await status(page)).tick-paused.tick<25);
      result.checks.push('Optional cinema; 1280x720 -> 1248x720; fullscreen/restoration; simulated hidden-tab pause/resume');
      assert.deepEqual(external,[]);assert.deepEqual(errors,[]);assert.deepEqual(bad,[]);
      await context.close();

      const mobile=await browser.newContext({viewport:{width:390,height:844},deviceScaleFactor:2,hasTouch:true,timezoneId:'Asia/Tokyo'});
      mobile.setDefaultTimeout(15000);
      const touch=await mobile.newPage();await ready(touch);
      await touch.tap('#start');await touch.waitForTimeout(300);assert.equal((await status(touch)).starts,1);
      g=await geometry(touch);assert.equal(g.dpr,2);assert.equal(g.interpolated,0);assert.deepEqual(g.source,[520,300]);
      assert(g.composition.width<=390);result.geometries.push(g);
      await touch.screenshot({path:path.join(captures,`${engine}-touch-2x.png`)});
      await touch.tap('#close');await touch.waitForFunction(()=>document.documentElement.dataset.state==='ready');
      await touch.selectOption('#resolution','custom');await touch.fill('#width','800');await touch.fill('#height','600');
      assert.match(await touch.locator('#size-note').textContent(),/800 × 462/);
      await touch.fill('#width','0');await touch.tap('#start');assert.equal((await status(touch)).running,0);
      result.checks.push('Touch Start once; original close returns home; HiDPI 2x; responsive cinema; custom size validation');
      await mobile.close();
      // Text-mode uploads can change CRLF to LF. Both must restore the same
      // original bytes in MEMFS; stale responses must be revalidated/versioned.
      const script=await readFile(path.join(site,'assets/scene.txt'));
      const lf=Buffer.from(script.toString('utf8').replace(/\r\n/g,'\n'));
      const newlinePage=await browser.newPage();
      let scriptVersion, scriptRequests=0;
      await newlinePage.route(u=>u.pathname.endsWith('/assets/scene.txt'),route=>{
        scriptVersion=new URL(route.request().url()).searchParams.get('v');
        if(++scriptRequests===1) return route.fulfill({status:429,headers:{'Retry-After':'0'},body:'Server busy'});
        return route.fulfill({status:200,body:lf});
      });
      await ready(newlinePage);
      const restored=await newlinePage.evaluate(()=>Array.from(codePolice.FS.readFile('/assets/scene.txt')));
      assert.deepEqual(Buffer.from(restored),script);
      assert.equal(scriptVersion,hash(script));
      assert.equal(scriptRequests,2);
      await newlinePage.click('#start');
      await newlinePage.waitForFunction(()=>JSON.parse(codePolice.UTF8ToString(codePolice._cp_status())).frames>=20);
      await newlinePage.close();
      result.loading={originalScriptBytes:script.length,lfBytes:lf.length,restoredBytesExact:true,resourceVersion:scriptVersion,rateLimitRecovered:true};
      // Missing/corrupt external media and missing WASM must name the resource.
      for(const fault of ['missing','corrupt','wasm','script-truncated','script-corrupt','script-busy']){
        const broken=await browser.newPage();
        const resource=fault==='wasm'?'code-police.wasm':fault.startsWith('script-')?'assets/scene.txt':'assets/Images/flare1.jpg.png';
        await broken.route(u=>u.pathname.endsWith('/'+resource),async route=>{
          if(fault==='script-busy'){
            await route.fulfill({status:429,headers:{'Retry-After':'0'},body:'Server busy'});
          } else if(fault==='script-truncated'){
            await route.fulfill({status:200,body:lf.subarray(0,1000)});
          } else if(fault==='script-corrupt'){
            const bytes=Buffer.from(lf);bytes[100]^=1;
            await route.fulfill({status:200,body:bytes});
          } else if(fault==='corrupt'){
            const bytes=Buffer.from(await readFile(path.join(site,resource)));bytes[100]^=255;
            await route.fulfill({status:200,body:bytes});
          } else await route.fulfill({status:404,body:'Missing test resource'});
        });
        await broken.goto(url);
        await broken.waitForFunction(()=>document.documentElement.dataset.state==='error',null,{timeout:30000});
        assert((await broken.locator('#status').innerText()).includes(resource), await broken.locator('#status').innerText());
        if(fault==='script-corrupt') assert.match(await broken.locator('#status').innerText(),/checksum mismatch/);
        if(fault==='script-truncated') assert.match(await broken.locator('#status').innerText(),/expected 44195 bytes, received 1000/);
        assert(await broken.locator('#start').isDisabled());await broken.close();
      }
      result.checks.push('Missing/corrupt media and missing WASM errors name resource; local-only HTTP requests; all original/package hashes');
      result.checks.push('LF-uploaded script restored byte-exactly; hash-versioned resource URL; truncated and altered scripts rejected; English launch page');
      report.browsers.push(result);
      await writeFile(path.join(output,'validation.json'),JSON.stringify(report,null,2)+'\n');
    } finally {await browser.close();}
  }
  console.log(JSON.stringify(report,null,2));
} finally {server.close();await rm(scratch,{recursive:true,force:true});}
