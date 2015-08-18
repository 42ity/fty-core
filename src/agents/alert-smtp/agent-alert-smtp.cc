/*
Copyright (C) 2014-2015 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>

#include "log.h"
#include "agents.h"
#include "utils_ymsg.h"
#include "agent-alert-smtp.h"
#include "email.h"
#include "str_defs.h"
#include <sys/types.h>
#include <unistd.h>
#include <cxxtools/split.h>

std::string AlertSmtpAgent::emailBody( alert_state_t state )
{
    return ( state == ALERT_STATE_ONGOING_ALERT ) ? _emailBodyStart : _emailBodyEnd;
};

std::string AlertSmtpAgent::emailSubject( alert_state_t state )
{
    return ( state == ALERT_STATE_ONGOING_ALERT ) ? _emailSubjectStart : _emailSubjectEnd ;
};

void AlertSmtpAgent::configure()
{
    char *env;

    env = getenv("BIOS_EMAIL_FROM");
    _emailFrom = env ? env : "bios@eaton.com" ;
    
    env = getenv("BIOS_EMAIL_TO");
    if( env && strlen(env) ) {
        cxxtools::split( ',', std::string(env), std::back_inserter(_emailTo) );
    }
    env = getenv("BIOS_EMAIL_BODY_START");
    _emailBodyStart = env ? env : "Alert ${rulename} started.";
    env = getenv("BIOS_EMAIL_SUBJECT_START");
    _emailSubjectStart = env ? env : "Alert P${priority} ${rulename} is active!";
    
    env = getenv("BIOS_EMAIL_BODY_END");
    _emailBodyEnd = env ? env : "Alert ${rulename} ended.";
    env = getenv("BIOS_EMAIL_SUBJECT_END");
    _emailSubjectEnd = env ? env : "Alert P${priority} ${rulename} ended.";

    env = getenv("BIOS_SMTP_SERVER");
    _smtpServer = env ? env : "";
    if( _smtpServer.empty() ) log_error("SMTP server is not specified");
    if( _emailTo.empty() ) log_error("Mail recipient is not specified");
}

std::string AlertSmtpAgent::replaceTokens( const std::string &text, const std::string &pattern, const std::string &replacement) const
{
    std::string result = text;
    size_t pos = 0;
    while( ( pos = result.find(pattern, pos) ) != std::string::npos){
        result.replace(pos, pattern.length(), replacement);
        pos += replacement.length();
    }
    return result;
}

std::string AlertSmtpAgent::replaceTokens( const std::string &text, alertIterator_t it ) const
{
    std::string result = replaceTokens(text, "${rulename}", it->first );
    return replaceTokens(result, "${priority}", std::to_string( it->second.priority() ) );
}

std::map <std::string, AlertBasic>::iterator 
    AlertSmtpAgent::addAlertNew
        (AlertBasic a)
{
    LOG_START;
    auto tmp = alertList.insert(std::pair<std::string, AlertBasic>(a.ruleName(), a));
    if ( tmp.second == false )
    {
        log_debug ("element is already in myDB, we need to check if we need to update it");
        // nebyl vlozen, protoze key is duplicate
        // tmp->first->second - actualni hodnota nalezeneho alertu
        if ( tmp.first->second.since() < a.since() )
        {
            //rewrite
            tmp.first->second = a;
            log_debug ("this alarm started some time ago, then finished. Now it started again.");
        }
        else
        {
            // je to stejny alarm,
            log_debug ("alarm is stil active, and we know it");
            // TODO: if we want to notify user if some attribute of the alarm changed
            // it is right place to place here some code
        }
    }
    else{
        log_debug ("element was not found in myDB, it was successfully added to myDB");
    }
    //byl vlozen uspesne
    LOG_END;
    return tmp.first;
}

std::map <std::string, AlertBasic>::iterator 
    AlertSmtpAgent::addAlertClose
        (AlertBasic a)
{
    LOG_START;
    auto tmp = alertList.insert(std::pair<std::string, AlertBasic>(a.ruleName(), a));
    if ( tmp.second == false )
    {
        log_debug ("element is already in myDB, we need to check if we need to update it");
        // nebyl vlozen, protoze key is duplicate
        // tmp->first->second - actualni hodnota nalezeneho alertu
        tmp.first->second.till(a.till());
        tmp.first->second.state(a.state());
    }
    //byl vlozen uspesne
    else{
        log_debug ("element was not found in myDB, it was successfully added to myDB");
    }
    LOG_END;
    return tmp.first;
}

void 
    AlertSmtpAgent::notify
        (std::map <std::string, AlertBasic>::iterator it)
{
    LOG_START;
    //pid_t tmptmp = getpid();
    log_debug("alert state is %i",it->second.state() );
    if( emailAddressTo().empty() || smtpServer().empty() ) {
        log_error("Mail system is not configured!");
        return;
    }
    if ( ( ! it->second.informedStart() ) && ( it->second.state() == ALERT_STATE_ONGOING_ALERT ) )
    {
        log_debug ("Want to notify, that event started");
        shared::Smtp smtp{ smtpServer(), emailAddressFrom() };
        try {
            smtp.sendmail(
                emailAddressTo(),
                replaceTokens( emailSubject( ALERT_STATE_ONGOING_ALERT ), it ),
                replaceTokens( emailBody( ALERT_STATE_ONGOING_ALERT ), it ));
            log_error("blue mail sended");
            it->second.informStart();
        }
        catch (const std::runtime_error& e) {
            LOG_END_ABNORMAL(e);
            // here we'll handle the error
        }
    }
    
    if ( ( it->second.till() != 0 )  && ( ! it->second.informedEnd() ) && ( it->second.state() == ALERT_STATE_NO_ALERT ) )
    {
        log_debug ("What to notify, that event ended");
        shared::Smtp smtp{ smtpServer(), emailAddressFrom() };
        try {
            smtp.sendmail(
                emailAddressTo(),
                replaceTokens( emailSubject( ALERT_STATE_NO_ALERT ), it ),
                replaceTokens( emailBody( ALERT_STATE_NO_ALERT ), it ));
            log_error("red mail sended");
            it->second.informEnd();
        }
        catch (const std::runtime_error& e) {
            LOG_END_ABNORMAL(e);
           // here we'll handle the error
        }
    }
    LOG_END;
    return;
}

// ASSUMPTION:
//  processes are running forever and never restarts 
//  -> we can have all neseccarry information inside
void AlertSmtpAgent::onSend(ymsg_t **message) {
    LOG_START;
    // when some mmessage from the stream is recieved

    // prepare for decode
    char   *ruleName    = NULL;
    char   *devices     = NULL;
    char   *description = NULL;
    time_t  since       = 0;
    uint8_t priority;
    int8_t    state;

    log_debug ("topic='%s'", subject());
    log_debug ("sender='%s'", sender());
    // decode
    int rv = bios_alert_extract (*message, &ruleName, &priority,
                   &state, &devices, &description, &since);
    log_debug ("ruleName=%s",ruleName);
    log_debug ("since=%ld",since);
    log_debug ("state=%d",state);
    if ( !rv ) //if decode was successful
    {
        if ( state == ALERT_STATE_NO_ALERT )
        {
            log_debug ("ALERT_STATE_NO_ALERT");
            AlertBasic a (ruleName, ALERT_STATE_NO_ALERT, devices, description, since, 0, priority);
            if( alertList.find(ruleName) == alertList.end() ) {
                // such alert doesn't exists and is ok => don't notify
                auto it = addAlertClose(a);
                it->second.informEnd();
            } else {
                auto it = addAlertClose(a);
                notify (it);
            }
        }
        if ( state == ALERT_STATE_ONGOING_ALERT )
        {  
            log_debug ("ALERT_STATE_ONGOING_ALERT");
            AlertBasic a (ruleName, ALERT_STATE_ONGOING_ALERT, devices, description, 0, since, priority);
            auto it = addAlertNew(a);
            notify (it);
        }
    }
    if ( ruleName ) free(ruleName);
    if ( devices ) free(devices);
    if ( description ) free (description);
    LOG_END;
    return;
}

int main(int argc, char **argv){
    if( argc > 0 ) {}; // silence compiler warnings
    if( argv ) {};  // silence compiler warnings
    
    int result = 1;
    log_open();
    log_info ("Alert Smtp Agent started");
    AlertSmtpAgent agent("alert-smtp");
    if( agent.connect(MLM_ENDPOINT, bios_get_stream_main(), "^alert\\.") ) {
        result = agent.run();
    }
    log_info ("Alert Smpt Agent exited with code %i\n", result);
    return result;
}
