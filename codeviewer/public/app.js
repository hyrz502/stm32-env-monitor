/* ============ CodeView 手机代码查看器 ============ */
'use strict';

/* ---------------- 状态 ---------------- */
const state = {
  rootName: '',
  dirCache: {},          // rel -> entries
  rootNode: null,        // 文件树根节点
  treeView: 'tree',      // 'tree' | 'search'
  searchResults: [],     // 搜索结果(顶层匹配)
  expandedDirs: new Set(), // 已展开目录 rel
  openFiles: [],         // [{path,name,lang,binary,mime,size}]
  active: null,          // 当前文件 path
  contents: {},          // path -> 文件数据(已渲染HTML缓存)
  searchQ: ''
};

const $ = s => document.querySelector(s);
const MAX_RENDER_LINES = 20000;

/* ---------------- 工具 ---------------- */
async function api(url) {
  const r = await fetch(url);
  return r.json();
}

function esc(s) {
  return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function sizeStr(n) {
  if (n < 0) return '';
  if (n < 1024) return n + ' B';
  if (n < 1048576) return (n / 1024).toFixed(1) + ' KB';
  return (n / 1048576).toFixed(2) + ' MB';
}

let toastTimer = null;
function toast(msg) {
  const t = $('#toast');
  t.textContent = msg;
  t.classList.add('show');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => t.classList.remove('show'), 2000);
}

