/* HRONOAZA Audio Module, specification 1.1. No audio starts on import. */
const root = globalThis;
  'use strict';
  const MODES = Object.freeze({M1: 136.1, M2: 136.1 / Math.sqrt(2), M3: 136.1 / Math.sqrt(3)});
  const SETTINGS = Object.freeze({base: 0.45, depth: 0.1, start: 0.8, stop: 0.5,
    mode: 0.3, threshold: -2, knee: 6, ratio: 12, attack: 0.003, release: 0.1});
  function validateMode(mode) {
    if (!Object.hasOwn(MODES, mode)) throw new RangeError('Unknown audio mode: ' + mode);
    return mode;
  }
  function disposeGraph(graph) {
    if (!graph) return;
    for (const node of graph.nodes) {
      try { if (typeof node.stop === 'function') node.stop(); } catch (_) {}
      try { node.disconnect(); } catch (_) {}
    }
  }
  function createCompressor(ctx) {
    const node = ctx.createDynamicsCompressor();
    for (const key of ['threshold', 'knee', 'ratio', 'attack', 'release']) {
      node[key].setValueAtTime(SETTINGS[key], ctx.currentTime);
    }
    return node;
  }
  // The same graph factory serves live playback and OfflineAudioContext tests.
  // phase is a test seam in radians; live playback uses zero phase.
  function createGraph(ctx, {mode = 'M1', destination = ctx.destination, phase = 0} = {}) {
    validateMode(mode);
    if (!Number.isFinite(phase)) throw new TypeError('phase must be finite');
    const g = {ctx, nodes: [], started: false};
    const own = node => (g.nodes.push(node), node);
    try {
      g.carrier = own(ctx.createOscillator());
      g.modOsc = own(ctx.createOscillator());
      g.carrierGain = own(ctx.createGain());
      g.modGain = own(ctx.createGain());
      g.outputGain = own(ctx.createGain());
      g.compressor = own(createCompressor(ctx));
      g.analyser = own(ctx.createAnalyser());
      const now = ctx.currentTime;
      g.carrier.type = 'sine';
      g.carrier.frequency.setValueAtTime(136.1, now);
      g.modOsc.type = 'sine';
      if (phase !== 0) {
        g.modOsc.setPeriodicWave(ctx.createPeriodicWave(
          new Float32Array([0, Math.sin(phase)]),
          new Float32Array([0, Math.cos(phase)]), {disableNormalization: true}));
      }
      g.modOsc.frequency.setValueAtTime(MODES[mode], now);
      g.carrierGain.gain.setValueAtTime(1, now);
      g.modGain.gain.setValueAtTime(SETTINGS.depth, now);
      g.outputGain.gain.setValueAtTime(0, now);
      g.analyser.fftSize = 512;
      g.carrier.connect(g.carrierGain);
      g.modOsc.connect(g.modGain);
      g.modGain.connect(g.carrierGain.gain);
      g.carrierGain.connect(g.outputGain);
      g.outputGain.connect(g.compressor);
      g.compressor.connect(g.analyser);
      g.analyser.connect(destination);
      return g;
    } catch (error) { disposeGraph(g); throw error; }
  }
  function startGraph(g, now = g.ctx.currentTime) {
    if (g.started) throw new Error('Graph already started');
    g.started = true;
    g.envelope = {from: 0, to: SETTINGS.base, start: now, end: now + SETTINGS.start};
    g.outputGain.gain.setValueAtTime(0, now);
    g.outputGain.gain.linearRampToValueAtTime(SETTINGS.base, g.envelope.end);
    g.carrier.start(now);
    g.modOsc.start(now);
  }
  function envelopeAt(g, time) {
    const r = g.envelope;
    const t = Math.max(0, Math.min(1, (time - r.start) / (r.end - r.start)));
    return r.from + t * (r.to - r.from);
  }
  function scheduleStop(g, now = g.ctx.currentTime) {
    const p = g.outputGain.gain;
    const value = envelopeAt(g, now);
    // Explicitly anchor the fade at the intended current envelope value.
    // cancelAndHoldAtTime() does not provide consistent anchoring for the
    // subsequent ramp across tested OfflineAudioContext implementations.
    p.cancelScheduledValues(now);
    p.setValueAtTime(value, now);
    const end = now + SETTINGS.stop;
    p.linearRampToValueAtTime(0, end);
    g.envelope = {from: value, to: 0, start: now, end};
    g.carrier.stop(end);
    g.modOsc.stop(end);
    return end;
  }
  function scheduleMode(g, mode, now = g.ctx.currentTime) {
    validateMode(mode);
    const p = g.modOsc.frequency;
    // Public lifecycle serializes mode changes, so no earlier mode ramp is active.
    const value = p.value;
    p.cancelScheduledValues(now);
    p.setValueAtTime(value, now);
    p.linearRampToValueAtTime(MODES[mode], now + SETTINGS.mode);
    return now + SETTINGS.mode;
  }
  class AudioEngine {
    constructor({contextFactory = () => {
      const AudioCtx = root.AudioContext || root.webkitAudioContext;
      if (!AudioCtx) throw new Error('Web Audio API is unavailable');
      return new AudioCtx();
    }} = {}) {
      this.contextFactory = contextFactory;
      this.ctx = null;
      this.graph = null;
      this.state = 'idle';
      this.currentMode = 'M1';
      this.epoch = 0;
      this.waiters = new Set();
    }
    get isPlaying() { return !!this.graph; }
    get isBusy() { return !['idle', 'playing'].includes(this.state); }
    get analyser() { return this.graph?.analyser || null; }
    _cancelWaits() { for (const cancel of [...this.waiters]) cancel(); }
    _waitUntil(ctx, end, token) {
      return new Promise(resolve => {
        let timer;
        const finish = value => {
          clearTimeout(timer); this.waiters.delete(cancel); resolve(value);
        };
        const cancel = () => finish(false);
        const poll = () => {
          if (token !== this.epoch || ctx.state === 'closed') return finish(false);
          if (ctx.currentTime >= end) return finish(true);
          timer = setTimeout(poll, 16);
        };
        this.waiters.add(cancel); poll();
      });
    }
    async start(mode = this.currentMode) {
      validateMode(mode);
      if (this.state !== 'idle') return false;
      const token = ++this.epoch;
      this.state = 'starting';
      try {
        if (!this.ctx || this.ctx.state === 'closed') this.ctx = this.contextFactory();
        const ctx = this.ctx;
        if (ctx.state !== 'running') await ctx.resume();
        if (token !== this.epoch) return false;
        if (ctx.state !== 'running') throw new Error('AudioContext is not running');
        this.graph = createGraph(ctx, {mode});
        startGraph(this.graph);
        this.currentMode = mode;
        if (!await this._waitUntil(ctx, this.graph.envelope.end, token)) {
          if (token === this.epoch) this.hardReset();
          return false;
        }
        this.state = 'playing';
        return true;
      } catch (error) {
        if (token !== this.epoch) return false;
        this.hardReset(); throw error;
      }
    }
    async setMode(mode) {
      validateMode(mode);
      if (this.state !== 'playing') return false;
      if (mode === this.currentMode) return true;
      const token = ++this.epoch;
      this.state = 'changing';
      try {
        const end = scheduleMode(this.graph, mode);
        if (!await this._waitUntil(this.ctx, end, token)) {
          if (token === this.epoch) this.hardReset();
          return false;
        }
        this.currentMode = mode;
        this.state = 'playing';
        return true;
      } catch (error) {
        if (token !== this.epoch) return false;
        this.hardReset(); throw error;
      }
    }
    async stop() {
      if (this.state === 'idle' || this.state === 'stopping') return false;
      // Stop may interrupt start or a mode ramp; old continuations become inert.
      if (!this.graph) { this.hardReset(); return true; }
      const token = ++this.epoch;
      this._cancelWaits();
      this.state = 'stopping';
      try {
        const end = scheduleStop(this.graph);
        const completed = await this._waitUntil(this.ctx, end, token);
        if (token !== this.epoch) return false;
        this.hardReset();
        return completed;
      } catch (error) {
        if (token !== this.epoch) return false;
        this.hardReset(); throw error;
      }
    }
    hardReset() {
      ++this.epoch;
      this._cancelWaits();
      const graph = this.graph;
      this.graph = null;
      this.state = 'idle';
      disposeGraph(graph);
    }
    async dispose() {
      this.hardReset();
      const ctx = this.ctx;
      this.ctx = null;
      if (ctx && ctx.state !== 'closed') await ctx.close();
    }
  }
  function peakOf(samples) {
    let peak = 0;
    for (const value of samples) {
      if (!Number.isFinite(value)) return {peak: NaN, state: 'ERROR'};
      peak = Math.max(peak, Math.abs(value));
    }
    return {peak, state: peak >= 0.98 ? 'LIMIT' : peak >= 0.9 ? 'WARN' : 'OK'};
  }

export {MODES, SETTINGS, createGraph, startGraph, scheduleMode, scheduleStop, disposeGraph, createCompressor, peakOf, AudioEngine};
