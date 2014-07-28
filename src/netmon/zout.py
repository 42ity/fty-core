#!/usr/bin/python

# Test the zeromq messaging for netmon
#
# 1.) Create an internal ZMQ bus and bind there
# 2.) Setup ZMQ_BUS environment for communication
# 3.) Wait on messages and print them on stdout

import zmq
import subprocess
import atexit
import time

import json

NETMON="../../netmon"
#XXX: to be changed!!
ZMQ_BUS="ipc:///tmp/netmon-bus.ipc"

ctx = zmq.Context()
responder = ctx.socket(zmq.REP)
responder.bind(ZMQ_BUS)

netmon = subprocess.Popen([NETMON, ], env={"ZMQ_BUS" : ZMQ_BUS})

def kill_proc(proc):
    proc.kill()

atexit.register(kill_proc, netmon)

while True:
    response = responder.recv()
    print('ZOUT: got ``%s'' with len %d' % (response, len(response)))
    obj = json.loads(response)
    print('ZOUT: and parsable as JSON!!!')
    responder.send(b'')

