#!/usr/bin/env python
import json as _j
import jsonschema as _js

def parse(schema_file,config_file):
    f = open(schema_file,'r')
    schema = _j.load(f)
    f.close()

    f = open(config_file,'r')
    config = _j.load(f)
    f.close()

    _js.validate(config, schema, format_checker=_js.FormatChecker())

if __name__ == '__main__':
    import sys
    config_file  = sys.argv[1]

    parse("../../resources/config_schema.json", config_file)
