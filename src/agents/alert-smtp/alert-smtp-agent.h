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

#ifndef SRC_AGENTS_ALERT_SMTP_AGENT_H__
#define SRC_AGENTS_ALERT_SMTP_AGENT_H__

#include <set>
#include <map>
#include "bios_agent++.h"
#include "alert-basic.h"

class AlertSmtpAgent : public BIOSAgent {
public:
    explicit AlertSmtpAgent( const char *agentName ) :BIOSAgent( agentName ) {  };
    explicit AlertSmtpAgent( const std::string &agentName ) :BIOSAgent( agentName ) { };
    
    void onSend(ymsg_t **message);
    void onReply(ymsg_t **message){};
    
    struct alert_info_t{
        std::string ruleName;
        int64_t dateFrom;
        int64_t dateTill;
        int8_t notification;
    };
    std::map <std::string, AlertBasic>::iterator addAlertClose(AlertBasic a);
    std::map <std::string, AlertBasic>::iterator addAlertNew(AlertBasic a);
private:
    // TODO: should be moved somewhere????
    static const int ALERT_START=0x01;
    static const int ALERT_END  =0x02;
    static const std::string _emailTo;
    static const std::string _emailFrom;

    void notifyUser(const std::string &email, alert_info_t info, 
                    const char * devices, const char * descriprion,
                    alert_priority_t  priority, alert_state_t state){};
    void notifyEndDB(const char *rule_name, int64_t since){};
    void notifyMyself(const char *rule_name, int8_t flag, int64_t since){};
protected:
        // represents an information about alerts retrieved from DB
    std::map <std::string, AlertBasic> alertList;
    
    void addAlertToList(const std::string &ruleName,int64_t dateFrom, int64_t dateTill);
    void addAlertToList(const char        *ruleName,int64_t dateFrom, int64_t dateTill);

    
    void notily();
    std::string emailAddressTo(void);
    std::string emailAddressFrom(void);
    void askDB(const std::string &ruleName){};
    void askDBforNotNotified(void){};
    void notify (std::map <std::string, AlertBasic>::iterator it);


};

#endif // SRC_AGENTS_NUT_NUT_AGENT_H__

