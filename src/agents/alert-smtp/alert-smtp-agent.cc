/*
Copyright (C) 2014-2015 Eaton
 
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

/*! \file   alert-smtp-agent.h
    \brief  Agent that sends notifications through smtp
    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#include <stdio.h>

#include "log.h"
#include "agents.h"
#include "utils_ymsg.h"
#include "alert-smtp-agent.h"
#include "email.h"
#include <sys/types.h>
#include <unistd.h>

const std::string AlertSmtpAgent::_emailFrom = "alenachernikava@eaton.com";
const std::string AlertSmtpAgent::_emailTo = "alenachernikava@eaton.com";

std::string AlertSmtpAgent::emailAddressTo(void)
{
    return _emailTo;
}

std::string AlertSmtpAgent::emailAddressFrom(void)
{
    return _emailFrom;
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
            log_debug ("ten alarm uz nekdy byl, ale skoncil. Ted zase zacal");
        }
        else
        {
            // je to stejny alarm,
            log_debug ("ten alarm porad bezi");
//            tmp.first->second = a;
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
    pid_t tmptmp = getpid();
    if ( ! (it->second.informedStart()) )
    {
        log_debug ("Whant to notify, that event started");
        shared::Smtp smtp{"mail.etn.com", emailAddressFrom() };
        try {
            smtp.sendmail(
                emailAddressTo(),
                "The blue pill taken by Neo",
                "Dear Mr. Smith,\n......" + it->second.toString() +"pid=" + std::to_string(tmptmp));
            log_error("blue mail sended");
            it->second.informStart();
        }
        catch (const std::runtime_error& e) {
            log_error("unexcpected error");
           // here we'll handle the error
        }
    }
    
    if ( ( it->second.till() != 0 )  && ( ! (it->second.informedEnd()) ))
    {
        log_debug ("Whant to notify, that event ended");
        shared::Smtp smtp{"mail.etn.com", emailAddressFrom() };
        try {
            smtp.sendmail(
                emailAddressTo(),
                "The red pill taken by Neo",
                "Dear Mr. Smith,\n......"+ it->second.toString() +"pid=" + std::to_string(tmptmp));
            log_error("red mail sended");
            it->second.informEnd();
        }
        catch (const std::runtime_error& e) {
            log_error("unexcpected error");
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
    alert_priority_t priority;
    alert_state_t    state;

    log_debug ("topic='%s'", subject());
    log_debug ("sender='%s'", sender());
    // decode
    int rv = bios_alert_decode (message, &ruleName, &priority,
                   &state, &devices, &description, &since);
    log_debug ("ruleName=%s",ruleName);
    log_debug ("since=%ld",since);
    log_debug ("state=%d",state);
    if ( !rv ) //if decode was successful
    {
        if ( state == ALERT_STATE_NO_ALERT )
        {
            log_debug ("ALERT_STATE_NO_ALERT");
            AlertBasic a (ruleName, 1, devices, description, since, 0, priority);
            auto it = addAlertClose(a);
            notify (it);
        }
        if ( state == ALERT_STATE_ONGOING_ALERT )
        {  
            log_debug ("ALERT_STATE_ONGOING_ALERT");
            AlertBasic a (ruleName, 1, devices, description, 0, since, priority);
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
    log_set_level(LOG_DEBUG);
    log_info ("agent alert smtp started");
    AlertSmtpAgent agent("alert-smtp");
    if( agent.connect("ipc://@/malamute", bios_get_stream_main(), "^alert\\..*") ) {
        result = agent.run();
    }
    log_info ("Alert Smpt Agent exited with code %i\n", result);
    return result;
}
