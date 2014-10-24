#!/usr/bin/python

"""
Test the netmon -> zeromq -> db integration.

1.) Check if initial state is the same in db and what ip addr show reports
2.) Turn off the first interface off
3.) Check the status again
4.) Turn it not
5.) Check it again
"""

import subprocess
import time
import re
import MySQLdb
import sys

def parse_ip_a_s(out):
    """
    parse output of command ip a(ddr) s(how)
    
    return - list of tuple (name, ipver, ipaddr, prefixlen: int, mac)
    """

    state = "start"
    header = list()
    ret = list()

    for line in out.split('\n'):

        if line and line[0] != ' ':
            state = "start"
            header = list()
            nic_name = line.split(':')[1].strip()
            if nic_name == 'lo':
                continue
            header.append(nic_name)
            state = "read-mac"
            continue

        # skip entries for 'lo'
        if state == "start" and not header:
            continue

        if line.startswith("    link/"):
            try:
                mac = line.split()[1]
            except IndexError as e:
                print(line)
                print(e)
            header.append(mac)
        elif line.startswith("    inet"):
            name, mac = header
            ipver, ipaddr = line.split()[:2]
            ipaddr, prefixlen = ipaddr.split('/')

            ret.append((name, ipver, ipaddr, int(prefixlen), mac))
    return ret

# why?
def decode_mac(mac_i):
    """Decode DB number back to string ..."""

    mac = ""
    for i in range(6):
        mac = "%x:%s" % (0xff & mac_i, mac)
        mac_i = mac_i >> 8
    return mac[:-1]

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
        ret = [(name, "inet" if "." in ip else "inet6", ip, mask, decode_mac(mac)) for 
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
    """using O(n^2) algorithm compare results from ip a s and database"""
    if len(ipres) != len(dbres):
        return False

    for ipentry in ipres:
        found = False
        for dbentry in dbres:
            if compare_entries(ipentry, dbentry):
                found = True
                break
        if not found:
            return False

    return True

#### fixture ini ####

# restart simple
subprocess.call(["/usr/bin/killall", "-9", "simple", "netmon"])
# restart mysql daemon
ret = subprocess.call(["/usr/bin/sudo", "/bin/systemctl", "restart", "mysql.service"])
assert(ret == 0), "Can't run mysql database, skipping"
simple_proc = subprocess.Popen(["./simple"])
time.sleep(1)
ret = simple_proc.poll()
assert(ret is None), "./simple does not run, skipping"

time.sleep(3) # to give things enough time to start


#### MAIN ####
ipout = subprocess.check_output(["/bin/ip", "a", "s"])
ipres = parse_ip_a_s(ipout)

assert len(ipres) > 0, "TODO: move to skip - there is nothing to test on this box"

db = MySQLdb.connect(host="localhost", user="root", db="box_utf8")
dbres = read_db(db)

assert compare_results(ipres, dbres), "results of ip addr show is not equal with database results"

nic_name = ipres[0][0]
assert nic_name, "Name of network card is empty!"

subprocess.check_call(["/usr/bin/sudo", "/bin/ip", "link", "set", nic_name, "down"])
time.sleep(10)

db.commit() #WTF WTF
dbres = read_db(db)
assert not compare_results(ipres, dbres), "%s got changed, ipres should not be equal to dbres" % nic_name

ipout = subprocess.check_output(["/bin/ip", "a", "s"])
ipres = parse_ip_a_s(ipout)
assert compare_results(ipres, dbres), "results of ip addr show is not equal with database results"

subprocess.check_call(["/usr/bin/sudo", "/bin/ip", "link", "set", nic_name, "up"])
time.sleep(10)

db.commit()
dbres = read_db(db)
assert not compare_results(ipres, dbres), "%s got changed, ipres should not be equal to dbres" % nic_name

ipout = subprocess.check_output(["/bin/ip", "a", "s"])
ipres = parse_ip_a_s(ipout)
assert compare_results(ipres, dbres), "results of ip addr show is not equal with database results"

#### fixture fini ######

simple_proc.kill()
time.sleep(0.5)
simple_proc.terminate()
