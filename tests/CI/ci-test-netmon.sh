#!/bin/sh

# NOTE this script should be run as root

# TODO (nice to have, if there is nothing to do):
# try to grep second iface and do a couple add/del on it as well (lo is special)
# later install brctl and add own iface
# vlan 

dirname=$(dirname $0)
topsrc_dir=$(realpath -e "$dirname")
# For those of us with operating systems that don't have bourne again shell :)
if [ -z "$topsrc_dir" ]; then
    topsrc_dir=$(cd "$dirname" && pwd -P)
    if [ -z "$topsrc_dir" ]; then
        exit 1
    fi
fi
topsrc_dir=${topsrc_dir%tests/CI}

export PATH="$PATH:$topsrc_dir:$topsrc_dir/tools"

killall malamute
dsh_file=$(mktemp -p"$topsrc_dir/tests/CI/")
malamute "$topsrc_dir/tools/malamute.cfg" &
dshell.sh networks ".*" >"$dsh_file" &
netmon &
sleep 2

# These actions have to be reflected in dsh_file for this test to succeed.
ip a a 101.25.138.2 dev lo
ip a a 103.15.3.0/24 dev lo
ip a a 20.13.5.4/32 dev lo

# Reverting back
ip a d 101.25.138.2 dev lo
ip a d 103.15.3.0/24 dev lo
ip a d 20.13.5.4/32 dev lo

file=$(<$dsh_file) # `cat file` for non-bash shell

# Actual dshell output format:
#sender=NETMON subject=add content=
#D: 15-01-21 08:37:04 NETDISC_MSG_AUTO_ADD:
#D: 15-01-21 08:37:04     name='lo'
#D: 15-01-21 08:37:04     ipver=0
#D: 15-01-21 08:37:04     ipaddr='101.25.138.2'
#D: 15-01-21 08:37:04     prefixlen=32
#D: 15-01-21 08:37:04     mac='00:00:00:00:00:00'


# TODO (later, if many regexps are done, we could use ${var/needle/replace} construct)
#re_tmpl=".*sender=NETMON subject=##SUBJECT## content=.*?##MSGTYPE##.*?name='##IFNAME##'.*?ipver=##IPVER##.*?ipaddr='##IPADDR##'.*?prefixlen=##PREFIXLEN##"

# 101.25.138.2 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='101.25.138.2'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    exit 1
fi
re=".*sender=NETMON subject=del content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='101.25.138.2'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    exit 1
fi

# 103.15.3.0/24 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='103.15.3.0'.*?prefixlen=24"
if [[ ! "$file" =~ $re ]]; then
    exit 1
fi
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='103.15.3.0'.*?prefixlen=24"
if [[ ! "$file" =~ $re ]]; then
    exit 1
fi

# 20.13.5.4/32 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='20.13.5.4'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    exit 1
fi
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='20.13.5.4'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    exit 1
fi

killall malamute dshell netmon
rm -f "$dsh_file"
exit 0

