#include "log.h"
#include "cidr.h"
#include "persistence.h"
#include "netdisc_msg.h"
void
parse(NetHistory *nethistory, msg *msg)
{
    // Rewrites the object
    // Destroys msg
    assert (msg);   
    
    int msg_id = netdisc_msg_id (netdisc_msg_t *self);
    
    const char *name; 
    byte ipver;
    const char *ipaddr;
    byte prefixlen;
    char command;
    std::string mac;

    switch (msg_id){
        case NETDISC_MSG_AUTO_ADD:
        {

            //---- read only fields needed to validate unique of the input

            // url is stored in dbpath.h TODO
            utils::NetHistory nethistory = utils::NetHistory(url);
            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'a' ;                   // TODO some constant
            
            CIDRADdress adress(ipaddr,prefixlen);
            
            nethistory->setAddress(address); // address
            nethistory->setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 0 )
            {   // if unique, then read rest fields and save to db
                name      = netdisc_msg_name (msg);
                mac       = std::string(netdisc_msg_mac (msg));
            
                // because the mac-address is stored as number, we need to remove : (5 times) and drop last 5 characters.
                std::string mac(s); 
                std::remove(mac.begin(), mac.end(), ':');
                mac.erase(12,5);

                nethistory->setName(name);      // name
                nethistory->setMac(s);          // mac

                int n = nethistory.save();

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
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

            // url is stored in dbpath.h TODO
            utils::NetHistory nethistory = utils::NetHistory(url);
            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'a' ;                   // TODO some constant
            
            CIDRADdress adress(ipaddr,prefixlen);
            
            nethistory->setAddress(address); // address
            nethistory->setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 1 )  
            {   // if it was added before need to delete it from db
                // we are not interested in name and mac
                // name      = netdisc_msg_name (msg);
                // mac       = std::string(netdisc_msg_mac (msg));
            
                int n = nethistory.deleteById(ids[0]);

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
         
        }
        case NETDISC_MSG_MAN_ADD:
        {
            // url is stored in dbpath.h TODO
            utils::NetHistory nethistory = utils::NetHistory(url);
            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'm' ;               // TODO some constant
            
            CIDRADdress adress(ipaddr,prefixlen);
            
            nethistory->setAddress(address); // address
            nethistory->setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 0 )  
            { 
                int n = nethistory.save();

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
        }
        case NETDISC_MSG_MAN_DEL:
        { 
            // url is stored in dbpath.h TODO
            utils::NetHistory nethistory = utils::NetHistory(url);
            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'm' ;               // TODO some constant
            
            CIDRADdress adress(ipaddr,prefixlen);
            
            nethistory->setAddress(address); // address
            nethistory->setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 1 )  
            { 
                int n = nethistory.deleteById(ids[0]);

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;
        }
        case NETDISC_MSG_EXCL_ADD:
        {            
            // url is stored in dbpath.h TODO
            utils::NetHistory nethistory = utils::NetHistory(url);
            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'e' ;               // TODO some constant
            
            CIDRADdress adress(ipaddr,prefixlen);
            
            nethistory->setAddress(address); // address
            nethistory->setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 0 )  
            { 
                int n = nethistory.save();

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
            }
            if ( ids.size() >1 )
            {
                // log this should never happen here is more than one row according to the 
            }
            break;

        }
        case NETDISC_MSG_EXCL_DEL:
        {            // url is stored in dbpath.h TODO
            utils::NetHistory nethistory = utils::NetHistory(url);
            
            //TODO now it stores IP as string in DB.
            //1. want to store as number
            //2. want to store as ipv6
            ipver     = netdisc_msg_ipver (msg);
            ipaddr    = netdisc_msg_ipaddr (msg);
            prefixlen = netdisc_msg_prefixlen (msg);
            command   = 'e' ;               // TODO some constant
            
            CIDRADdress adress(ipaddr,prefixlen);
            
            nethistory->setAddress(address); // address
            nethistory->setCommand(command); // command

            std::vector<int> ids = nethistory.checkUnique();
            
            if ( ids.size() == 1 )  
            { 
                int n = nethistory.deleteById(ids[0]);

                if ( n == 0 )
                {
                     // log THERE IS SOME PRoBLEM WITH DB
                }
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
        // THIS SHOULD NEVER HAPPEN Unknown type of message
    }
       

        
    netdisc_msg_destroy (&msg);

}
