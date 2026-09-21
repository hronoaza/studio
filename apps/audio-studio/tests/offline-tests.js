import {MODES, createGraph, startGraph, scheduleStop, scheduleMode,
  createCompressor, disposeGraph, AudioEngine, peakOf} from '../audio-engine.js';

function assert(condition, message) { if (!condition) throw new Error(message); }
function stats(data, from = 0, to = data.length) {
  let sum = 0, sum2 = 0, peak = 0;
  for (let i = from; i < to; i++) {
    const x = data[i]; assert(Number.isFinite(x), 'Non-finite audio sample at ' + i);
    peak = Math.max(peak, Math.abs(x)); sum += x; sum2 += x*x;
  }
  return {peak, mean: sum/(to-from), rms: Math.sqrt(sum2/(to-from))};
}
async function render({rate = 48000, mode = 'M1', phase = 0, stop = null, change = null} = {}) {
  const ctx = new OfflineAudioContext(2, rate * 12, rate);
  // Channel 0 taps outputGain before compression, channel 1 uses the full graph.
  const merger = ctx.createChannelMerger(2); merger.connect(ctx.destination);
  const post = ctx.createGain(); post.connect(merger, 0, 1);
  const graph = createGraph(ctx, {mode, phase, destination: post});
  graph.outputGain.connect(merger, 0, 0);
  startGraph(graph, 0);

  // Runtime automation must be issued when audio time actually reaches the
  // intervention point. Scheduling cancel/hold operations in advance at t=0
  // does not model the live AudioContext semantics used by the application.
  const interventions = [];
  if (change) {
    interventions.push((async () => {
      await ctx.suspend(change.time);
      scheduleMode(graph, change.mode, ctx.currentTime);
      await ctx.resume();
    })());
  }
  if (stop !== null) {
    interventions.push((async () => {
      await ctx.suspend(stop);
      scheduleStop(graph, ctx.currentTime);
      await ctx.resume();
    })());
  }

  const rendering = ctx.startRendering();
  await Promise.all(interventions);
  const buffer = await rendering;
  disposeGraph(graph);
  return {pre: buffer.getChannelData(0), post: buffer.getChannelData(1), rate};
}

