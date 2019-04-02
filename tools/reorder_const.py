import glob
import os.path
import re

for filename in glob.iglob('**/*.[ch]pp', recursive=True):
    if not os.path.isfile(filename):
        continue
    with open(filename, 'r', encoding='utf-8') as f:
        content = f.read()
    changed = content

    for m in re.finditer(r'(\n *|[()[\]<>{},:;][ \n]*|static )const([ ]+)((?:<[^>]+>|[^ \n()[\]<>{};&*]+)*)([&*]?)', changed):
        if m.group(3) == '' or m.group(3) == 'override' or m.group(3) == '=':
            continue
        changed = changed[0:m.start()] + m.group(1) + m.group(3) + m.group(2) + 'const' + m.group(4) + changed[m.end():]

    if changed == content:
        continue
    print(filename)
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(changed)
