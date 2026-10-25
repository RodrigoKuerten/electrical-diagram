import json, collections

d = json.load(open(r'docs\cadesimu\toolbars.json', encoding='utf-8'))
seen = collections.OrderedDict()
for bid, b in d.items():
    for r in b['buttons']:
        if r['command'] and r['text']:
            name = r['text'].split('\n')[0]
            item = seen.setdefault(r['command'], {'name': name, 'bars': set(), 'texts': set()})
            item['bars'].add(int(bid))
            item['texts'].add(r['text'])

with open(r'docs\cadesimu\named_commands.tsv', 'w', encoding='utf-8') as f:
    f.write('command\tbars\tname\n')
    for cmd, item in sorted(seen.items()):
        f.write(f"{cmd}\t{','.join(map(str, sorted(item['bars'])))}\t{item['name']}\n")
print('unique named commands', len(seen))
