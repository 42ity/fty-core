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

/*! \file alert.h
    \brief Class for basic alert
    \author Chernikava Alena <AlenaChernikava@eaton.com>
*/
 
#ifndef SRC_AGENTS_ALERT_SMTP_ALERT_BASIC_H__
#define SRC_AGENTS_ALERT_SMTP_ALERT_BASIC_H__

#include <string>

class AlertBasic {
public:
    
    int8_t state() const  
        { return _state; };
    void state(int8_t newState)
        { _state = newState; };

    std::string ruleName() const  
        { return _rule_name; };
    void ruleName(const char *text) 
        { _rule_name = text; };
    void ruleName(const std::string &text) 
        { _rule_name = text; };
    
    bool informed () const 
        { return _informed; };
    bool informedStart () const 
        { return  ( _informed & ALERT_START ); };
    bool informedEnd () const 
        { return  ( _informed & ALERT_END ); };
    void informStart()
        { _informed = ( _informed | ALERT_START ); }; 
    void informEnd()
        { _informed = ( _informed | ALERT_END ); }; 

    std::string devices() const
        { return _devices; }
    void devices(const char* text)
        { _devices = text; }
    void ddevice(const std::string &text)
        { _devices = text; }

    std::string description() const
        { return _description; }
    void description(const char* text)
        { _description = text; }
    void description(const std::string &text)
        { _description = text; }

    int8_t priority() const
        { return _priority; }
    void priority(int8_t priority)
        { _priority = priority; }

    time_t since() const
        { return _since; };
    void since (time_t datesince)
        { _since = datesince; } ;

    time_t till() const
        { return _till; };
    void till (time_t datetill)
        { _till = datetill; } ;

    AlertBasic (const char *rulename, uint8_t state, 
        const char *devices, const char *description, 
        time_t till,
        time_t since, int8_t priority)
    {
        _since = since;
        _rule_name = rulename;
        _state = state;
        _devices = devices;
        _description = description;
        _priority = priority;
        _till = till;
    };

    std::string toString() const{
        return "rulename= "+ruleName() + "; devices=" + devices() + "; since = " 
            + std::to_string(since()) + ";till=" + std::to_string(till());
    }
 protected:
    static const int ALERT_START=0x01;
    static const int ALERT_END  =0x02;

    time_t _since = 0;
    int8_t _informed = 0;
    int8_t _state = 0;
    std::string _rule_name;
    std::string _devices;
    std::string _description;
    int8_t _priority;
    time_t _till;

};

#endif // SRC_AGENTS_ALERT_SMTP_ALERT_BASIC_H__
