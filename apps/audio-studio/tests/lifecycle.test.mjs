import test from 'node:test';
import assert from 'node:assert/strict';
import {AudioEngine, createGraph, MODES, peakOf} from '../audio-engine.js';

// Controlled clock/AudioNode doubles test ownership and ordering only, not DSP.
function context() {
  const param = () => ({value:0, events:[], setValueAtTime(v,t){this.value=v;this.events.push(['set',v,t]);},
    linearRampToValueAtTime(v,t){this.events.push(['ramp',v,t]);}, cancelScheduledValues(t){this.events.push(['cancel',t]);}});
  const ctx = {state:'running', currentTime:0, nodes:[], destination:{},
    resume:async () => {ctx.state='running';}, close:async () => {ctx.state='closed';}};
  const node = () => {
    const n = {connections:[], disconnected:false, connect(target){this.connections.push(target);}, disconnect(){this.disconnected=true;}};
    ctx.nodes.push(n); return n;
  };
  ctx.createOscillator = () => Object.assign(node(), {frequency:param(), start(t){this.started=t;}, stop(t){this.stopped=t ?? ctx.currentTime;}});
  ctx.createGain = () => Object.assign(node(), {gain:param()});
  ctx.createAnalyser = () => Object.assign(node(), {fftSize:2048});
  ctx.createDynamicsCompressor = () => Object.assign(node(), Object.fromEntries(['threshold','knee','ratio','attack','release'].map(k=>[k,param()])));
  return ctx;
}
const engineFor = ctx => new AudioEngine({contextFactory:() => ctx});

test('engine imports without DOM; exact mode calculations', () => {
  assert.equal(MODES.M2,136.1/Math.sqrt(2)); assert.equal(MODES.M3,136.1/Math.sqrt(3));
});
test('production graph uses v1.1 compressor and AudioParam AM connection', () => {
  const g=createGraph(context());
  assert.equal(g.compressor.knee.value,6); assert.equal(g.compressor.release.value,.1);
  assert.deepEqual(g.modGain.connections,[g.carrierGain.gain]);
});
test('double start is ignored and busy spans the audio-time fade', async () => {
  const ctx=context(), e=engineFor(ctx), pending=e.start();
  assert.equal(e.isBusy,true); assert.equal(await e.start(),false);
  assert.equal(ctx.nodes.length,7); ctx.currentTime=.81;
  assert.equal(await pending,true); assert.equal(e.state,'playing'); e.hardReset();
});
test('reset invalidates unresolved resume', async () => {
  const ctx=context(); ctx.state='suspended'; let release;
  ctx.resume=()=>new Promise(resolve=>{release=()=>{ctx.state='running';resolve();};});
  const e=engineFor(ctx), pending=e.start(); e.hardReset(); release();
  assert.equal(await pending,false); assert.equal(ctx.nodes.length,0); assert.equal(e.state,'idle');
});
test('stop can cancel a pending resume', async () => {
  const ctx=context(); ctx.state='suspended'; let release;
  ctx.resume=()=>new Promise(resolve=>{release=()=>{ctx.state='running';resolve();};});
  const e=engineFor(ctx), pending=e.start(); assert.equal(await e.stop(),true); release();
  assert.equal(await pending,false); assert.equal(e.isPlaying,false);
});
test('old stop continuation cannot reset a newly started graph', async () => {
  const ctx=context(), e=engineFor(ctx);
  const first=e.start(); ctx.currentTime=.81; await first;
  const stopping=e.stop(); e.hardReset();
  const second=e.start(); const newGraph=e.graph;
  assert.equal(await stopping,false); assert.equal(e.graph,newGraph);
  ctx.currentTime=1.7; assert.equal(await second,true); e.hardReset();
});
test('stop interrupts fade using current envelope; zero is scheduled on audio clock', async () => {
  const ctx=context(), e=engineFor(ctx), started=e.start();
  ctx.currentTime=.4; const g=e.graph, stopped=e.stop();
  assert.equal(await started,false);
  assert.deepEqual(g.outputGain.gain.events.slice(-2),[['set',.225,.4],['ramp',0,.9]]);
  assert.equal(g.carrier.stopped,.9); assert.equal(e.state,'stopping');
  ctx.currentTime=.91; assert.equal(await stopped,true); assert.equal(e.state,'idle');
});
test('mode lock spans 0.3 seconds; stop takes priority', async () => {
  const ctx=context(), e=engineFor(ctx), start=e.start(); ctx.currentTime=.81; await start;
  const changing=e.setMode('M2'); assert.equal(e.isBusy,true);
  assert.equal(await e.setMode('M3'),false);
  assert.deepEqual(e.graph.modOsc.frequency.events.at(-1),['ramp',MODES.M2,1.11]);
  const stop=e.stop(); assert.equal(await changing,false);
  ctx.currentTime=1.32; assert.equal(await stop,true);
});
test('resume error rejects and clears state', async () => {
  const ctx=context(); ctx.state='suspended'; ctx.resume=async()=>{throw Error('denied');};
  const e=engineFor(ctx); await assert.rejects(e.start(),/denied/); assert.equal(e.isBusy,false);
});
test('invalid mode rejects before creating context', async () => {
  let called=false; const e=new AudioEngine({contextFactory:()=>{called=true;}});
  await assert.rejects(e.start('invalid'),RangeError); assert.equal(called,false);
});
test('partial graph build cleans allocated nodes', () => {
  const ctx=context(); ctx.createAnalyser=()=>{throw Error('allocation');};
  assert.throws(()=>createGraph(ctx),/allocation/); assert.ok(ctx.nodes.every(n=>n.disconnected));
});
test('disposal closes old context and leaves no graph', async () => {
  const ctx=context(), e=engineFor(ctx), start=e.start(); ctx.currentTime=.81; await start;
  await e.dispose(); assert.equal(ctx.state,'closed'); assert.equal(e.ctx,null); assert.equal(e.graph,null);
});
test('peak thresholds have unambiguous boundaries', () => {
  assert.equal(peakOf([.9]).state,'WARN'); assert.equal(peakOf([-.98]).state,'LIMIT');
  assert.equal(peakOf([NaN]).state,'ERROR'); assert.equal(peakOf([.899]).state,'OK');
});