/* ---------------- 语法高亮 ---------------- */
const LANGS = {
  c: {
    preproc: true,
    keywords: ['auto','break','case','char','const','continue','default','do','double','else','enum','extern','float','for','goto','if','inline','int','long','register','return','short','signed','sizeof','static','struct','switch','typedef','union','unsigned','void','volatile','while'],
    types: ['uint8_t','uint16_t','uint32_t','uint64_t','int8_t','int16_t','int32_t','int64_t','size_t','bool','true','false','NULL','FILE','char_t']
  },
  cpp: {
    preproc: true,
    keywords: ['auto','break','case','catch','class','const','constexpr','continue','default','delete','do','double','else','enum','explicit','extern','false','float','for','friend','goto','if','inline','int','long','namespace','new','nullptr','operator','private','protected','public','register','return','short','signed','sizeof','static','struct','switch','template','this','throw','true','try','typedef','typename','union','unsigned','using','virtual','void','volatile','while'],
    types: ['uint8_t','uint16_t','uint32_t','uint64_t','int8_t','int16_t','int32_t','int64_t','size_t','bool','string','vector','map','set','list','queue','stack','shared_ptr','unique_ptr','NULL','FILE']
  },
  js: {
    preproc: false,
    keywords: ['var','let','const','function','return','if','else','for','while','do','switch','case','break','continue','new','class','extends','super','this','typeof','instanceof','in','of','try','catch','finally','throw','async','await','yield','default','import','export','from','delete','void'],
    types: ['undefined','null','true','false','NaN','Infinity','Object','Array','String','Number','Boolean','Promise','Set','Map','Error','console']
  },
  ts: {
    preproc: false,
    keywords: ['var','let','const','function','return','if','else','for','while','do','switch','case','break','continue','new','class','extends','implements','super','this','typeof','instanceof','in','of','try','catch','finally','throw','async','await','yield','default','import','export','from','interface','type','enum','declare','namespace','module','readonly','delete','void','public','private','protected','static','abstract','as','any','unknown','never'],
    types: ['undefined','null','true','false','NaN','number','string','boolean','symbol','Object','Array','String','Number','Boolean','Promise','Set','Map','Error','console','void','bigint']
  },
  py: {
    preproc: false,
    keywords: ['def','return','if','elif','else','for','while','in','not','and','or','pass','break','continue','import','from','as','class','try','except','finally','with','lambda','yield','global','nonlocal','raise','del','assert','is','None','True','False'],
    types: ['print','len','range','type','str','int','float','bool','list','dict','tuple','set','super','self','object','Exception','abs','min','max','sum','open']
  },
  java: {
    preproc: false,
    keywords: ['public','private','protected','static','final','void','int','long','double','float','boolean','char','byte','short','class','interface','extends','implements','new','return','if','else','for','while','do','switch','case','break','continue','try','catch','finally','throw','throws','package','import','this','super','instanceof','abstract','volatile','synchronized','default','enum','null','true','false','native','transient','strictfp'],
    types: ['String','Integer','Long','Double','Float','Boolean','Character','Byte','Short','Object','List','Map','Set','ArrayList','HashMap','HashSet','System','Exception','Thread','Math']
  },
  go: {
    preproc: false,
    keywords: ['package','import','func','var','const','type','struct','interface','map','chan','go','defer','select','range','return','if','else','for','switch','case','break','continue','default','fallthrough','goto'],
    types: ['int','int8','int16','int32','int64','uint','uint8','uint16','uint32','uint64','float32','float64','string','bool','byte','rune','error','nil','true','false','any']
  },
  rs: {
    preproc: false,
    keywords: ['fn','let','mut','pub','struct','enum','impl','trait','use','mod','crate','super','self','match','if','else','for','while','loop','return','break','continue','const','static','ref','where','as','dyn','async','await','move','unsafe','in','type'],
    types: ['i8','i16','i32','i64','i128','u8','u16','u32','u64','u128','f32','f64','bool','char','str','String','Vec','Option','Result','Box','true','false','None','Some','Ok','Err']
  },
  sh: {
    preproc: false,
    keywords: ['if','then','else','elif','fi','for','while','until','do','done','case','esac','function','return','echo','exit','export','local','read','set','unset','trap','source','shift','select'],
    types: ['true','false']
  },
  php: {
    preproc: false,
    keywords: ['echo','print','function','return','if','else','elseif','foreach','as','while','for','switch','case','break','continue','new','class','extends','implements','public','private','protected','static','const','var','global','try','catch','finally','throw','require','include','require_once','include_once','namespace','use','abstract','final','interface','instanceof','null','true','false'],
    types: ['string','int','float','bool','array','object','$this']
  },
  rb: {
    preproc: false,
    keywords: ['def','end','if','elsif','else','unless','while','until','for','do','return','class','module','require','puts','print','each','case','when','begin','rescue','ensure','yield','and','or','not','nil','true','false','self','super','new','alias'],
    types: ['String','Integer','Float','Array','Hash','Symbol','Object','nil']
  },
  sql: {
    preproc: false,
    keywords: ['select','from','where','insert','into','values','update','set','delete','create','table','index','view','drop','alter','join','inner','left','right','outer','full','on','group','by','order','having','limit','distinct','as','and','or','not','union','all','exists','between','like','in','is','null','primary','key','foreign','references','default','unique','check','asc','desc','offset','count','sum','avg','min','max','case','when','then','else','end'],
    types: ['true','false','null','int','varchar','char','text','date','time','timestamp','boolean','float','double','decimal']
  },
  yaml: {
    preproc: false,
    keywords: [],
    types: ['true','false','null','yes','no','on','off','None','True','False']
  },
  json: {
    preproc: false,
    keywords: [],
    types: ['true','false','null']
  },
  html: {
    preproc: false, html: true,
    keywords: [],
    types: []
  },
  css: {
    preproc: false,
    keywords: [],
    types: ['red','blue','green','black','white','none','solid','dashed','dotted','flex','block','inline','absolute','relative','fixed','sticky']
  },
  asm: {
    preproc: false,
    keywords: ['mov','add','sub','mul','div','push','pop','call','ret','jmp','je','jne','jz','jnz','jg','jl','jge','jle','cmp','and','or','xor','not','shl','shr','inc','dec','loop','nop','int','ldr','str','b','bl','bx','cpsr','svc','mrs','msr','bne','beq','bgt','blt','bge','ble','tst','teq','ldmia','stmia'],
    types: ['r0','r1','r2','r3','r4','r5','r6','r7','r8','r9','r10','r11','r12','r13','r14','r15','sp','lr','pc','r0-r7']
  },
  mk: {
    preproc: false,
    keywords: ['if','ifdef','ifndef','else','endif','include','define','export','unexport','override','define','undefine','target','phony','clean'],
    types: ['true','false','MAKE','CC','CXX','AR','LD']
  },
  lua: {
    preproc: false,
    keywords: ['function','end','if','then','else','elseif','for','while','do','repeat','until','return','local','nil','true','false','and','or','not','break','goto'],
    types: ['print','pairs','ipairs','type','tostring','tonumber','string','table','math','os','io']
  },
  txt: { preproc: false, keywords: [], types: [] }
};

