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

/*! \file   agent-alert-smtp.h
    \brief  Agent that sends notifications through smtp
    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#ifndef SRC_AGENTS_ALERT_SMTP_AGENT_H__
#define SRC_AGENTS_ALERT_SMTP_AGENT_H__

#include <set>
#include <map>
#include "bios_agent++.h"
#include "alert-basic.h"

typedef std::map <std::string, AlertBasic>::iterator alertIterator_t;

class AlertSmtpAgent : public BIOSAgent {
public:
    explicit AlertSmtpAgent( const char *agentName ) :BIOSAgent( agentName ) { configure(); };
    explicit AlertSmtpAgent( const std::string &agentName ) :BIOSAgent( agentName ) { };
    
    void onSend(ymsg_t **message);
    void configure();
    
    std::map <std::string, AlertBasic>::iterator addAlertClose(AlertBasic a);
    std::map <std::string, AlertBasic>::iterator addAlertNew(AlertBasic a);
private:
    // TODO: should be moved somewhere????
    static const int ALERT_START=0x01;
    static const int ALERT_END  =0x02;
    std::vector<std::string> _emailTo;
    std::string _emailFrom;
    std::string _emailBodyStart;
    std::string _emailSubjectStart;
    std::string _emailBodyEnd;
    std::string _emailSubjectEnd;
    std::string _smtpServer;

protected:
    // represents an information about alerts retrieved from DB
    std::map <std::string, AlertBasic> alertList;
    
    std::vector<std::string> emailAddressTo(void) { return _emailTo; };
    std::string emailAddressFrom(void) { return _emailFrom; };
    std::string emailBody( alert_state_t state );
    std::string emailSubject( alert_state_t state );
    std::string smtpServer() { return _smtpServer; }
    void notify (std::map <std::string, AlertBasic>::iterator it);
    std::string replaceTokens(const std::string &text, const std::string &pattern, const std::string &replacement) const;
    std::string replaceTokens( const std::string text, alertIterator_t it ) const;
};

#endif // SRC_AGENTS_ALERT_SMTP_AGENT_H__

