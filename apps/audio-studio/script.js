import {AudioEngine, peakOf} from './audio-engine.js';
const root = globalThis;
  function attachResponsiveOscilloscope(canvas, engine, {onPeak = () => {}} = {}) {
    const ctx = canvas.getContext('2d');
    if (!ctx) throw new Error('Canvas 2D is unavailable');
    let frame, active = true, samples = new Float32Array(512);
    let width = 0, height = 0, dpr = 0;
    function resize() {
      const w = canvas.clientWidth, h = canvas.clientHeight;
      const ratio = root.devicePixelRatio || 1;
      if (!w || !h) return false;
      const bw = Math.max(1, Math.round(w * ratio)), bh = Math.max(1, Math.round(h * ratio));
      if (canvas.width !== bw || canvas.height !== bh) {
        canvas.width = bw; canvas.height = bh;
      }
      if (width !== w || height !== h || dpr !== ratio) {
        width = w; height = h; dpr = ratio;
      }
      ctx.setTransform(bw / w, 0, 0, bh / h, 0, 0);
      return true;
    }
    const observer = new ResizeObserver(resize);
    observer.observe(canvas);
    function draw() {
      if (!active) return;
      frame = requestAnimationFrame(draw);
      // Checking DPR here also covers monitor moves with unchanged CSS dimensions.
      if (!resize()) return;
      ctx.clearRect(0, 0, width, height);
      const analyser = engine.analyser;
      if (analyser) {
        if (samples.length !== analyser.fftSize) samples = new Float32Array(analyser.fftSize);
        analyser.getFloatTimeDomainData(samples);
      } else samples.fill(0);
      onPeak(peakOf(samples));
      ctx.beginPath(); ctx.lineWidth = 1.5;
      ctx.strokeStyle = analyser ? '#00e5ff' : '#587781';
      for (let i = 0; i < samples.length; i++) {
        const x = i * width / (samples.length - 1);
        const y = (1 - samples[i]) * height / 2;
        if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
    draw();
    return () => { active = false; cancelAnimationFrame(frame); observer.disconnect(); };
  }

export {attachResponsiveOscilloscope};
const canvas = document.querySelector('#scope');
if (canvas) {
  const engine = new AudioEngine();
  const status = document.querySelector('#status');
  const meter = document.querySelector('#meter');
  const mode = document.querySelector('#mode');
  const start = document.querySelector('#start');
  const stop = document.querySelector('#stop');
  const reset = document.querySelector('#reset');
  const labels = {idle: 'Зупинено', starting: 'Плавний старт…', playing: 'Відтворення', changing: 'Зміна режиму…', stopping: 'Плавна зупинка…'};
  let errorText = '';
  function sync() {
    status.textContent = errorText || labels[engine.state];
    start.disabled = engine.state !== 'idle';
    stop.disabled = engine.state === 'idle' || engine.state === 'stopping';
    mode.disabled = engine.isBusy;
  }
  const detach = attachResponsiveOscilloscope(canvas, engine, {onPeak: ({peak, state}) => {
    meter.textContent = `${state} · ${Number.isFinite(peak) ? peak.toFixed(3) : '—'}`;
    meter.dataset.state = state;
    sync();
  }});
  async function action(fn) {
    errorText = '';
    try { const pending = fn(); sync(); await pending; }
    catch (error) { errorText = 'Помилка: ' + error.message; }
    sync();
  }
  start.addEventListener('click', () => action(() => engine.start(mode.value)));
  stop.addEventListener('click', () => action(() => engine.stop()));
  mode.addEventListener('change', () => {
    if (engine.isPlaying) action(() => engine.setMode(mode.value));
  });
  reset.addEventListener('click', () => action(() => engine.hardReset()));
  window.addEventListener('pagehide', event => {
    engine.hardReset();
    if (!event.persisted) { detach(); engine.dispose().catch(() => {}); }
  });
  sync();
}
