/*
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! file bios_agent++.h
    brief Class implementing basic agent.
    author Tomas Halman <tomashalman@eaton.com>
*/

#ifndef INCLUDE_BIOS_AGENTPP_H__
#define INCLUDE_BIOS_AGENTPP_H__

#include <string>
#include <vector>
#include <ctime>

#include "bios_agent.h"
    
class BIOSAgent {
 public:
    explicit BIOSAgent(const char *agentName) { _agentName = agentName; };
    explicit BIOSAgent(const std::string &agentName) { _agentName = agentName; };
    virtual ~BIOSAgent() { bios_agent_destroy( &_bios_agent ); };

    int send(  const char *subject,  ymsg_t **msg_p ) { return bios_agent_send( _bios_agent, subject, msg_p ); };
    int sendto(  const char *address,  const char *subject,  ymsg_t **send_p ) { return bios_agent_sendto( _bios_agent, address, subject, send_p ); };
    int replyto(  const char *address,  const char *subject,  ymsg_t **reply_p,  ymsg_t *send ) { return bios_agent_replyto( _bios_agent, address, subject, reply_p, send ); };
    int sendfor(  const char *address,  const char *subject,  ymsg_t **send_p ) { return bios_agent_sendfor( _bios_agent, address, subject, send_p ); };
    ymsg_t * recv(  ) { return bios_agent_recv( _bios_agent ); };
    ymsg_t * recv_wait(  int timeout ) { return bios_agent_recv_wait( _bios_agent, timeout ); };
    int set_producer(  const char *stream ) { return bios_agent_set_producer( _bios_agent, stream ); };
    int set_consumer(  const char *stream,  const char *pattern ) { return bios_agent_set_consumer( _bios_agent, stream, pattern ); };
    const char * command(  ) { return bios_agent_command( _bios_agent ); };
    int status(  ) { return bios_agent_status( _bios_agent ); };
    const char * reason(  ) { return bios_agent_reason( _bios_agent ); };
    const char * address(  ) { return bios_agent_address( _bios_agent ); };
    const char * sender(  ) { return bios_agent_sender( _bios_agent ); };
    const char * subject(  ) { return bios_agent_subject( _bios_agent ); };
    ymsg_t * content(  ) { return bios_agent_content( _bios_agent ); };
    zactor_t * actor(  ) { return bios_agent_actor( _bios_agent ); };
    zsock_t * msgpipe(  ) { return bios_agent_msgpipe( _bios_agent ); };

    void timeout(const int timeoutms) { _timeout = timeoutms; };
    int timeout() { return _timeout; };

    std::string agentName() { return _agentName; };
    void agentName(const std::string newname) { _agentName = newname; }
    virtual void onSend( ymsg_t **message ) { ymsg_destroy( message ); };
    virtual void onReply( ymsg_t **message ) { ymsg_destroy( message ); };
    virtual void onPoll() { };
    virtual void onStart() { };
    virtual void onEnd() { };
    virtual void main() {
        zsock_t *pipe = msgpipe();
        zpoller_t *poller = zpoller_new(pipe, NULL);
        zsock_t *which = NULL;
        
        while(! zsys_interrupted) {
            which = (zsock_t *)zpoller_wait(poller, _timeout);
            if( zsys_interrupted ) break;
            if(which) {
                ymsg_t *message = recv( );
                if( message ) {
                    switch( ymsg_id(message) ) {
                    case YMSG_REPLY:
                        onReply( &message );
                    case YMSG_SEND:
                        onSend( &message );
                    }
                    ymsg_destroy( &message );
                }
            } else {
                onPoll();
            }
        }
        zpoller_destroy( &poller );
    };
    bool connect(const char * endpoint, const char *stream = NULL,
                                        const char *pattern = NULL) {
        if( endpoint == NULL || _agentName.empty() ) return false; 
        if( _bios_agent ) bios_agent_destroy( &_bios_agent );
        _bios_agent = bios_agent_new( endpoint, _agentName.c_str() );
        if( _bios_agent == NULL ) return false;
        if( stream ) {
            if( set_producer( stream ) < 0 ) {
                bios_agent_destroy( &_bios_agent );
                return false;
            }
            if( pattern ) {
                if( set_consumer( stream, pattern ) < 0 ) {
                    bios_agent_destroy(&_bios_agent);
                    return false;
                }
            }
        }
        return true;
    };
    bios_agent_t *get_c_bios_agent() { return _bios_agent; }
    int run() { onStart(); main(); onEnd(); return _exitStatus; }
 private:
    void handleReplies( ymsg_t *message );
 protected:
    bios_agent_t *_bios_agent = NULL;
    int _exitStatus = 0;
    int _timeout = 2000;
    std::string _agentName;
};

#endif // INCLUDE_BIOS_AGENTPP_H__


