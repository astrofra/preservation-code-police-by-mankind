// Real elapsed playback, separate from the imposed reference timeline.
// Usage: node wasm/live-smoke.mjs http://localhost:8000/
import assert from 'node:assert/strict';
import {writeFile} from 'node:fs/promises';
import {chromium} from 'playwright';
const browser=await chromium.launch({channel:'chrome',headless:true,timeout:15000});
try {
  const page=await browser.newPage({viewport:{width:1280,height:900}});
  page.setDefaultTimeout(15000);
  const errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.goto(process.argv[2]||'http://localhost:8000/');
  await page.waitForFunction(()=>document.documentElement.dataset.state==='ready');
  await page.click('#start');
  await page.evaluate(()=>{
    window.audioProof={peak:0,callbacks:0};
    const node=codePolice.SDL2.audio.scriptProcessorNode, original=node.onaudioprocess;
    node.onaudioprocess=function(event){
      original.call(this,event); ++audioProof.callbacks;
      for(const n of event.outputBuffer.getChannelData(0))audioProof.peak=Math.max(audioProof.peak,Math.abs(n));
    };
  });
  const start=Date.now(), samples=[];
  let previous=-1,loop=false;
  while(Date.now()-start<235000){
    await page.waitForTimeout(1000);
    const state=await page.evaluate(()=>JSON.parse(codePolice.UTF8ToString(codePolice._cp_status())));
    assert.equal(state.running,1);assert.equal(state.suspended,0);assert(state.frames>0);
    samples.push({elapsed_ms:Date.now()-start,...state});
    if(previous>21500&&state.tick<1000){loop=true;break;}
    previous=state.tick;
    if(samples.length%20===0)console.log(`${state.frames} frames; timeline ${state.tick/100}s`);
  }
  assert(loop,'Expected the 222-second script to wrap during real playback');
  const audio=await page.evaluate(()=>audioProof);assert(audio.peak>0);assert.deepEqual(errors,[]);
  await page.click('#back');
  const stopped=await page.evaluate(()=>JSON.parse(codePolice.UTF8ToString(codePolice._cp_status())));
  assert.equal(stopped.running,0);
  const report={browser:browser.version(),elapsed_ms:Date.now()-start,loop,audio,errors,stopped,samples};
  await writeFile(new URL('../evidence/wasm/live-playback.json',import.meta.url),JSON.stringify(report,null,2)+'\n');
  console.log(`Completed real script loop in ${report.elapsed_ms}ms; ${stopped.frames} frames; clean return to home.`);
} finally {await browser.close();}
