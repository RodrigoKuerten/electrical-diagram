// GDI trace service for the 32-bit CADe_SIMU process.
// Usage: node gdi_service.js <pid> <out-dir>
// stdin commands: start | stop <name> | quit
const frida = require('frida');
const fs = require('fs');
const path = require('path');
const readline = require('readline');

const pid = parseInt(process.argv[2], 10);
const outDir = process.argv[3];
fs.mkdirSync(outDir, { recursive: true });

const source = `
let records = [];
let recording = false;
let seq = 0;

function rec(fn, args) {
  if (!recording) return;
  records.push(Object.assign({ seq: seq++, fn: fn }, args));
}

const gdi = Process.getModuleByName('gdi32.dll');

function hook(name, reader) {
  let addr;
  try { addr = gdi.getExportByName(name); } catch (e) { return; }
  Interceptor.attach(addr, {
    onEnter(args) {
      if (!recording) return;
      try { reader(args); } catch (e) { rec(name, { error: String(e) }); }
    }
  });
}

hook('MoveToEx', a => rec('MoveToEx', { hdc: a[0].toInt32(), x: a[1].toInt32(), y: a[2].toInt32() }));
hook('LineTo', a => rec('LineTo', { hdc: a[0].toInt32(), x: a[1].toInt32(), y: a[2].toInt32() }));
hook('Ellipse', a => rec('Ellipse', { hdc: a[0].toInt32(), l: a[1].toInt32(), t: a[2].toInt32(), r: a[3].toInt32(), b: a[4].toInt32() }));
hook('Rectangle', a => rec('Rectangle', { hdc: a[0].toInt32(), l: a[1].toInt32(), t: a[2].toInt32(), r: a[3].toInt32(), b: a[4].toInt32() }));
hook('RoundRect', a => rec('RoundRect', { hdc: a[0].toInt32(), l: a[1].toInt32(), t: a[2].toInt32(), r: a[3].toInt32(), b: a[4].toInt32(), ew: a[5].toInt32(), eh: a[6].toInt32() }));
hook('Arc', a => rec('Arc', { hdc: a[0].toInt32(), l: a[1].toInt32(), t: a[2].toInt32(), r: a[3].toInt32(), b: a[4].toInt32(), xs: a[5].toInt32(), ys: a[6].toInt32(), xe: a[7].toInt32(), ye: a[8].toInt32() }));
hook('Pie', a => rec('Pie', { hdc: a[0].toInt32(), l: a[1].toInt32(), t: a[2].toInt32(), r: a[3].toInt32(), b: a[4].toInt32(), xs: a[5].toInt32(), ys: a[6].toInt32(), xe: a[7].toInt32(), ye: a[8].toInt32() }));
hook('Polyline', a => {
  const count = a[2].toInt32();
  const pts = [];
  for (let i = 0; i < Math.min(count, 500); i++) {
    pts.push([a[1].add(i * 8).readS32(), a[1].add(i * 8 + 4).readS32()]);
  }
  rec('Polyline', { hdc: a[0].toInt32(), points: pts });
});
hook('Polygon', a => {
  const count = a[2].toInt32();
  const pts = [];
  for (let i = 0; i < Math.min(count, 500); i++) {
    pts.push([a[1].add(i * 8).readS32(), a[1].add(i * 8 + 4).readS32()]);
  }
  rec('Polygon', { hdc: a[0].toInt32(), points: pts });
});
hook('TextOutA', a => rec('TextOutA', { hdc: a[0].toInt32(), x: a[1].toInt32(), y: a[2].toInt32(), text: a[3].readAnsiString(a[4].toInt32()) }));
hook('TextOutW', a => rec('TextOutW', { hdc: a[0].toInt32(), x: a[1].toInt32(), y: a[2].toInt32(), text: a[3].readUtf16String(a[4].toInt32()) }));
hook('SetTextColor', a => rec('SetTextColor', { hdc: a[0].toInt32(), color: a[1].toInt32() >>> 0 }));
hook('SetBkColor', a => rec('SetBkColor', { hdc: a[0].toInt32(), color: a[1].toInt32() >>> 0 }));
hook('SetBkMode', a => rec('SetBkMode', { hdc: a[0].toInt32(), mode: a[1].toInt32() }));
hook('CreatePen', a => rec('CreatePen', { style: a[0].toInt32(), width: a[1].toInt32(), color: a[2].toInt32() >>> 0 }));
hook('CreateSolidBrush', a => rec('CreateSolidBrush', { color: a[0].toInt32() >>> 0 }));
hook('SelectObject', a => rec('SelectObject', { hdc: a[0].toInt32(), obj: a[1].toInt32() }));
hook('DeleteObject', a => rec('DeleteObject', { obj: a[0].toInt32() }));
hook('SetROP2', a => rec('SetROP2', { hdc: a[0].toInt32(), mode: a[1].toInt32() }));
hook('SetPolyFillMode', a => rec('SetPolyFillMode', { hdc: a[0].toInt32(), mode: a[1].toInt32() }));

rpc.exports = {
  start() { records = []; seq = 0; recording = true; return true; },
  stop() { recording = false; return records; }
};
`;

(async () => {
  const device = await frida.getLocalDevice();
  const session = await device.attach(pid);
  const script = await session.createScript(source);
  await script.load();
  console.log('READY');
  const rl = readline.createInterface({ input: process.stdin });
  rl.on('line', async (line) => {
    const trimmed = line.trim();
    if (!trimmed) return;
    if (trimmed === 'start') {
      await script.exports.start();
      console.log('STARTED');
    } else if (trimmed.startsWith('stop ')) {
      const name = trimmed.slice(5).trim();
      const records = await script.exports.stop();
      const file = path.join(outDir, name + '.json');
      fs.writeFileSync(file, JSON.stringify(records));
      console.log('STOPPED ' + file + ' ' + records.length);
    } else if (trimmed === 'quit') {
      await script.unload();
      await session.detach();
      process.exit(0);
    }
  });
})().catch(err => { console.error('ERR', err); process.exit(1); });
