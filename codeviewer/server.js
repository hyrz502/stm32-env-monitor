#!/usr/bin/env node
/**
 * 手机端代码查看器 (CodeView) — 零依赖 Node.js 服务
 *
 * 用法:
 *   node server.js                        # 默认根目录 = 本目录的上一级, 端口 8090
 *   node server.js --root=/path/to/proj   # 指定要浏览的工程根目录
 *   node server.js --port=9000            # 指定端口
 *   环境变量: CV_ROOT, CV_PORT 同样生效
 */
'use strict';

const http = require('http');
const fs = require('fs');
const path = require('path');
const os = require('os');

/* ---------------- 参数解析 ---------------- */
function argVal(flag, env) {
  const a = process.argv.find(v => v.startsWith(flag));
  if (a) return a.slice(flag.length);
  return process.env[env] || '';
}

const ROOT = (argVal('--root=', 'CV_ROOT') || path.resolve(__dirname, '..'));
const PORT = parseInt(argVal('--port=', 'CV_PORT') || '8090', 10);
const HOST = '0.0.0.0';

const RESOLVED_ROOT = path.resolve(ROOT);

/* ---------------- 配置 ---------------- */
const IGNORED = new Set([
  '.git', 'node_modules', '__pycache__', '.idea', '.vscode',
  'DebugConfig', 'EventRecorderStub.scvd', '.cache', '.pytest_cache'
]);

const TEXT_EXTS = new Set([
  'c','h','cpp','hpp','cc','cxx','js','mjs','cjs','ts','jsx','tsx','py','java',
  'json','md','html','htm','css','scss','less','sh','bash','yaml','yml','ini',
  'conf','cfg','txt','xml','svg','sql','toml','makefile','s','asm','ld','map',
  'inc','mk','properties','log','gitignore','editorconfig','clangd','ioc',
  'uvprojx','uvoptx','sct','scvd','rc','bat','cmd','ps1','rst','adoc','csv','tsv',
  'php','rb','go','rs','lua','swift','kt','dart','vue','svelte','dockerfile',
  'cmake','pri','pro','qml','list','def','tex','bib','ini'
]);

/* ---------------- 工具函数 ---------------- */
function safeResolve(rel) {
  if (!rel) rel = '';
  const resolved = path.resolve(RESOLVED_ROOT, rel);
  if (resolved !== RESOLVED_ROOT && !resolved.startsWith(RESOLVED_ROOT + path.sep)) {
    return null;
  }
  return resolved;
}

function extOf(name) {
  const lower = name.toLowerCase();
  if (!lower.includes('.')) return '';
  return lower.split('.').pop();
}

function langOf(ext, name) {
  const m = {
    c: 'c', h: 'c', cpp: 'cpp', hpp: 'cpp', cc: 'cpp', cxx: 'cpp',
    js: 'js', mjs: 'js', cjs: 'js', ts: 'ts', jsx: 'js', tsx: 'ts',
    py: 'py', java: 'java', json: 'json', md: 'md', html: 'html', htm: 'html',
    css: 'css', scss: 'css', less: 'css', sh: 'sh', bash: 'sh',
    yaml: 'yaml', yml: 'yaml', ini: 'ini', conf: 'ini', cfg: 'ini',
    txt: 'txt', xml: 'xml', svg: 'xml', sql: 'sql', toml: 'toml',
    s: 'asm', asm: 'asm', go: 'go', rs: 'rs', lua: 'lua', php: 'php',
    rb: 'rb', swift: 'swift', kt: 'kt', dart: 'dart', vue: 'html',
    gitignore: 'txt', editorconfig: 'ini', clangd: 'yaml', ioc: 'ini',
    uvprojx: 'xml', uvoptx: 'xml', sct: 'txt', scvd: 'txt', map: 'txt',
    mk: 'mk', makefile: 'mk', dockerfile: 'txt', cmake: 'txt', bat: 'txt', cmd: 'txt'
  };
  const lname = name.toLowerCase();
  if (lname === 'makefile' || lname === 'gnumakefile') return 'mk';
  if (lname === 'dockerfile') return 'dockerfile';
  if (lname === 'cmakelists.txt') return 'cmake';
  return m[ext] || 'txt';
}

function mimeOf(ext) {
  const m = {
    png: 'image/png', jpg: 'image/jpeg', jpeg: 'image/jpeg', gif: 'image/gif',
    webp: 'image/webp', svg: 'image/svg+xml', bmp: 'image/bmp', ico: 'image/x-icon',
    html: 'text/html', css: 'text/css', js: 'text/javascript', json: 'application/json',
    txt: 'text/plain', md: 'text/markdown', xml: 'application/xml'
  };
  return m[ext] || '';
}

function sizeStr(n) {
  if (n < 0) return '';
  if (n < 1024) return n + ' B';
  if (n < 1024 * 1024) return (n / 1024).toFixed(1) + ' KB';
  return (n / 1024 / 1024).toFixed(2) + ' MB';
}

function isBinary(fpath) {
  try {
    const fd = fs.openSync(fpath, 'r');
    const buf = Buffer.alloc(8192);
    const n = fs.readSync(fd, buf, 0, 8192, 0);
    fs.closeSync(fd);
    for (let i = 0; i < n; i++) if (buf[i] === 0) return true;
  } catch (e) { return true; }
  return false;
}

