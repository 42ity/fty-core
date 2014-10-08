#include <algorithm>

#include <czmq.h>

#include "cidr.h"
#include "persistence.h"
#include "netdisc_msg.h"

namespace utils{

bool
process_message(std::string url, netdisc_msg_t *msg)
{
    // Rewrites the object
    // Destroys msg
    assert (msg);   
    utils::NetHistory nethistory(url);

    bool result = false;

    int msg_id = netdisc_msg_id (msg);
    
    const char *name; 
    byte ipver;
    const char *ipaddr;
    byte prefixlen;
    char command;
    const char *mac_in;
    std::string mac;

    int n = 0;

    switch (msg_id){
        case NETDISC_MSG_AUTO_ADD:
        {

            //---- read only fields needed to validate unique of the input

            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'a' ;                   // TODO some constant
            
            CIDRAddress address(ipaddr,prefixlen);
            
            nethistory.setAddress(address); // address
            nethistory.setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 0 )
            {   // if unique, then read rest fields and save to db
                name      = netdisc_msg_name (msg);
                mac_in    = netdisc_msg_mac (msg);
                mac       = std::string(mac_in);
            
                // because the mac-address is stored as number, we need to remove : (5 times) and drop last 5 characters.
                std::remove(mac.begin(), mac.end(), ':');
                mac.erase(12,5);

                nethistory.setName(name);      // name
                nethistory.setMac(mac);        // mac

                n = nethistory.dbsave();

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
                else
                    result = true;
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
        }
        case NETDISC_MSG_AUTO_DEL:
        {   
            //---- read only fields needed to validate if we need to delete input from db

            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'a' ;                   // TODO some constant
            
            CIDRAddress address(ipaddr,prefixlen);
            
            nethistory.setAddress(address); // address
            nethistory.setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 1 )  
            {   // if it was added before need to delete it from db
                // we are not interested in name and mac
                // name      = netdisc_msg_name (msg);
                // mac       = std::string(netdisc_msg_mac (msg));
            
                n = nethistory.deleteById(ids[0]);

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
                else
                    result = true;
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
         
        }
        case NETDISC_MSG_MAN_ADD:
        {
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'm' ;               // TODO some constant
            
            CIDRAddress address(ipaddr,prefixlen);
            
            nethistory.setAddress(address); // address
            nethistory.setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 0 )  
            { 
                n = nethistory.dbsave();

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
                else
                    result = true;
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
        }
        case NETDISC_MSG_MAN_DEL:
        { 
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'm' ;               // TODO some constant
            
            CIDRAddress address(ipaddr,prefixlen);
            
            nethistory.setAddress(address); // address
            nethistory.setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 1 )  
            { 
                n = nethistory.deleteById(ids[0]);

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
                else
                    result = true;
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
        }
        case NETDISC_MSG_EXCL_ADD:
        {            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'e' ;               // TODO some constant
            
            CIDRAddress address(ipaddr,prefixlen);
            
            nethistory.setAddress(address); // address
            nethistory.setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 0 )  
            { 
                n = nethistory.dbsave();

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
                else
                    result = true;
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;

        }
        case NETDISC_MSG_EXCL_DEL:
        {            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'e' ;               // TODO some constant
            
            CIDRAddress address(ipaddr,prefixlen);
            
            nethistory.setAddress(address); // address
            nethistory.setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 1 )  
            { 
                n = nethistory.deleteById(ids[0]);

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
                else
                    result = true;
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;

        }
        case NETDISC_MSG_TEST:
        {
        }
        default:
        {// THIS SHOULD NEVER HAPPEN Unknown type of message
        }
    }
       

        
    netdisc_msg_destroy (&msg);
    return n;
};

}
