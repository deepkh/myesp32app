#!/usr/bin/env python3
#
# MIT License
#
# Copyright (c) 2026 Gary Huang (deepkh@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction...

import sys
import yaml
import os

from pathlib import Path

MAX_STR_LEN = 64

AUTOGEN_DECLARATION="""// DO NOT EDIT THE FILE MANUALLY. 
// THE FILE IS GENERATED AUTOMATICLY BY EspConfigGenerator.py
"""

def infer_type(value):
    """Infer C++ type based on the Python value."""
    if isinstance(value, int):
        return "int"
    elif isinstance(value, str):
        return f"char"
    else:
        return None  # Nested struct

def struct_name_from_key(key):
    """Convert YAML key to a C++ struct name (capitalize words)."""
    return ''.join([w.capitalize() for w in key.split('_')])

def generate_structs(data, structs):
    """
    Recursively generate struct definitions.
    structs: dict of struct_name -> {field_name: field_type or nested_struct_name}
    """
    for key, value in data.items():
        if isinstance(value, dict):
            struct_fields = {}
            for subkey, subval in value.items():
                if isinstance(subval, dict):
                    nested_struct_name = struct_name_from_key(subkey)
                    generate_structs({subkey: subval}, structs)
                    struct_fields[subkey] = nested_struct_name
                else:
                    struct_fields[subkey] = infer_type(subval)
            struct_name = struct_name_from_key(key)
            structs[struct_name] = struct_fields
        else:
            # top-level scalar? (unlikely)
            pass

def write_header_file(structs, top_struct_name, header_path):
    with open(header_path, 'w') as f:
        f.write(AUTOGEN_DECLARATION)
        f.write("#pragma once\n\n")
        f.write("namespace MyEsp { \n")
        for struct_name, fields in structs.items():
            f.write(f"struct {struct_name} {{\n")
            for fname, ftype in fields.items():
                if ftype == "char":
                    f.write(f"  {ftype} {fname}[{MAX_STR_LEN}];\n")
                else: 
                    f.write(f"  {ftype} {fname};\n")
            f.write("};\n")
        f.write("}\n\n")
        f.write(f"extern const MyEsp::{top_struct_name} g_{top_struct_name.lower()};\n")

def write_cpp_file(structs, top_struct_name, data, cpp_path, header_path):
    """Generate the C++ initialization code."""
    def format_value(value):
        if isinstance(value, str):
            if "ESPCONFIG_PRIVATE" in value:
                return f'{value}'
            else:
                return f'"{value}"'
        else:
            return str(value)

    def generate_init(struct_name, value_dict, level):
        lines = []
        fields = structs[struct_name]
        intents = ' ' * ( level * 2 )
        for fname, ftype in fields.items():
            val = value_dict[fname]
            if ftype in structs:  # nested struct
                nested_init = generate_init(ftype, val, level + 1)
                lines.append(f"\n{intents}//.{fname} = ")
                lines.append(f"\n{intents}{{")
                lines.append(f"{nested_init}")
                lines.append(f" \n{intents}}},")
            else:
                lines.append(f"\n{intents}/*.{fname} = */ {format_value(val)},")
        return "".join(lines)
        #return ', '.join(lines)

    with open(cpp_path, 'w') as f:
        f.write(AUTOGEN_DECLARATION)
        f.write(f"#include \"{Path(header_path).name}\"\n")
        f.write(f"#include \"EspConfigPrivate.h\"\n\n")
        f.write(f"const MyEsp::{top_struct_name} g_{top_struct_name.lower()} = {{")
        top_key = list(data.keys())[0]
        init_lines = generate_init(top_struct_name, data[top_key], 1)
        # indent
        f.write('  ' + init_lines+ '\n')
        f.write("};\n")

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 HeaderCppGenerator.py Defination.yml Output.h Output.cpp")
        sys.exit(1)

    yaml_file = sys.argv[1]
    header_file = sys.argv[2]
    cpp_file = sys.argv[3]

    if not os.path.exists(yaml_file):
        print(f"Error: YAML file {yaml_file} does not exist.")
        sys.exit(1)

    with open(yaml_file, 'r') as f:
        data = yaml.safe_load(f)
    

    top_key = list(data.keys())[0]
    top_struct_name = struct_name_from_key(top_key)

    structs = {}
    generate_structs(data, structs)

    write_header_file(structs, top_struct_name, header_file)
    write_cpp_file(structs, top_struct_name, data, cpp_file, header_file)
    print(f"Generated {header_file} and {cpp_file} successfully.")

if __name__ == "__main__":
    main()
