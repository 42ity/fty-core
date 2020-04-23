#!/usr/bin/env python
#
# Copyright (C) 2014 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    insert-measurement.py
#  \brief   tests for inserting measurement from command line
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#  \details inserting measurement from command line
#           automatically creates topic item, discovered device item
#           and link in t_bios_monitor_asset_relation
#

import MySQLdb as db
import random
import time
import datetime
import calendar
import sys


units = {
    "temperature" : "C",
    "realpower"   : "W",
    "voltage"     : "V" ,
    "current"     : "A" ,
    "load"        : "%" ,
    "charge"      : "%" ,
    "frequency"   : "Hz",
    "power"       : "W" ,
    "runtime"     : "s"
}

class Devicer():
    def __init__(self,connection):
        self.devices = {}
        self.connection = connection;
    def updateAssetLink(self, id, name):
        cur = self.connection.cursor()
        cur.execute("select * from t_bios_monitor_asset_relation where id_discovered_device = %s", [id] );
        line = cur.fetchone()
        if line != None: return
        cur.execute("select * from t_bios_asset_element where name = %s", [name]);
        line = cur.fetchone()
        if line == None:
            print "Warning: link between asset and discovered device can't be established"
            return
        assetid = line[0]
        cur.execute("insert into t_bios_monitor_asset_relation ( id_discovered_device, id_asset_element ) values ( %s, %s )",[id,assetid])
        print "Info:  link between asset and discovered device has been established"
    def ID(self, name):
        if self.devices.has_key(name) : return self.devices[name][0]
        cur = self.connection.cursor()
        cur.execute("select * from t_bios_discovered_device where name = %s", [name] );
        line = cur.fetchone()
        if line == None:
            cur.execute("insert into t_bios_discovered_device ( name, id_device_type ) values (%s,1)",[name])
            print cur._executed
            self.connection.commit()
            cur.execute("select * from t_bios_discovered_device where name = %s", [name] );
            line = cur.fetchone()
        if line != None:
            self.updateAssetLink(line[0],line[1])
            self.devices[name] = line
            return int(line[0])
        return None

class Topicer:
    def __init__(self, connection):
        self.topics = {}
        self.connection = connection;
    def ID(self, topic):
        if self.topics.has_key(topic) : return self.topics[topic][0]
        cur = self.connection.cursor()
        cur.execute("select * from t_bios_measurement_topic where topic = '%s'" % (topic) );
        line = cur.fetchone()
        if line == None:
            source,device = topic.split("@",2)
            unit = source.split(".",2)[0]
            if units.has_key(unit) : unitshort = units[unit]
            else : unitshort = ""
            deviceID = Devicer(self.connection).ID(device)
            insert = self.connection.cursor()
            insert.execute(
                "insert into t_bios_measurement_topic ( device_id, units, topic ) values ( %s, %s, %s )",
                [deviceID,unitshort,topic]
            )
            print insert._executed
            self.connection.commit()
            cur.execute("select * from t_bios_measurement_topic where topic = '%s'" % (topic) );
            line = cur.fetchone()
        if line != None :
            self.topics[topic] = line
            return int(line[0])
        return None

class Measurement:
    def __init__(self,connection):
        self.connection = connection;
        self.topicer = Topicer(connection);
    def insert(self, topic, value, time = int( time.time() ) ):
        if topic[:12] == "measurement.":
            topic = topic[12:]
        (what,device) = topic.split("@",2)
        value = int(value*100);
        scale = -2
        tid = self.topicer.ID(topic)
        try:
            sql = "insert into t_bios_measurement ( timestamp, value, scale, topic_id ) values ( %s, %s, %s, %s )"
            cur = self.connection.cursor()
            cur.execute( sql, [time, value, scale, tid] )
        except:
            print "Unexpected error: ", sys.exc_info()[0]
            print "failed to insert measurement"

def usage():
    print "usage: insert-measurement [opts]"
    print "  -t|--topic topic (mandatory)"
    print "  -v|--value value (mandatory)"
    print "  -d|--date \"YYYY-MM-DD hh:mm:ss\" You can specify hh:mm:ss part only -"
    print "             actual date will be used then. if date option is not present"
    print "             at all, current date-time is used." 
    print "  -h|--host address - mysql server address [127.0.0.1]"
    print "  -p|--port port - mysql server port [3306]"

topic = ""
value = ""
date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S");
host = "127.0.0.1"
port = 3306

i = 1;
while i < len(sys.argv) - 1 :
    if sys.argv[i] == "--topic" or sys.argv[i] == "-t":
        topic = sys.argv[i+1]
        i += 1
    elif sys.argv[i] == "--value" or sys.argv[i] == "-v" :
        value = float(sys.argv[i+1])
        i += 1
    elif sys.argv[i] == "--host" or sys.argv[i] == "-h" :
        host = sys.argv[i+1]
        i += 1
    elif sys.argv[i] == "--port" or sys.argv[i] == "-p" :
        port = int(sys.argv[i+1])
        i += 1
    elif sys.argv[i] == "--date" or sys.argv[i] == "-d" :
        try:
            date = sys.argv[i+1]
            datetime.datetime.strptime(date,"%Y-%m-%d %H:%M:%S")
        except:
            try:
                date = datetime.datetime.today().strftime("%Y-%m-%d ") + sys.argv[i+1];
                datetime.datetime.strptime(date, "%Y-%m-%d %H:%M:%S");
            except:
                print "Unknown date format"
        i += 1
    else:
        print "Unknown parameter %s" % (sys.argv[i],)
        usage()
        sys.exit(1)
    i+=1

if topic == "" or value == "":
    print "Option --topic and --value are mandatory!"
    usage()
    sys.exit(1)

conn=1
try:
    conn = db.connect( host=host, port=port, user="root", db="box_utf8")
except:
    print "Connection failed"
    sys.exit(1)
measurement = Measurement(conn);
measurement.insert(topic, value, date);
conn.commit()
sys.exit(0)