export async function runTests() {
  const results = [];
  const test = async (name, fn) => {
    try { results.push({name, status: 'PASS', measurements: await fn()}); }
    catch (error) { results.push({name, status: 'FAIL', error: String(error.stack || error)}); }
  };
  for (const rate of [44100, 48000]) {
    for (const mode of Object.keys(MODES)) {
      await test(`Shared graph: ${mode}, ${rate} Hz`, async () => {
        const {pre, post} = await render({rate, mode});
        let maxError = 0;
        for (let i = rate; i < pre.length; i++) {
          const t = i/rate;
          const expected = .45 * (1 + .1*Math.sin(2*Math.PI*MODES[mode]*t)) * Math.sin(2*Math.PI*136.1*t);
          maxError = Math.max(maxError, Math.abs(pre[i] - expected));
        }
        const raw = stats(pre, rate), processed = stats(post, rate);
        assert(maxError < 0.0001, 'Waveform deviates from analytical AM: ' + maxError);
        assert(raw.peak <= .495001, 'Pre-compressor AM bound exceeded');
        assert(processed.peak < 1, 'Nominal sample peak reaches full scale');
        assert(processed.rms > .1, 'Silent or disconnected processed output');
        return {maxError, pre: raw, post: processed};
      });
    }
    for (const phase of [0, Math.PI/2, Math.PI]) {
      await test(`M1 DC phase ${phase.toFixed(6)}, ${rate} Hz`, async () => {
        const {pre, post} = await render({rate, phase});
        const raw = stats(pre, rate, 11*rate), processed = stats(post, rate, 11*rate);
        const expectedDC = .0225*Math.cos(phase);
        assert(Math.abs(raw.mean - expectedDC) < .00002, 'Incorrect phase-dependent DC');
        return {expectedDC, preDC: raw.mean, postDC: processed.mean};
      });
    }
    await test(`Fade-in and exact fade-out, ${rate} Hz`, async () => {
      const {pre, post} = await render({rate, stop: 2});
      let maxError = 0, worst = null;
      for (let i = 0; i < 2.6*rate; i++) {
        const t = i/rate;
        const gain = t < .8 ? .45*t/.8 : t < 2 ? .45 : t < 2.5 ? .45*(2.5-t)/.5 : 0;
        const expected = gain*(1+.1*Math.sin(2*Math.PI*136.1*t))*Math.sin(2*Math.PI*136.1*t);
        const error = Math.abs(pre[i]-expected);
        if (error > maxError) {
          maxError = error;
          worst = {sample:i, time:t, observed:pre[i], expected, error};
        }
      }
      assert(maxError < .0001, 'Ramp mismatch: ' + JSON.stringify({maxError, worst,
        probes:[1.99,2,2.1,2.25,2.49,2.5].map(t=>({t, sample:pre[Math.round(t*rate)]}))}));
      assert(stats(pre, Math.ceil(2.5*rate)).peak < 1e-7, 'Fade-out did not reach zero');
      assert(stats(post, 3*rate).peak < 1e-7, 'Output did not settle to silence');
      return {maxError};
    });
    await test(`Mode ramp phase integration, ${rate} Hz`, async () => {
      const {pre} = await render({rate, change: {mode: 'M2', time: 2}});
      let maxError = 0, worst = null;
      const f0 = MODES.M1, f1 = MODES.M2;
      for (let i = rate; i < pre.length; i++) {
        const t = i/rate, u = t-2;
        const cycles = u < 0 ? f0*t : u <= .3 ? 2*f0 + f0*u + .5*(f1-f0)*u*u/.3 : 2*f0 + .3*(f0+f1)/2 + f1*(u-.3);
        const expected = .45*(1+.1*Math.sin(2*Math.PI*cycles))*Math.sin(2*Math.PI*136.1*t);
        const error = Math.abs(pre[i]-expected);
        if (error > maxError) {
          maxError = error;
          worst = {sample:i, time:t, observed:pre[i], expected, error};
        }
      }
      assert(maxError < .001, 'Mode transition differs from linear 0.3 s ramp: '+JSON.stringify({maxError, worst,
        probes:[1.99,2,2.05,2.15,2.3,2.31].map(t=>({t, sample:pre[Math.round(t*rate)]}))}));
      return {maxError};
    });
    await test(`Primitive GainNode automation, ${rate} Hz`, async () => {
      const ctx = new OfflineAudioContext(1, rate*3, rate);
      const source = ctx.createConstantSource(), gain = ctx.createGain();
      source.offset.setValueAtTime(1, 0);
      gain.gain.setValueAtTime(0, 0);
      gain.gain.linearRampToValueAtTime(.45, .8);
      source.connect(gain).connect(ctx.destination); source.start(0); source.stop(2.6);
      const intervention = (async () => {
        await ctx.suspend(2);
        const now = ctx.currentTime;
        gain.gain.cancelScheduledValues(now);
        gain.gain.setValueAtTime(.45, now);
        gain.gain.linearRampToValueAtTime(0, now + .5);
        await ctx.resume();
      })();
      const rendering = ctx.startRendering();
      await intervention;
      const rendered = await rendering, data = rendered.getChannelData(0);
      let maxError = 0, worst = null;
      for (let i=0; i<2.6*rate; i++) {
        const t=i/rate;
        const expected=t<.8?.45*t/.8:t<2?.45:t<2.5?.45*(2.5-t)/.5:0;
        const error=Math.abs(data[i]-expected);
        if(error>maxError){maxError=error;worst={sample:i,time:t,observed:data[i],expected,error};}
      }
      assert(maxError < .00002, 'Primitive gain automation mismatch: '+JSON.stringify({maxError,worst}));
      return {maxError,worst};
    });
    await test(`Primitive Oscillator frequency ramp, ${rate} Hz`, async () => {
      const ctx = new OfflineAudioContext(1, rate*3, rate);
      const osc = ctx.createOscillator();
      const f0=MODES.M1, f1=MODES.M2;
      osc.type='sine'; osc.frequency.setValueAtTime(f0,0);
      osc.connect(ctx.destination); osc.start(0); osc.stop(2.9);
      const intervention = (async () => {
        await ctx.suspend(2);
        const now = ctx.currentTime;
        osc.frequency.cancelScheduledValues(now);
        osc.frequency.setValueAtTime(f0, now);
        osc.frequency.linearRampToValueAtTime(f1, now + .3);
        await ctx.resume();
      })();
      const rendering=ctx.startRendering();
      await intervention;
      const rendered=await rendering, data=rendered.getChannelData(0);
      let maxError=0,worst=null;
      for(let i=rate;i<2.9*rate;i++){
        const t=i/rate,u=t-2;
        const cycles=u<0?f0*t:u<=.3?2*f0+f0*u+.5*(f1-f0)*u*u/.3:2*f0+.3*(f0+f1)/2+f1*(u-.3);
        const expected=Math.sin(2*Math.PI*cycles);
        const error=Math.abs(data[i]-expected);
        if(error>maxError){maxError=error;worst={sample:i,time:t,observed:data[i],expected,error};}
      }
      assert(maxError < .002, 'Primitive oscillator ramp mismatch: '+JSON.stringify({maxError,worst}));
      return {maxError,worst};
    });
    await test(`Compressor transient measurement, ${rate} Hz`, async () => {
      const ctx = new OfflineAudioContext(2, rate*2, rate);
      const merger = ctx.createChannelMerger(2); merger.connect(ctx.destination);
      const source = ctx.createBufferSource(), buffer = ctx.createBuffer(1, rate*2, rate);
      const x = buffer.getChannelData(0);
      x[Math.round(.1*rate)] = 4;
      for (let i = .3*rate; i < 1.3*rate; i++) x[i] = 4*Math.sin(2*Math.PI*1000*i/rate);
      source.buffer = buffer;
      const compressor = createCompressor(ctx);
      source.connect(merger, 0, 0); source.connect(compressor); compressor.connect(merger, 0, 1);
      source.start(); const rendered = await ctx.startRendering();
      const dry = rendered.getChannelData(0), wet = rendered.getChannelData(1);
      const steadyDry = stats(dry, .8*rate, 1.2*rate), steadyWet = stats(wet, .8*rate, 1.2*rate);
      assert(steadyWet.rms > 0 && steadyWet.rms < steadyDry.rms*.8, 'No sustained compression');
      const pulse = stats(wet, .09*rate, .2*rate);
      assert(pulse.peak > 0, 'Pulse missing from processed signal');
      return {inputPeak: stats(dry).peak, outputPeak: stats(wet).peak,
        impulseOutputPeak: pulse.peak, steadyRMSRatio: steadyWet.rms/steadyDry.rms,
        note: 'Diagnostic sample peaks; no intersample ceiling assertion'};
    });
  }
  await test('Peak-meter thresholds and invalid samples', () => {
    for (const [sample, expected] of [[.899,'OK'],[.9,'WARN'],[.979,'WARN'],[.98,'LIMIT'],[-1.1,'LIMIT'],[NaN,'ERROR']]) {
      assert(peakOf([sample]).state === expected, 'Incorrect threshold: '+sample);
    }
  });
  await test('Reset cancels pending resume; stale continuation cannot build graph', async () => {
    let release, created = 0;
    const ctx = {state:'suspended', resume:() => new Promise(resolve => {release = () => {ctx.state='running'; resolve();};}),
      createOscillator:() => {created++; throw new Error('Unexpected graph');}};
    const engine = new AudioEngine({contextFactory:() => ctx});
    const pending = engine.start(); engine.hardReset(); release();
    assert(await pending === false, 'Cancelled start completed');
    assert(created === 0 && engine.state === 'idle', 'Stale start mutated state');
  });
  await test('Resume rejection clears busy state and is observable', async () => {
    const engine = new AudioEngine({contextFactory:() => ({state:'suspended', resume:async () => {throw new Error('denied');}})});
    let rejected = false;
    try { await engine.start(); } catch (_) { rejected = true; }
    assert(rejected && !engine.isBusy && !engine.isPlaying, 'Failure was swallowed or left busy');
  });
  await test('Unknown mode rejects before context creation', async () => {
    let created = false, rejected = false;
    const engine = new AudioEngine({contextFactory:() => {created = true;}});
    try { await engine.start('bad'); } catch (_) { rejected = true; }
    assert(rejected && !created && engine.currentMode === 'M1', 'Invalid mode corrupted state');
  });
  return {suite:'HRONOAZA Audio 1.1', userAgent:navigator.userAgent, passed:results.filter(x=>x.status==='PASS').length,
    failed:results.filter(x=>x.status==='FAIL').length, results,
    scope:'Offline sample-domain DSP and selected async regressions; no hearing-safety or true-peak certification'};
}
