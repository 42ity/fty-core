#!/usr/bin/python

"""
Test the netmon -> zeromq -> db integration.

1.) Check if initial state is the same in db and what ip addr show reports
2.) Turn off the first interface off
3.) Check the status again
4.) Turn it not
5.) Check it again

 Expects:
1.) database empty before start
2.) main daemon running
3.) netmon running
"""

import subprocess
import time
import re
import sys
import copy

from shared import parse_ip_a_s, connect_to_db

def read_db(db):
    """
    read content from db
    
    return - list of tuple (name, ipver, ipaddr, prefixlen: int, mac)
    """

    assert(db is not None)
    ret = list()
    cur = db.cursor()

    try:
        cur.execute("SELECT name, ip, mask, mac FROM v_bios_net_history WHERE command = 'a'")
        ret = [(name, "inet" if "." in ip else "inet6", ip, mask, mac.lower()) for 
            name, ip, mask, mac in cur.fetchall()]
    except Exception as e:
        raise e
    finally:
        cur.close()
        db.commit()

    return ret

def ip_compare(ipi, ipd):
    """netmon does not do ANY normalization using libcidr, so it's ipaddr is ip
    address of a host. DB on the other hand do. This is lazy man's approach -
    to return/check number of common characters in a string."""

    r = 0
    for c1, c2 in zip(ipi, ipd):
        if c1 != c2:
            return r
        r += 1

    return r

def compare_entries(ipentry, dbentry):
    """return if two tuples are equal or not"""
    namei, ipveri, ipi, maski, maci = ipentry
    named, ipverd, ipd, maskd, macd = dbentry

    return namei == named  and\
            ipveri == ipverd and \
            ip_compare(ipi, ipd) >= 2 and \
            maski == maskd and \
            maci == macd
    
def compare_results(ipres, dbres):
    """using O(n^2) algorithm compare results from ip a s and database, return the diff between those two"""

    ret = copy.copy(ipres)
    ret.extend(copy.copy(dbres))

    for ipentry in ipres:
        for dbentry in dbres:
            if compare_entries(ipentry, dbentry):
                if ipentry in ret:
                    ret.remove(ipentry)
                if dbentry in ret:
                    ret.remove(dbentry)

    return ret


#### fixture ini ####

# check all deamons running
for daemon in ("simple", "netmon", "mysqld"):
    ret = subprocess.call(["/bin/pidof", daemon])
    assert (ret == 0), "%s does not running!" % (daemon, )

#### MAIN ####
ipout = subprocess.check_output(["/bin/ip", "a", "s"])
ipres = parse_ip_a_s(ipout)

# to create some dummy interface?
nic_name = ipres[0][0]
assert nic_name, "Name of network card is empty!"

assert len(ipres) > 0, "TODO: move to skip - there is nothing to test on this box"

db = connect_to_db()
dbres = read_db(db)
try:
    df = compare_results(ipres, dbres)
    assert df == [], "01-initial-check: results of ip addr SHOULD equals with database results"
except AssertionError as e:
    print(df)
    raise e

subprocess.check_call(["/usr/bin/sudo", "/bin/ip", "addr", "add", "203.0.113.42/24", "dev", nic_name])
time.sleep(4)

db.commit() #WTF WTF
dbres = read_db(db)
try:
    df = compare_results(ipres, dbres)
    assert df != [], "02-added-TEST-NET-3: results of ip addr show SHOULD NOT equals with database results"
except AssertionError as e:
    print(df)
    raise e

ipout = subprocess.check_output(["/bin/ip", "a", "s"])
ipres = parse_ip_a_s(ipout)
try:
    df = compare_results(ipres, dbres)
    assert df == [], "03-added-TEST-NET-3: results of ip addr show SHOULD equals with database results"
except AssertionError as e:
    print(df)
    raise e

subprocess.check_call(["/usr/bin/sudo", "/bin/ip", "addr", "del", "203.0.113.42/24", "dev", nic_name])
time.sleep(3)

db.commit()
dbres = read_db(db)
try:
    df = compare_results(ipres, dbres)
    assert df != [], "04-deleted-TEST-NET-3: results of ip addr show SHOULD NOT equals with database results"
except AssertionError as e:
    print(df)
    raise e

ipout = subprocess.check_output(["/bin/ip", "a", "s"])
ipres = parse_ip_a_s(ipout)
try:
    df = compare_results(ipres, dbres)
    assert df == [], "05-deleted-TEST-NET-3: results of ip addr show SHOULD equals with database results"
except AssertionError as e:
    print(df)
    raise e
