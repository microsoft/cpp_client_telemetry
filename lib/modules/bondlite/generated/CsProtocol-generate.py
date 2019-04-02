#!/usr/bin/python
# Copyright (c) Microsoft. All rights reserved.

from __future__ import print_function
import re


def main():
    steps = \
        [""" Baked from BondDataPackage.bond from infrastructure_data_pipe with the following recipe: """]

    steps += \
        ["""   1) Take BondDataPackage.bond """]
    try:
        with open('BondDataPackage.bond', 'r') as f:
            output = f.read()
    except Exception as e:
        print('Error loading BondDataPackage.bond: {}'.format(e))
        return False

    steps += \
        ["""   2) Filter only relevant types and fields: """]
    wanted = {
        'BondRecordType': None,
        'BondPIIScrubber': None,
        'BondPIIKind': None,
        'BondPII': [1, 2, 3],
        'BondRecord': [1, 3, 5, 6, 13, 24, 30],
        'BondDataPackage': [1, 2, 3, 4, 5, 6, 7, 8],
        'BondClientToCollectorRequest': [1, 2, 3]
    }
    for type in wanted:
        steps.append('        {:30}{} '.format(type, wanted[type] if wanted[type] else '-all-'))
    for m in re.finditer(r'(?:^//[^\n]+[ \t\n]*)*^(?:struct|enum)\s+([^\s]+)\s+\{.*?^};?\n', output, re.MULTILINE | re.DOTALL):
        definition = m.group(0)
        name = m.group(1)
        if name not in wanted:
            updated = ''
        else:
            updated = definition
            for m in re.finditer(r'(?:^\s*//[^\n]+[ \t\n]*)*^\s*(\d+):[^\n]+\n', definition, re.MULTILINE):
                field = m.group(0)
                id = int(m.group(1))
                if id not in wanted[name]:
                    updated = updated.replace(field, '')
        output = output.replace(definition, updated)

    steps += \
        ["""   3) Change namespace to AriaProtocol """]
    output = output.replace(
        'namespace Skype.Data.Models.Server.Bond',
        'namespace AriaProtocol;')

    steps += \
        ["""   4) Remove Bond* prefix of type names """]
    output = re.sub(r'(enum\s+|struct\s+|optional\s+|,\s*|<\s*)Bond', r'\1', output)

    steps += \
        ["""   5) Remove default 'nothing' for strings to avoid maybe<> """]
    output = output.replace(' = nothing', '')

    steps += \
        ["""   6) Cleanup (trailing whitespace, extra lines, formatting etc.) """]
    output = re.sub(r'^\s+(//[^\n]*\n)*', '', output)
    output = re.sub(r'\n{3,}', '\n\n', output)
    output = re.sub(r'[ \t]+\n', '\n', output)
    output = output.replace('}\n', '};\n')
    output = output.replace('  // This name deliberately does not match the one in protobuf (Schema),\n  // as this would cause a clash in Bond generated code...\n', '')

    header = reduce(lambda buf, step: buf + "// " + step[1:-1] + "\n", steps, "")

    with open('AriaProtocol.bond', 'w') as f:
        f.write(header)
        f.write(output)


if __name__ == '__main__':
    main()
