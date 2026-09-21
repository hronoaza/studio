import {createServer} from 'node:http';
import {readFile, writeFile} from 'node:fs/promises';
import {fileURLToPath} from 'node:url';
import {resolve, sep, extname} from 'node:path';
import {createRequire} from 'node:module';
const root = fileURLToPath(new URL('..', import.meta.url));
const types = {'.html':'text/html', '.js':'text/javascript', '.css':'text/css', '.json':'application/json', '.svg':'image/svg+xml', '.png':'image/png'};
const server = createServer(async (req, res) => {
  try {
    const url = new URL(req.url, 'http://localhost');
    const prefix = '/hronoaza-audio-studio/';
    if (!url.pathname.startsWith(prefix)) {res.writeHead(404).end(); return;}
    const relative = decodeURIComponent(url.pathname.slice(prefix.length)) || 'index.html';
    const path = resolve(root, relative);
    if (!path.startsWith(resolve(root) + sep)) {res.writeHead(403).end(); return;}
    const body = await readFile(path);
    res.writeHead(200, {'content-type':types[extname(path)] || 'text/plain'}).end(body);
  } catch (_) {res.writeHead(404).end();}
});
let browser;
try {
  const require = createRequire(import.meta.url);
  let playwright;
  try {playwright = require('playwright');}
  catch (error) {
    if (!process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES) throw error;
    playwright = require(resolve(process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES, 'playwright'));
  }
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  browser = await playwright.chromium.launch({headless:true});
  const page = await browser.newPage({deviceScaleFactor:2});
  const errors = [];
  page.on('pageerror', error => errors.push(error.message));
  const base = `http://127.0.0.1:${server.address().port}/hronoaza-audio-studio/`;
  await page.goto(base + 'offline-audio-test.html');
  await page.waitForFunction(() => !!window.testDone);
  const report = await Promise.race([
    page.evaluate(() => window.testDone),
    new Promise((_, reject) => {const timer = setTimeout(() => reject(new Error('Audio tests timed out')), 120000); timer.unref();})
  ]);
  await page.goto(base);
  await page.waitForFunction(() => {
    const c = document.querySelector('canvas'); return c.width === Math.round(c.clientWidth*devicePixelRatio);
  });
  const manifest = await page.evaluate(async () => {
    const link = document.querySelector('link[rel=manifest]');
    const url = new URL(link.href), m = await (await fetch(url)).json();
    for (const icon of m.icons) if (!(await fetch(new URL(icon.src, url))).ok) throw new Error('Missing manifest icon');
    return {start:new URL(m.start_url, url).href, scope:new URL(m.scope, url).href};
  });
  if (!manifest.start.startsWith(base) || manifest.scope !== base) errors.push('Manifest escaped repository subpath');
  await page.setViewportSize({width:390, height:844});
  await page.waitForFunction(() => {
    const c = document.querySelector('canvas');
    return c.width === Math.round(c.clientWidth*devicePixelRatio) && document.documentElement.scrollWidth <= innerWidth;
  });
  report.uiErrors = errors;
  report.failed += errors.length;
  await writeFile(new URL('../test-results.json', import.meta.url), JSON.stringify(report, null, 2) + '\n');
  console.log(JSON.stringify(report, null, 2));
  process.exitCode = report.failed ? 1 : 0;
} catch (error) {console.error(error); process.exitCode = 1;}
finally {await browser?.close(); server.close();}
