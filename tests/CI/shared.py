# shared routines for CI tests written in python
import MySQLdb

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
            if not "state UP" in line:
                continue
            header = list()
            nic_name = line.split(':')[1].strip()
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

def connect_to_db():
    return MySQLdb.connect(host="localhost", user="root", db="box_utf8")
