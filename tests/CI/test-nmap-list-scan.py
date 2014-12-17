#!/usr/bin/python

"""
Test the netmon -> nmap scanner -> db integration and logic (filip) integration.

 Description:
1.) Check the ip addresses of machine from DB
2.) Load and parse XML of nmap
3.) Compare with results of driver-nmap from DB

 Expects:
1.) database empty before start
2.) main daemon running with environment variable BIOS_DEBUG=1 to dump nmap xml output to disk
3.) netmon running
4.) nmap driver running

Usage:
    test-nmap-list-scan.py path/to/main/daemon's/cwd
"""

from __future__ import print_function

import time
import io
import collections
import xml.etree.ElementTree as ET
import sys
import os.path
import glob

from shared import connect_to_db

def parse_host_element(node):
    """
Get tuple (ip-address, {hostname: type}) for each host - if there is no hostname
element, return (None, None)
    """
    assert node.tag == "host"

    address = node.find('address')
    assert address != None
    hostnames = node.find('hostnames')
    if hostnames is None:
        return (None, None)
    return  (
            address.get('addr'),
            {h.get("name") : h.get("type") for h in hostnames}
            )

def parse_list_scan(xml):
    """
Get list of tuples (ip-address, {hostname: type}) for each host - if there is no hostname
element, return (None, None)

@param path with xml file or file object
    """
    root = ET.parse(xml).getroot()
    result = list()
    for node in root:
        if node.tag == "host":
            res = parse_host_element(node)
            if res != (None, None):
                result.append(res)
    return result

def list_scanx(files):
    """
    return (ip-address, {hostname: type}) for all ipaddresses
    """
    result = list()
    for path in files:
        with open(path, "rt") as f:
            result.extend(parse_list_scan(f))
    return result

def db_read_discovered_ip(db):
    """
    read content from db
    
    return - list of tuple (ipaddr)
    """

    assert(db is not None)
    ret = list()
    cur = db.cursor()

    try:
        cur.execute("SELECT ip FROM v_bios_discovered_ip")
        ret = [ip[0] for ip in cur.fetchall()]
    except Exception as e:
        raise e
    finally:
        cur.close()
        db.commit()

    return ret

def db_read_ip_to_scan(db):
    """
    read content from db
    
    return - list ipaddr/mask
    """

    assert(db is not None)
    ret = list()
    cur = db.cursor()

    try:
        cur.execute("SELECT ip, mask FROM v_bios_net_history WHERE command = 'a'")
        ret = ["%s/%s" % (ip, mask) for ip, mask in cur.fetchall() if "." in ip]
    except Exception as e:
        raise e
    finally:
        cur.close()
        db.commit()

    return ret

ListScanDiff = collections.namedtuple("ListScanDiff", "nmap, db")
def compare(nmap, dbres):
    """
    compare results from nmap and database
    """
    nmapset = {ip for ip, hostnames in nmap}
    dbresset= {ip for ip in dbres}

    nmapd = nmapset.difference(dbresset)
    dbd   = dbresset.difference(nmapset)

    return ListScanDiff(nmapd, dbd)

###

# parse sys.argv
try:
    DAEMON_CWD=sys.argv[1]
except IndexError:
    print("FATAL: expects path to deamon's working directory", file=sys.stderr)
    sys.exit(1)

if not os.path.isdir(DAEMON_CWD):
    print("FATAL: given daemon dir '%s' does not exists or not a directory" % DAEMON_CWD, file=sys.stderr)
    sys.exit(1)

db = connect_to_db()
networks = []
for i in range(10):
    networks = db_read_ip_to_scan(db)
    if networks == []:
        print("[INFO]: trying to read list of networks from database, sleep %d seconds" % (i+1), file=sys.stderr)
        time.sleep(i+1)
        continue
    break

assert networks != [], "01-check-networks: FATAL nothing in v_bios_net_history found, is netmon running?"

netfiles = []
for i in range(10):
    netfiles = glob.glob(DAEMON_CWD + "/nmap-defaultlistscan-*.xml")
    if len(netfiles) == len(networks):
        break
    print("[INFO]: trying to read dump files from driver-nmap" % (i+1), file=sys.stderr)
    time.sleep(i+1)

assert len(netfiles) == len(networks), "02-check-netfiles: FATAL length of network does not match found files, did you use BIOS_DEBUG=1?"

nmapres = list_scanx(netfiles)

dbres = db_read_discovered_ip(db)
ldf = compare(nmapres, dbres)
assert ldf == (set(), set()), "03-list-scan-compare: output of db and nmap does not match, '%s'" % repr(ldf)
