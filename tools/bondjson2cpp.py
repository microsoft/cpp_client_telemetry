#!/usr/bin/python
# Copyright (c) Microsoft. All rights reserved.

from __future__ import print_function
import argparse
import json
import os


__version__ = '2017.09.19.1'


class BondJson2Cpp:
    """Class encompassing the whole conversion process"""

    def __init__(self):
        """Construct the class and set basic structures"""
        self.types_map = {
            'bool': {'c++type': 'bool', 'def': 'false', 'writer': 'Bool'},
            'int8': {'c++type': 'int8_t', 'def': '0', 'writer': 'Int8'},
            'int16': {'c++type': 'int16_t', 'def': '0', 'writer': 'Int16'},
            'int32': {'c++type': 'int32_t', 'def': '0', 'writer': 'Int32'},
            'int64': {'c++type': 'int64_t', 'def': '0', 'writer': 'Int64'},
            'uint8': {'c++type': 'uint8_t', 'def': '0', 'writer': 'UInt8'},
            'uint16': {'c++type': 'uint16_t', 'def': '0', 'writer': 'UInt16'},
            'uint32': {'c++type': 'uint32_t', 'def': '0', 'writer': 'UInt32'},
            'uint64': {'c++type': 'uint64_t', 'def': '0', 'writer': 'UInt64'},
            'float': {'c++type': 'float', 'def': '0.0f', 'writer': 'Float'},
            'double': {'c++type': 'double', 'def': '0.0', 'writer': 'Double'},
            'string': {'c++type': 'std::string', 'isset': '!{0}.empty()', 'writer': 'String'},
        }
        self.output_file = None

    def process_bond_data_types(self):
        """Convert basic Bond types from bond_const.json"""
        try:
            with open('bond_const.json', 'r') as f:
                data = json.load(f)
        except Exception as e:
            print('Error loading bond_const.json: {}'.format(e))
            return False

        print('bond_const.json')
        print('    Creating BondConstTypes.hpp...')
        with open('BondConstTypes.hpp', 'w') as f:
            self.output_file = f
            self.write_header('bond_const.json', ())
            self.wl(0, '')
            self.wl(0, 'namespace bond_lite {{')

            for d in data['declarations']:
                self.wl(0, '')
                self.wl(0, 'enum {} {{', d['declName'])
                max_len = max(map(lambda c: len(c['constantName']), d['enumConstants']))
                for i, c in enumerate(d['enumConstants']):
                    self.wl(1, '{:{}} = {}{}', c['constantName'], max_len, c['constantValue'], ',' if i < len(d['enumConstants']) - 1 else '')
                self.wl(0, '}};')

            self.wl(0, '')
            self.wl(0, '}} // namespace bond_lite')
            self.output_file = None

        print('    Done.')
        print('')
        return True

    def format_bond_type(self, ft):
        """Return human-readable description of specified Bond type (for comments)"""
        if type(ft) is dict:
            if ft['type'] == 'user':
                return ft['declaration']['declName']
            elif ft['type'] == 'vector':
                return 'vector<{}>'.format(self.format_bond_type(ft['element']))
            elif ft['type'] == 'map':
                return 'map<{}, {}>'.format(self.format_bond_type(ft['key']), self.format_bond_type(ft['element']))
            else:
                raise RuntimeError('Unsupported fieldType {}'.format(ft['type']))
        return ft

    def get_basic_type_info(self, ft):
        """Return specified type description from self.types_map, raise exception if unknown"""
        if ft not in self.types_map:
            raise RuntimeError('Unknown field type {}'.format(ft))
        return self.types_map[ft]

    def format_cpp_type(self, ft):
        """Return C++ code denoting the corresponding C++ type of specified Bond type"""
        if type(ft) is dict:
            if ft['type'] == 'user':
                return self.format_cpp_full_name(ft['declaration'])
            elif ft['type'] == 'vector':
                etype = self.format_cpp_type(ft['element'])
                # Avoid <: digraph
                if etype.startswith(':'):
                    etype = ' ' + etype
                return 'std::vector<{}>'.format(etype)
            elif ft['type'] == 'map':
                ktype = self.format_cpp_type(ft['key'])
                etype = self.format_cpp_type(ft['element'])
                # Avoid <: digraph
                if ktype.startswith(':'):
                    ktype = ' ' + ktype
                return 'std::map<{}, {}>'.format(ktype, etype)
            else:
                raise RuntimeError('Unsupported fieldType {}'.format(ft['type']))
        return self.get_basic_type_info(ft)['c++type']

    def format_cpp_bt_const(self, t):
        """Return C++ code denoting BT_xxx type constant for specified type"""
        if type(t) is dict:
            if t['type'] == 'user':
                name = 'BT_' + t['declaration']['tag'].upper()
            elif t['type'] == 'vector':
                name = 'BT_LIST'
            elif t['type'] == 'map':
                name = 'BT_MAP'
            else:
                name = 'BT_' + t['tag'].upper()
        elif not t.startswith('BT_'):
            name = 'BT_' + t.upper()
        else:
            name = t
        return '{}'.format(name)

    def format_cpp_full_name(self, struct, relativeTo=None):
        """Return C++ code denoting the fully-qualified name of specified user type"""
        ns = struct['declNamespaces'][0]['name']
        skip = 0
        if relativeTo is not None:
            refNs = relativeTo['declNamespaces'][0]['name']
            while skip < len(ns) and skip < len(refNs) and ns[skip] == refNs[skip]:
                skip += 1
        parts = []
        if skip == 0:
            parts.append('')
        parts.extend(ns[skip:])
        parts.append(struct['declName'])
        return '::'.join(parts)

    def format_cpp_field_default(self, f):
        """Return C++ code denoting the default value for specified field"""
        fd = f['fieldDefault']
        if fd is None:
            ft = f['fieldType']
            if type(ft) is not dict:
                bti = self.get_basic_type_info(ft)
                if 'def' in bti:
                    return bti['def']
            return None
        elif fd['type'] == 'integer':
            return str(fd['value'])
        elif fd['type'] == 'float':
            return str(fd['value'])
        elif fd['type'] == 'enum':
            return '{}::{}'.format(self.format_cpp_full_name(f['fieldType']['declaration']), fd['value'])
        else:
            raise RuntimeError('Unsupported fieldDefault {}'.format(fd))

    def wl(self, indent, str, *args):
        """Write a formatted line with requested indentation to the output file"""
        if str == '':
            # Avoid trailing whitespace on empty lines
            self.output_file.write('\n')
        else:
            self.output_file.write(' ' * 4 * indent + str.format(*args) + '\n')

    def write_header(self, input_filename, includes):
        """Write the auto-generated C++ banner and common include lines to specified file"""
        self.wl(0, '//------------------------------------------------------------------------------')
        self.wl(0, '// This code was generated by a tool.')
        self.wl(0, '//')
        self.wl(0, '//   Tool : bondjson2cpp {}'.format(__version__))
        self.wl(0, '//   File : {}'.format(input_filename))
        self.wl(0, '//')
        self.wl(0, '// Changes to this file may cause incorrect behavior and will be lost when')
        self.wl(0, '// the code is regenerated.')
        self.wl(0, '// <auto-generated />')
        self.wl(0, '//------------------------------------------------------------------------------')
        self.wl(0, '')
        self.wl(0, '#pragma once')
        for include in includes:
            self.wl(0, '#include {}', include)

    def write_types(self, declarations):
        """Convert all types declared in the input to C++ and write the output to specified file"""
        cur_ns = []
        for d in declarations:
            if len(d['declNamespaces']) != 1:
                raise RuntimeError('More than one entry in declNamespaces')
            ns = d['declNamespaces'][0]['name']
            while len(cur_ns) > len(ns) or (len(cur_ns) > 0 and cur_ns[-1] != ns[-1]):
                self.wl(0, '}} // namespace {}', cur_ns.pop())
            if ns != cur_ns:
                self.wl(0, '')
                while len(ns) > len(cur_ns):
                    cur_ns.append(ns[len(cur_ns)])
                    self.wl(0, 'namespace {} {{', cur_ns[-1])
                self.wl(0, '')

            if d['tag'] == 'Struct':
                if len(d['declParams']) != 0:
                    raise RuntimeError('Struct with declParams that contain something')
                if len(d['declAttributes']) != 0:
                    raise RuntimeError('Struct with declAttributes that contain something')
                if d['structBase'] is not None:
                    raise RuntimeError('Struct with structBase that is not null')
                self.wl(0, 'struct {} {{', d['declName'])

                # Write fields, possibly with default values (C++11 initialization)
                for f in d['structFields']:
                    self.wl(1, '// {}: {} {} {}', f['fieldOrdinal'], f['fieldModifier'].lower(), self.format_bond_type(f['fieldType']), f['fieldName'])
                    fd = self.format_cpp_field_default(f)
                    if fd is None:
                        self.wl(1, '{} {};', self.format_cpp_type(f['fieldType']), f['fieldName'])
                    else:
                        self.wl(1, '{} {} = {};', self.format_cpp_type(f['fieldType']), f['fieldName'], fd)

                # Default constructor, copy constructor, move constructor, assignment operator,
                # move-assignment operator and destructor are created automatically by default.

                # operator==
                self.wl(1, '')
                self.wl(1, 'bool operator==({} const& other) const', d['declName'])
                self.wl(1, '{{')
                for i, f in enumerate(d['structFields']):
                    self.wl(2, '{0} ({1} == other.{1}){2}', 'return' if i == 0 else '    &&', f['fieldName'], ';' if i == len(d['structFields']) - 1 else '')
                self.wl(1, '}}')

                # operator!=
                self.wl(1, '')
                self.wl(1, 'bool operator!=({} const& other) const', d['declName'])
                self.wl(1, '{{')
                self.wl(2, 'return !(*this == other);')
                self.wl(1, '}}')

                # The Boost-Bond also generated swap() method and external swap() function.
                # Skipping that for now as C++11 default std::swap() uses move semantics
                # and in the end is arguably as good as a dedicated swap implementation.

                self.wl(0, '}};')
                self.wl(0, '')

            elif d['tag'] == 'Enum':
                if len(d['declAttributes']) != 0:
                    raise RuntimeError('Enum and declAttributes contain something')
                # Using the same trick as the original Boost-Bond to not leak all enum
                # member symbols to the parent namespace.
                self.wl(0, 'namespace _bond_enumerators {{')
                self.wl(0, 'namespace {} {{', d['declName'])
                self.wl(0, 'enum {} {{', d['declName'])
                max_len = max(map(lambda c: len(c['constantName']), d['enumConstants']))
                for i, c in enumerate(d['enumConstants']):
                    self.wl(1, '{:{}} = {}{}', c['constantName'], max_len, c['constantValue'], ',' if i < len(d['enumConstants']) - 1 else '')
                self.wl(0, '}};')
                self.wl(0, '}}')
                self.wl(0, '}}')
                self.wl(0, 'using namespace _bond_enumerators::{};', d['declName'])
                self.wl(0, '')

            else:
                raise RuntimeError('Unsupported tag {}'.format(d['tag']))

        while len(cur_ns) > 0:
            self.wl(0, '}} // namespace {}', cur_ns.pop())

    def write_item_writer(self, ft, var, indent):
        """Construct C++ code for writing specified variable and write it to the output file"""
        if type(ft) is dict:
            if ft['type'] == 'user':
                if ft['declaration']['tag'] == 'Enum':
                    # The assumption that the enum is actually INT32 is tested with
                    # a static_assert generated in write_field_writer() below.
                    self.wl(indent, 'writer.WriteInt32(static_cast<int32_t>({}));', var)
                else:
                    self.wl(indent, 'Serialize(writer, {}, false);', var)
            elif ft['type'] == 'vector':
                self.wl(indent, 'writer.WriteContainerBegin({}.size(), {});', var, self.format_cpp_bt_const(ft['element']))
                self.wl(indent, 'for (auto const& item{} : {}) {{', indent, var)
                self.write_item_writer(ft['element'], 'item{}'.format(indent), indent + 1)
                self.wl(indent, '}}')
                self.wl(indent, 'writer.WriteContainerEnd();')
            elif ft['type'] == 'map':
                self.wl(indent, 'writer.WriteMapContainerBegin({}.size(), {}, {});', var, self.format_cpp_bt_const(ft['key']), self.format_cpp_bt_const(ft['element']))
                self.wl(indent, 'for (auto const& item{} : {}) {{', indent, var)
                self.write_item_writer(ft['key'], 'item{}.first'.format(indent), indent + 1)
                self.write_item_writer(ft['element'], 'item{}.second'.format(indent), indent + 1)
                self.wl(indent, '}}')
                self.wl(indent, 'writer.WriteContainerEnd();')
            else:
                raise RuntimeError('Unsupported fieldType {}'.format(ft['type']))
        else:
            self.wl(indent, 'writer.Write{}({});', self.get_basic_type_info(ft)['writer'], var)

    def write_field_writer(self, f, var, indent):
        """Construct C++ code for writing specified field and write it to the output file"""
        ft = f['fieldType']

        if type(ft) is dict and not (ft['type'] == 'user' and ft['declaration']['tag'] == 'Enum'):
            # Composed types
            if ft['type'] == 'user':
                self.wl(indent, 'writer.WriteFieldBegin({}, {}, nullptr);', self.format_cpp_bt_const(ft), f['fieldOrdinal'])
                self.write_item_writer(ft, var, indent)
                self.wl(indent, 'writer.WriteFieldEnd();')
            elif ft['type'] == 'vector':
                self.wl(indent, 'if (!{}.empty()) {{', var)
                self.wl(indent + 1, 'writer.WriteFieldBegin({}, {}, nullptr);', self.format_cpp_bt_const(ft), f['fieldOrdinal'])
                self.write_item_writer(ft, var, indent + 1)
                self.wl(indent + 1, 'writer.WriteFieldEnd();')
                self.wl(indent, '}} else {{')
                self.wl(indent + 1, 'writer.WriteFieldOmitted({}, {}, nullptr);', self.format_cpp_bt_const(ft), f['fieldOrdinal'])
                self.wl(indent, '}}')
            elif ft['type'] == 'map':
                self.wl(indent, 'if (!{}.empty()) {{', var)
                self.wl(indent + 1, 'writer.WriteFieldBegin({}, {}, nullptr);', self.format_cpp_bt_const(ft), f['fieldOrdinal'])
                self.write_item_writer(ft, var, indent + 1)
                self.wl(indent + 1, 'writer.WriteFieldEnd();')
                self.wl(indent, '}} else {{')
                self.wl(indent + 1, 'writer.WriteFieldOmitted({}, {}, nullptr);', self.format_cpp_bt_const(ft), f['fieldOrdinal'])
                self.wl(indent, '}}')
            else:
                raise RuntimeError('Unsupported fieldType {}'.format(ft['type']))
            return

        # Plain types
        if type(ft) is dict:
            self.wl(indent, 'static_assert(sizeof({}) == 4, "Invalid size of enum");', var)
            bt = self.format_cpp_bt_const('BT_INT32')
        else:
            bt = self.format_cpp_bt_const(ft)
        fd = self.format_cpp_field_default(f)
        if fd is not None:
            isset = '{} != {}'.format(var, fd)
        else:
            isset = self.get_basic_type_info(ft)['isset'].format(var)
        self.wl(indent, 'if ({}) {{', isset)
        self.wl(indent + 1, 'writer.WriteFieldBegin({}, {}, nullptr);', bt, f['fieldOrdinal'])
        self.write_item_writer(ft, var, indent + 1)
        self.wl(indent + 1, 'writer.WriteFieldEnd();')
        self.wl(indent, '}} else {{')
        self.wl(indent + 1, 'writer.WriteFieldOmitted({}, {}, nullptr);', bt, f['fieldOrdinal'])
        self.wl(indent, '}}')

    def write_writers(self, declarations):
        """Construct C++ writers for all structures declared in the input and write the output to specified file"""
        self.wl(0, '')
        self.wl(0, 'namespace bond_lite {{')
        self.wl(0, '')

        for d in declarations:
            if d['tag'] != 'Struct':
                continue

            self.wl(0, 'template<typename TWriter>')
            self.wl(0, 'void Serialize(TWriter& writer, {} const& value, bool isBase)', self.format_cpp_full_name(d))
            self.wl(0, '{{')
            self.wl(1, 'writer.WriteStructBegin(nullptr, isBase);')
            self.wl(1, '')

            for f in d['structFields']:
                self.write_field_writer(f, 'value.{}'.format(f['fieldName']), 1)
                self.wl(1, '')

            self.wl(1, 'writer.WriteStructEnd(isBase);')
            self.wl(0, '}}')
            self.wl(0, '')

        self.wl(0, '}} // namespace bond_lite')

    def write_item_reader(self, ft, var, indent):
        """Construct C++ code for reading specified serialized variable and write it to the output file"""
        if type(ft) is dict:
            if ft['type'] == 'user':
                if ft['declaration']['tag'] == 'Enum':
                    # The assumption that the enum is actually INT32 is tested with
                    # a static_assert generated in write_field_reader() below.
                    self.wl(indent, 'int32_t item{};', indent)
                    self.wl(indent, 'if (!reader.ReadInt32(item{})) {{', indent)
                    self.wl(indent + 1, 'return false;')
                    self.wl(indent, '}}')
                    etype = self.format_cpp_type(ft)
                    # Avoid <: digraph
                    if etype.startswith(':'):
                        etype = ' ' + etype
                    self.wl(indent, '{} = static_cast<{}>(item{});', var, etype, indent)
                else:
                    self.wl(indent, 'if (!Deserialize(reader, {}, false)) {{', var)
                    self.wl(indent + 1, 'return false;')
                    self.wl(indent, '}}')
            elif ft['type'] == 'vector':
                self.wl(indent, 'uint32_t size{};', indent)
                self.wl(indent, 'uint8_t type{};', indent)
                self.wl(indent, 'if (!reader.ReadContainerBegin(size{0}, type{0})) {{', indent)
                self.wl(indent + 1, 'return false;')
                self.wl(indent, '}}')
                self.wl(indent, 'if (type{} != {}) {{', indent, self.format_cpp_bt_const(ft['element']))
                self.wl(indent + 1, 'return false;')
                self.wl(indent, '}}')
                self.wl(indent, '{}.resize(size{});', var, indent)
                self.wl(indent, 'for (unsigned i{0} = 0; i{0} < size{0}; i{0}++) {{', indent)
                self.write_item_reader(ft['element'], '{}[i{}]'.format(var, indent), indent + 1)
                self.wl(indent, '}}')
                self.wl(indent, 'if (!reader.ReadContainerEnd()) {{')
                self.wl(indent + 1, 'return false;')
                self.wl(indent, '}}')
            elif ft['type'] == 'map':
                self.wl(indent, 'uint32_t size{};', indent)
                self.wl(indent, 'uint8_t keyType{}, valueType{};', indent, indent)
                self.wl(indent, 'if (!reader.ReadMapContainerBegin(size{0}, keyType{0}, valueType{0})) {{', indent)
                self.wl(indent + 1, 'return false;')
                self.wl(indent, '}}')
                self.wl(indent, 'if (keyType{0} != {1} || valueType{0} != {2}) {{', indent, self.format_cpp_bt_const(ft['key']), self.format_cpp_bt_const(ft['element']))
                self.wl(indent + 1, 'return false;')
                self.wl(indent, '}}')
                self.wl(indent, 'for (unsigned i{0} = 0; i{0} < size{0}; i{0}++) {{', indent)
                self.wl(indent + 1, '{} key{};', self.format_cpp_type(ft['key']), indent)
                self.write_item_reader(ft['key'], 'key{}'.format(indent), indent + 1)
                self.write_item_reader(ft['element'], '{}[key{}]'.format(var, indent), indent + 1)
                self.wl(indent, '}}')
                self.wl(indent, 'if (!reader.ReadContainerEnd()) {{')
                self.wl(indent + 1, 'return false;')
                self.wl(indent, '}}')
            else:
                raise RuntimeError('Unsupported fieldType {}'.format(ft['type']))
        else:
            self.wl(indent, 'if (!reader.Read{}({})) {{', self.get_basic_type_info(ft)['writer'], var)
            self.wl(indent + 1, 'return false;')
            self.wl(indent, '}}')

    def write_field_reader(self, f, var, indent):
        """Construct C++ code for reading specified serialized field and write it to the output file"""
        ft = f['fieldType']

        if type(ft) is dict and not (ft['type'] == 'user' and ft['declaration']['tag'] == 'Enum'):
            # Composed types
            if ft['type'] == 'user':
                self.write_item_reader(ft, var, indent)
            elif ft['type'] == 'vector':
                self.write_item_reader(ft, var, indent)
            elif ft['type'] == 'map':
                self.write_item_reader(ft, var, indent)
            else:
                raise RuntimeError('Unsupported fieldType {}'.format(ft['type']))
            return

        # Plain types
        if type(ft) is dict:
            self.wl(indent, 'static_assert(sizeof({}) == 4, "Invalid size of enum");', var)
            bt = self.format_cpp_bt_const('BT_INT32')
        else:
            bt = self.format_cpp_bt_const(ft)
        self.write_item_reader(ft, var, indent)

    def write_readers(self, declarations):
        """Construct C++ readers for all structures declared in the input and write the output to specified file"""
        self.wl(0, '')
        self.wl(0, 'namespace bond_lite {{')
        self.wl(0, '')

        for d in declarations:
            if d['tag'] != 'Struct':
                continue

            self.wl(0, 'template<typename TReader>')
            self.wl(0, 'bool Deserialize(TReader& reader, {}& value, bool isBase)', self.format_cpp_full_name(d))
            self.wl(0, '{{')
            self.wl(1, 'if (!reader.ReadStructBegin(isBase)) {{')
            self.wl(2, 'return false;')
            self.wl(1, '}}')
            self.wl(1, '')

            self.wl(1, 'uint8_t type;')
            self.wl(1, 'uint16_t id;')
            self.wl(1, 'for (;;) {{')
            self.wl(2, 'if (!reader.ReadFieldBegin(type, id)) {{')
            self.wl(3, 'return false;')
            self.wl(2, '}}')
            self.wl(2, '')

            self.wl(2, 'if (type == {} || type == {}) {{', self.format_cpp_bt_const('BT_STOP'), self.format_cpp_bt_const('BT_STOP_BASE'))
            self.wl(3, 'if (isBase != (type == {})) {{', self.format_cpp_bt_const('BT_STOP_BASE'))
            self.wl(4, 'return false;')
            self.wl(3, '}}')
            self.wl(3, 'break;')
            self.wl(2, '}}')
            self.wl(2, '')

            self.wl(2, 'switch (id) {{')
            for f in d['structFields']:
                self.wl(3, 'case {}: {{', f['fieldOrdinal'])
                self.write_field_reader(f, 'value.{}'.format(f['fieldName']), 4)
                self.wl(4, 'break;')
                self.wl(3, '}}')
                self.wl(3, '')
            self.wl(3, 'default:')
            self.wl(4, 'return false;')
            self.wl(2, '}}')
            self.wl(2, '')

            self.wl(2, 'if (!reader.ReadFieldEnd()) {{')
            self.wl(3, 'return false;')
            self.wl(2, '}}')
            self.wl(1, '}}')
            self.wl(1, '')

            self.wl(1, 'if (!reader.ReadStructEnd(isBase)) {{')
            self.wl(2, 'return false;')
            self.wl(1, '}}')
            self.wl(1, '')

            self.wl(1, 'return true;')
            self.wl(0, '}}')
            self.wl(0, '')

        self.wl(0, '}} // namespace bond_lite')

    def process(self, input_filename):
        """Process one input JSON schema file and create output C++ files"""

        print(input_filename)
        try:
            with open(input_filename, 'r') as f:
                input_json = json.load(f)
        except Exception as e:
            print('Error loading input file: {}'.format(e))
            return

        declarations = sorted(input_json['declarations'], key=lambda x: x['declNamespaces'])
        input_noext = os.path.splitext(input_filename)[0]

        print('    Creating {}_types.hpp...'.format(input_noext))
        with open(input_noext + '_types.hpp', 'w') as f:
            self.output_file = f
            self.write_header(input_filename, ('<cstdint>', '<string>', '<vector>', '<map>'))
            self.write_types(declarations)
            self.output_file = None

        print('    Creating {}_writers.hpp...'.format(input_noext))
        with open(input_noext + '_writers.hpp', 'w') as f:
            self.output_file = f
            self.write_header(input_filename, ('"BondConstTypes.hpp"',))
            self.write_writers(declarations)
            self.output_file = None

        print('    Creating {}_readers.hpp...'.format(input_noext))
        with open(input_noext + '_readers.hpp', 'w') as f:
            self.output_file = f
            self.write_header(input_filename, ('"BondConstTypes.hpp"',))
            self.write_readers(declarations)
            self.output_file = None

        print('    Done.')
        print('')


def main():
    print('-*- bondjson2cpp - version {} -*-\n'.format(__version__))

    parser = argparse.ArgumentParser(
        description='Convert Bond JSON schema to C++ headers for use with Bond lite serializer.',
        epilog='For each input Schema.json, output files Schema_types.hpp, Schema_writers.hpp and ' +
               'Schema_readers.hpp will be created (or overwritten). bond_const.json (generated from ' +
               'bond_const.bond) has to be in the current directory too.'
    )
    parser.add_argument('-v', '--version', action='version', version='%(prog)s ' + __version__)
    parser.add_argument('input', metavar='Schema.json', type=str, nargs='+',
                        help='Bond JSON schema, generated by `gbc schema ...`')
    args = parser.parse_args()

    processor = BondJson2Cpp()
    if not processor.process_bond_data_types():
        exit(1)

    for f in args.input:
        processor.process(f)


if __name__ == '__main__':
    main()