function escapeRegExp(s) {
  return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

/* ---------------- 业务 API ---------------- */
function listDir(rel) {
  const dir = safeResolve(rel);
  if (!dir || !fs.statSync(dir).isDirectory()) return null;
  let entries;
  try { entries = fs.readdirSync(dir, { withFileTypes: true }); }
  catch (e) { return null; }

  const out = [];
  for (const e of entries) {
    if (IGNORED.has(e.name)) continue;
    const full = path.join(dir, e.name);
    let type = '', size = -1, ext = '';
    if (e.isDirectory()) type = 'dir';
    else if (e.isFile()) {
      type = 'file';
      try { size = fs.statSync(full).size; } catch (err) {}
      ext = extOf(e.name);
    } else continue;
    out.push({ name: e.name, type, size, ext });
  }
  out.sort((a, b) => {
    if (a.type !== b.type) return a.type === 'dir' ? -1 : 1;
    return a.name.localeCompare(b.name);
  });
  return out;
}

function readFileApi(rel) {
  const f = safeResolve(rel);
  if (!f || !fs.statSync(f).isFile()) return null;
  const st = fs.statSync(f);
  const name = path.basename(f);
  const ext = extOf(name);

  if (st.size > 2 * 1024 * 1024) {
    return { ok: false, error: '文件过大（>2MB），暂不支持在线查看', name, size: st.size, path: rel };
  }

  if (isBinary(f)) {
    const mime = mimeOf(ext);
    return {
      ok: true, binary: true, mime: mime || 'application/octet-stream',
      name, path: rel, size: st.size, ext, lang: ''
    };
  }

  let content;
  try { content = fs.readFileSync(f, 'utf8'); }
  catch (e) { return { ok: false, error: '读取失败: ' + e.message }; }
  return {
    ok: true, binary: false, name, path: rel, size: st.size, ext,
    lang: langOf(ext, name), lines: content.split('\n').length, content
  };
}

function searchFiles(q) {
  const rx = new RegExp(escapeRegExp(q), 'i');
  const results = [];
  const MAX = 400;

  (function walk(dir, rel) {
    let entries;
    try { entries = fs.readdirSync(dir, { withFileTypes: true }); }
    catch (e) { return; }
    for (const e of entries) {
      if (IGNORED.has(e.name)) continue;
      if (results.length >= MAX) return;
      const full = path.join(dir, e.name);
      const rel2 = rel ? rel + '/' + e.name : e.name;
      if (e.isDirectory()) {
        if (rx.test(e.name)) results.push({ name: e.name, type: 'dir', path: rel2, size: -1 });
        walk(full, rel2);
      } else if (e.isFile() && rx.test(e.name)) {
        let size = 0;
        try { size = fs.statSync(full).size; } catch (err) {}
        results.push({ name: e.name, type: 'file', path: rel2, size });
      }
    }
  })(RESOLVED_ROOT, '');

  return results.slice(0, MAX);
}

/* ---------------- HTTP 服务 ---------------- */
function sendJson(res, code, obj) {
  const body = JSON.stringify(obj);
  res.writeHead(code, {
    'Content-Type': 'application/json; charset=utf-8',
    'Cache-Control': 'no-store',
    'Access-Control-Allow-Origin': '*'
  });
  res.end(body);
}

function sendErr(res, code, msg) {
  sendJson(res, code, { ok: false, error: msg });
}

const server = http.createServer((req, res) => {
  const u = new URL(req.url, 'http://localhost');
  const p = u.pathname;

  if (p === '/api/info') {
    sendJson(res, 200, { ok: true, root: RESOLVED_ROOT, rootName: path.basename(RESOLVED_ROOT), port: PORT });
    return;
  }

  if (p === '/api/list') {
    const dir = u.searchParams.get('dir') || '';
    const entries = listDir(dir);
    if (!entries) return sendErr(res, 404, '目录不存在: ' + dir);
    sendJson(res, 200, { ok: true, dir, entries });
    return;
  }

  if (p === '/api/file') {
    const r = readFileApi(u.searchParams.get('path') || '');
    if (!r) return sendErr(res, 404, '文件不存在');
    sendJson(res, 200, r);
    return;
  }

  if (p === '/api/raw') {
    const f = safeResolve(u.searchParams.get('path') || '');
    if (!f || !fs.statSync(f).isFile()) return sendErr(res, 404, '文件不存在');
    const mime = mimeOf(extOf(path.basename(f)));
    if (!mime.startsWith('image/')) return sendErr(res, 415, '仅支持图片预览');
    res.writeHead(200, { 'Content-Type': mime, 'Cache-Control': 'no-store' });
    fs.createReadStream(f).pipe(res);
    return;
  }

  if (p === '/api/search') {
    const q = (u.searchParams.get('q') || '').trim();
    if (!q) return sendJson(res, 200, { ok: true, q, results: [] });
    sendJson(res, 200, { ok: true, q, results: searchFiles(q) });
    return;
  }

  /* 静态资源 */
  let fpath;
  if (p === '/') fpath = path.join(__dirname, 'public', 'index.html');
  else fpath = path.join(__dirname, 'public', path.normalize(p));
  if (!fpath.startsWith(path.join(__dirname, 'public'))) return sendErr(res, 403, '禁止访问');
  if (!fs.existsSync(fpath) || !fs.statSync(fpath).isFile()) return sendErr(res, 404, 'Not Found');

  const ext = extOf(fpath);
  const mime = mimeOf(ext) || 'application/octet-stream';
  res.writeHead(200, { 'Content-Type': mime, 'Cache-Control': 'no-store' });
  fs.createReadStream(fpath).pipe(res);
});

server.listen(PORT, HOST, () => {
  console.log('==============================================');
  console.log('  CodeView 手机代码查看器已启动');
  console.log('  根目录: ' + RESOLVED_ROOT);
  console.log('  本机访问: http://localhost:' + PORT);
  const nets = os.networkInterfaces();
  for (const name of Object.keys(nets)) {
    for (const net of nets[name] || []) {
      if (net.family === 'IPv4' && !net.internal) {
        console.log('  局域网访问(手机同WiFi): http://' + net.address + ':' + PORT);
      }
    }
  }
  console.log('==============================================');
});