function highlight(src, lang) {
  const cfg = LANGS[lang] || LANGS.txt;
  const toks = [];
  let s = src;

  const place = (re, type) => {
    s = s.replace(re, m => {
      const id = toks.length;
      toks.push({ type, text: m });
      return '\u0001' + id + '\u0002';
    });
  };

  // 字符串
  if (lang === 'py') place(/"""[\s\S]*?"""|'''[\s\S]*?'''/g, 'str');
  place(/"(\\["\\bfnrtv]|\\x[0-9a-fA-F]{2}|\\u[0-9a-fA-F]{4}|[^"\\\n])*"/g, 'str');
  place(/'(\\['\\bfnrtv]|\\x[0-9a-fA-F]{2}|\\u[0-9a-fA-F]{4}|[^'\\\n])*'/g, 'str');

  // 注释
  const hashLangs = ['py','sh','yaml','ini','asm','mk','lua','rb','go','dockerfile'];
  const slashLangs = ['c','cpp','java','go','rs','js','ts','css','swift','kt','php','swift'];
  if (hashLangs.includes(lang)) place(/#[^\n]*/g, 'cmt');
  if (lang === 'lua') place(/\-\-\[\[[\s\S]*?\]\]|\-\-[^\n]*/g, 'cmt');
  if (slashLangs.includes(lang)) place(/\/\/[^\n]*/g, 'cmt');
  place(/\/\*[\s\S]*?\*\//g, 'cmt');
  if (lang === 'html' || lang === 'xml') place(/<!--[\s\S]*?-->/g, 'cmt');

  // 预处理
  if (cfg.preproc) place(/^\s*#\s*\w+[^\n]*/gm, 'pp');

  // HTML/CSS/MD 特殊
  if (cfg.html) {
    place(/<\/?[a-zA-Z][a-zA-Z0-9-]*[^>]*>/g, 'tag');
    place(/[a-zA-Z-]+(?=\s*=\s*["'])/g, 'attr');
  }
  if (lang === 'css') {
    place(/@[\w-]+/g, 'pp');
    place(/\.[a-zA-Z-]+|#[a-zA-Z-]+|::?[a-zA-Z-]+/g, 'ty');
  }
  if (lang === 'md') {
    place(/^#{1,6}\s.*$/gm, 'head');
    place(/\*\*[^*]+\*\*/g, 'bold');
    place(/`[^`]+`/g, 'fn');
    place(/\[[^\]]*\]\([^)]*\)/g, 'link');
  }

  // 关键字 / 类型 / 函数 / 数字: 合并为单个正则一次执行，
  // 避免多个 replace 先后执行时，后面的规则误匹配前面占位符里的数字/单词。
  const applyWordRules = (seg) => {
    const alts = [];
    const kwSet = new Set(cfg.keywords);
    const tySet = new Set(cfg.types);
    if (cfg.keywords.length) alts.push('\\b(?:' + cfg.keywords.join('|') + ')\\b');
    if (cfg.types.length) alts.push('\\b(?:' + cfg.types.join('|') + ')\\b');
    if (lang !== 'txt') alts.push('\\b[A-Za-z_][\\w]*(?=\\s*\\()');
    alts.push('\\b(?:0[xX][0-9a-fA-F]+|\\d+\\.?\\d*[uUlLfF]*%?)\\b');
    const re = new RegExp('(' + alts.join('|') + ')', 'g');
    return seg.replace(re, (m) => {
      let type = 'fn';
      if (/^\d/.test(m) || /^0[xX]/i.test(m)) type = 'num';
      else if (kwSet.has(m)) type = 'kw';
      else if (tySet.has(m)) type = 'ty';
      return inlineTok(m, toks, type);
    });
  };

  const parts = s.split(/(\u0001\d+\u0002)/g);
  let out = '';
  for (const part of parts) {
    if (/^\u0001\d+\u0002$/.test(part)) out += part;   // 占位符原样保留
    else out += esc(applyWordRules(part));
  }
  // 迭代还原占位符(支持嵌套占位符，如 #include "x.h" 中字符串在预处理行内)
  for (let guard = 0; guard < 20; guard++) {
    let changed = false;
    out = out.replace(/\u0001(\d+)\u0002/g, (m, id) => {
      changed = true;
      const t = toks[+id];
      return '<span class="' + t.type + '">' + esc(t.text) + '</span>';
    });
    if (!changed) break;
  }
  return out;
}

function inlineTok(m, toks, type) {
  const id = toks.length;
  toks.push({ type: type || 'tmp', text: m });
  return '\u0001' + id + '\u0002';
}

/* ---------------- 文件树 ---------------- */
const FOLDER_SVG = '<svg viewBox="0 0 24 24" width="16" height="16" fill="currentColor"><path d="M10 4H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2h-8l-2-2z"/></svg>';

async function getChildren(rel) {
  if (state.dirCache[rel] !== undefined) return state.dirCache[rel];
  try {
    const r = await api('/api/list?dir=' + encodeURIComponent(rel));
    if (!r.ok) { state.dirCache[rel] = []; return []; }
    state.dirCache[rel] = r.entries;
    return r.entries;
  } catch (e) { return []; }
}

function badgeFor(ext, name) {
  const lower = name.toLowerCase();
  let cls = ext || 'txt';
  if (lower === 'makefile' || lower === 'gnumakefile') cls = 'mk';
  if (lower === 'cmakelists.txt') cls = 'cmake';
  if (lower === 'dockerfile') cls = 'txt';
  if (!ext) cls = 'txt';
  return '<span class="badge ' + cls + '">' + (cls.length > 3 ? cls.slice(0, 3) : cls) + '</span>';
}

function renderTree() {
  const tree = $('#tree');
  tree.innerHTML = '';
  const root = state.rootNode;
  if (!root) return;
  const ul = document.createElement('ul');
  ul.className = 'tree-list';
  buildNodeHtml(root, ul);
  tree.appendChild(ul);
  markActiveRows();
}

function buildNodeHtml(node, parentUl) {
  const li = document.createElement('li');
  li.className = 'node ' + (node.entry.type === 'dir' ? 'dir' : 'file') + (node.open ? ' open' : '');
  li.dataset.path = node.path;
  li.dataset.type = node.entry.type;

  const row = document.createElement('div');
  row.className = 'row';
  row.dataset.path = node.path;
  row.dataset.type = node.entry.type;

  const caret = document.createElement('span');
  caret.className = 'caret';
  caret.textContent = node.entry.type === 'dir' ? '▸' : '';

  const icon = document.createElement('span');
  icon.className = 'folder-ico';
  if (node.entry.type === 'dir') icon.innerHTML = FOLDER_SVG;
  else icon.innerHTML = badgeFor(node.entry.ext, node.entry.name);

  const name = document.createElement('span');
  name.className = 'name';
  name.textContent = node.entry.name;

  const size = document.createElement('span');
  size.className = 'size';
  if (node.entry.type === 'file') size.textContent = sizeStr(node.entry.size);

  row.appendChild(caret);
  row.appendChild(icon);
  row.appendChild(name);
  row.appendChild(size);
  li.appendChild(row);

  const childrenUl = document.createElement('ul');
  childrenUl.className = 'tree-list';
  li.appendChild(childrenUl);
  parentUl.appendChild(li);

  if (node.open && node.children) {
    for (const c of node.children) buildNodeHtml(c, childrenUl);
  }
}

async function loadNodeChildren(node) {
  if (node.children) return;
  const entries = await getChildren(node.path);
  node.children = entries.map(e => ({
    entry: e,
    path: node.path ? node.path + '/' + e.name : e.name,
    open: false,
    children: null
  }));
  node.loaded = true;
}

async function toggleNode(node, rowEl) {
  if (node.entry.type !== 'dir') return;
  if (node.open) {
    node.open = false;
  } else {
    await loadNodeChildren(node);
    node.open = true;
  }
  renderTree();
}

/* ---------------- 搜索 ---------------- */
let searchTimer = null;
async function doSearch(q) {
  q = q.trim();
  if (!q) { backToTree(); return; }
  try {
    const r = await api('/api/search?q=' + encodeURIComponent(q));
    state.searchResults = r.results || [];
    state.searchQ = q;
    renderSearchResults();
    openDrawer();   // 自动打开抽屉展示结果
  } catch (e) {
    toast('搜索失败');
  }
}

function renderSearchResults() {
  const tree = $('#tree');
  tree.innerHTML = '';
  if (!state.searchResults.length) {
    tree.innerHTML = '<div class="search-empty">未找到匹配的文件</div>';
    return;
  }
  tree.appendChild(buildSearchList(state.searchResults, ''));
}

function buildSearchList(items, prefix) {
  const ul = document.createElement('ul');
  ul.className = 'tree-list';
  for (const it of items) {
    const rel = it.path;
    const li = document.createElement('li');
    li.className = 'node ' + it.type + (state.expandedDirs.has(rel) ? ' open' : '');
    li.dataset.path = rel;
    li.dataset.type = it.type;

    const row = document.createElement('div');
    row.className = 'row';
    row.dataset.path = rel;
    row.dataset.type = it.type;

    const caret = document.createElement('span');
    caret.className = 'caret';
    caret.textContent = it.type === 'dir' ? '▸' : '';

    const icon = document.createElement('span');
    icon.className = 'folder-ico';
    if (it.type === 'dir') icon.innerHTML = FOLDER_SVG;
    else icon.innerHTML = badgeFor(it.ext, it.name);

    const name = document.createElement('span');
    name.className = 'name';
    name.textContent = it.name;

    const size = document.createElement('span');
    size.className = 'size';
    if (it.type === 'file') size.textContent = sizeStr(it.size);

    row.appendChild(caret); row.appendChild(icon); row.appendChild(name); row.appendChild(size);
    li.appendChild(row);

    // 已展开目录: 递归显示其子节点
    if (it.type === 'dir' && state.expandedDirs.has(rel)) {
      const sub = document.createElement('ul');
      sub.className = 'tree-list';
      li.appendChild(sub);
      // 子节点异步填充
      getChildren(rel).then(entries => {
        const subItems = entries.map(e => ({ name: e.name, type: e.type, path: rel + '/' + e.name, size: e.size, ext: e.ext }));
        const subUl = buildSearchList(subItems, rel);
        sub.replaceWith(subUl);
      });
    }
    ul.appendChild(li);
  }
  return ul;
}

async function toggleSearchDir(pathStr) {
  if (state.expandedDirs.has(pathStr)) state.expandedDirs.delete(pathStr);
  else state.expandedDirs.add(pathStr);
  renderSearchResults();
}

/* ---------------- 打开文件 / 标签页 ---------------- */
function langColorBadge(lang) {
  return '<span class="badge ' + (lang || 'txt') + '">' + ((lang || 'txt').slice(0, 3)) + '</span>';
}

async function openFile(pathStr, name, size) {
  if (state.contents[pathStr] !== undefined) {
    setActive(pathStr);
    closeDrawer();
    return;
  }
  const r = await api('/api/file?path=' + encodeURIComponent(pathStr));
  if (!r.ok) { toast(r.error || '无法读取文件'); return; }

  const tab = { path: pathStr, name: r.name, lang: r.lang, binary: r.binary, mime: r.mime, size: r.size, lines: r.lines };
  state.openFiles.push(tab);
  state.contents[pathStr] = r;
  setActive(pathStr);
  closeDrawer();
}

function setActive(pathStr) {
  state.active = pathStr;
  renderTabs();
  renderContent(pathStr);
  scrollActiveTabIntoView();
  markActiveRows();
}

function closeTab(pathStr) {
  const idx = state.openFiles.findIndex(t => t.path === pathStr);
  if (idx < 0) return;
  state.openFiles.splice(idx, 1);
  delete state.contents[pathStr];
  if (state.active === pathStr) {
    const next = state.openFiles[idx] || state.openFiles[idx - 1];
    if (next) setActive(next.path);
    else {
      state.active = null;
      renderTabs();
      $('#codeArea').innerHTML = '';
      $('#emptyState').classList.remove('hidden');
      setStatus('就绪');
    }
  } else {
    renderTabs();
  }
}

function renderTabs() {
  const tabs = $('#tabs');
  tabs.innerHTML = '';
  if (!state.openFiles.length) return;
  for (const t of state.openFiles) {
    const el = document.createElement('div');
    el.className = 'tab' + (t.path === state.active ? ' active' : '');
    el.dataset.path = t.path;

    const badge = document.createElement('span');
    badge.className = 'badge ' + (t.lang || 'txt');
    badge.textContent = (t.lang || 'txt').slice(0, 3);

    const name = document.createElement('span');
    name.className = 'tab-name';
    name.textContent = t.name;

    const x = document.createElement('span');
    x.className = 'tab-x';
    x.innerHTML = '&times;';

    el.appendChild(badge); el.appendChild(name); el.appendChild(x);
    el.addEventListener('click', e => {
      if (e.target.classList.contains('tab-x')) {
        e.stopPropagation();
        closeTab(t.path);
      } else {
        setActive(t.path);
      }
    });
    tabs.appendChild(el);
  }
}

function scrollActiveTabIntoView() {
  const el = $('#tabs').querySelector('.tab.active');
  if (el) el.scrollIntoView({ inline: 'nearest', block: 'nearest' });
}

function renderContent(pathStr) {
  const area = $('#codeArea');
  const r = state.contents[pathStr];
  if (!r) return;
  $('#emptyState').classList.add('hidden');

  if (r.binary) {
    if (r.mime && r.mime.startsWith('image/')) {
      area.innerHTML = '<div class="img-view"><img src="/api/raw?path=' + encodeURIComponent(pathStr) + '" alt=""></div>';
    } else {
      area.innerHTML = '<div class="msg-view"><div class="msg-ico">BIN</div><p>二进制文件</p><p>' + sizeStr(r.size) + '</p></div>';
    }
    return;
  }

  let content = r.content;
  let note = '';
  const lineCount = r.lines || content.split('\n').length;
  if (lineCount > MAX_RENDER_LINES) {
    note = '（仅显示前 ' + MAX_RENDER_LINES + ' 行，共 ' + lineCount + ' 行）';
    content = content.split('\n').slice(0, MAX_RENDER_LINES).join('\n');
  }

  let hl = '';
  try { hl = highlight(content, r.lang); } catch (e) { hl = esc(content); }

  // 行号
  const n = content.split('\n').length;
  let gutter = '';
  for (let i = 1; i <= n; i++) gutter += i + '\n';

  area.innerHTML =
    '<div class="code-scroll">' +
    '<div class="gutter">' + gutter + '</div>' +
    '<pre class="code">' + hl + '</pre>' +
    '</div>';

  const st = $('#statusbar');
  st.innerHTML = '';
  const sp = document.createElement('span');
  sp.className = 'sp';
  sp.textContent = pathStr + '  ' + note;
  const info = document.createElement('span');
  info.textContent = lineCount + ' 行 · ' + sizeStr(r.size);
  st.appendChild(sp);
  st.appendChild(info);
}

function setStatus(s) {
  const st = $('#statusbar');
  st.innerHTML = '<span class="sp">' + s + '</span>';
}

/* ---------------- 抽屉 ---------------- */
function openDrawer() { $('#drawer').classList.add('open'); $('#drawerBackdrop').classList.add('show'); }
function closeDrawer() { $('#drawer').classList.remove('open'); $('#drawerBackdrop').classList.remove('show'); }

/* 事件委托: 文件树点击 */
$('#tree').addEventListener('click', async e => {
  const row = e.target.closest('.row');
  if (!row) return;
  const pathStr = row.dataset.path;
  const type = row.dataset.type;
  if (state.treeView === 'tree') {
    // 在 rootNode 中查找节点
    const node = findNode(state.rootNode, pathStr);
    if (node) {
      if (type === 'dir') toggleNode(node);
      else if (type === 'file') openFile(pathStr, node.entry.name, node.entry.size);
    }
  } else {
    if (type === 'dir') toggleSearchDir(pathStr);
    else openFile(pathStr, row.querySelector('.name').textContent, 0);
  }
});

function findNode(node, pathStr) {
  if (node.path === pathStr) return node;
  if (!node.children) return null;
  for (const c of node.children) {
    const r = findNode(c, pathStr);
    if (r) return r;
  }
  return null;
}

function markActiveRows() {
  if (!state.active) return;
  document.querySelectorAll('.row').forEach(r => {
    r.classList.toggle('active', r.dataset.path === state.active);
  });
}

/* ---------------- 顶栏/搜索事件 ---------------- */
$('#btnMenu').addEventListener('click', () => { openDrawer(); });
$('#btnDrawerClose').addEventListener('click', closeDrawer);
$('#drawerBackdrop').addEventListener('click', closeDrawer);
$('#btnRefresh').addEventListener('click', () => { state.dirCache = {}; refreshRoot(); toast('已刷新'); });
$('#btnSearch').addEventListener('click', () => {
  $('#searchBar').classList.remove('hidden');
  setTimeout(() => $('#searchInput').focus(), 100);
});
$('#btnSearchClear').addEventListener('click', () => { $('#searchInput').value = ''; backToTree(); });

$('#searchInput').addEventListener('input', e => {
  clearTimeout(searchTimer);
  const q = e.target.value;
  searchTimer = setTimeout(() => {
    if (q.trim()) { state.treeView = 'search'; doSearch(q); }
    else backToTree();
  }, 250);
});

function backToTree() {
  state.treeView = 'tree';
  state.searchQ = '';
  $('#searchInput').value = '';
  $('#searchBar').classList.add('hidden');
  renderTree();
}

/* ---------------- 初始化 ---------------- */
async function refreshRoot() {
  const info = await api('/api/info');
  state.rootName = info.rootName || 'workspace';
  $('#rootName').textContent = state.rootName;

  state.rootNode = {
    entry: { name: state.rootName, type: 'dir', ext: '', size: -1 },
    path: '',
    open: true,
    loaded: false,
    children: null
  };
  await loadNodeChildren(state.rootNode);
  state.treeView = 'tree';
  renderTree();
}

refreshRoot().then(() => setStatus(state.rootName + ' 就绪')).catch(() => setStatus('加载失败'));
