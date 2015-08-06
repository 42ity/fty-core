#!/usr/bin/env python
from __future__ import print_function

__doc__ = """
Compare REST API calls output with expected ones - it assumes one JSON document per line to support more results in one file!

Usage: python cmpjson.py file1 file2
"""

import json
import sys

def self_test():
    jsonstr1 = """{"current":[{"id":3,"realpower.1":1,"voltage.2":1,"current.2":12,"current.1":31,"voltage.1":3}]}"""
    jsonstr2 = """{"current":[{"id":3,"realpower.1":1,"current.2":12,"current.1":31,"voltage.2":1,"voltage.1":3}]}"""

    json1 = json.loads(jsonstr1)
    json2 = json.loads(jsonstr2)

    assert json1 == json2, "json1 should equals to json2"
    
    jsonstr3 = """{"current":[{"id":3,"realpower.1":1,"current.2":12,"current.1":31,"voltage.2":1,"voltage.2":3}]}"""
    json3 = json.loads(jsonstr3)
    assert json1 != json3, "json1 should NOT equald to json3"

def load_json(f):
    """results are not valid json documents, but each line contains one - thus return a list of object for each line"""
    return [json.loads(l) for l in f]

if len(sys.argv) <= 1:
    self_test()
    sys.exit(0)

if len(sys.argv) != 3:
    print(__doc__)
    sys.exit(1)

with open(sys.argv[1]) as f:
    output = load_json(f)
with open(sys.argv[2]) as f:
    result = load_json(f)

if output != result:
    sys.exit(1)
sys.exit(0)
