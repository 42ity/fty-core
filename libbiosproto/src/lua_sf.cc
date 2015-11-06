extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/directory.h>
#include <malamute.h>
#include "bios_proto.h"
#include <math.h>

#define ALERT_START 1
#define ALERT_ACK1  2
#define ALERT_ACK2  3
#define ALERT_ACK3  4
#define ALERT_ACK4  5
#define ALERT_RESOLVED 6

const char* get_status_string(int status)
{
    switch (status) {
        case ALERT_START:
            return "ACTIVE";
        case ALERT_ACK1:
            return "ACK-WIP";
        case ALERT_ACK2:
            return "ACK-PAUSE";
        case ALERT_ACK3:
            return "ACK-IGNORE";
        case ALERT_ACK4:
            return "ACK-SILENCE";
        case ALERT_RESOLVED:
            return "RESOLVED";
    }
    return "UNKNOWN";
}

class MetricInfo {
public:
    std::string _element_name;
    std::string _source;
    std::string _units;
    double      _value;
    int64_t     _timestamp;
    std::string _element_destination_name;

    std::string generateTopic(void) const{
        return _source + "@" + _element_name;
    };

    MetricInfo() {};
    MetricInfo (
        const std::string &element_name,
        const std::string &source,
        const std::string &units,
        double value,
        int64_t timestamp,
        const std::string &destination
        ):
        _element_name(element_name),
        _source(source),
        _units(units),
        _value(value),
        _timestamp(timestamp),
        _element_destination_name (destination)
    {};

};


class MetricList {
public:
    MetricList(){};
    ~MetricList(){};

    void addMetric (
        const std::string &element_name,
        const std::string &source,
        const std::string &units,
        double value,
        int64_t timestamp,
        const std::string &destination)
    {
        // create Metric first
        MetricInfo m = MetricInfo(element_name,
                                  source,
                                  units,
                                  value,
                                  timestamp,
                                  destination);

        // try to find topic
        auto it = knownMetrics.find (m.generateTopic());
        if ( it != knownMetrics.cend() ) {
            // if it was found -> replace with new value
            it->second = m;
        }
        else {
            // if it wasn't found -> insert new metric
            knownMetrics.emplace (m.generateTopic(), m);
        }
    };

    void addMetric (const MetricInfo &m)
    {
        // try to find topic
        auto it = knownMetrics.find (m.generateTopic());
        if ( it != knownMetrics.cend() ) {
            // if it was found -> replace with new value
            it->second = m;
        }
        else {
            // if it wasn't found -> insert new metric
            knownMetrics.emplace (m.generateTopic(), m);
        }
    };

    double findAndCheck (const std::string &topic)
    {
        auto it = knownMetrics.find(topic);
        if ( it == knownMetrics.cend() )
        {
            return NAN;
        }
        else
        {
            int maxLiveTime = 5*60;
            int64_t currentTimestamp = ::time(NULL);
            if ( ( currentTimestamp - it->second._timestamp ) > maxLiveTime )
            {
                knownMetrics.erase(it);
                return NAN;
            }
            else
            {
                return it->second._value;
            }
        }
    };

    double find (const std::string &topic) const
    {
        auto it = knownMetrics.find(topic);
        if ( it == knownMetrics.cend() )
        {
            return NAN;
        }
        else
        {
            return it->second._value;
        }
    };

    int getMetricInfo (const std::string &topic, MetricInfo &metricInfo) const
    {
        auto it = knownMetrics.find(topic);
        if ( it == knownMetrics.cend() )
        {
            return -1;
        }
        else
        {
            metricInfo = it->second;
            return 0;
        }
    };

    void removeOldMetrics()
    {
        int maxLiveTime = 5*60;
        int64_t currentTimestamp = ::time(NULL);

        for ( std::map<std::string, MetricInfo>::iterator iter = knownMetrics.begin(); iter != knownMetrics.end() ; )
        {
            if ( ( currentTimestamp - iter->second._timestamp ) > maxLiveTime )
            {
                knownMetrics.erase(iter++);
            }
            else
            {
                ++iter;
            }
        }
    };

private:
    std::map<std::string, MetricInfo> knownMetrics;
};

class Rule {
public:
    std::set<std::string> in;
    zrex_t *rex;
    std::string rex_str;
    std::string lua_code;
    std::string rule_name;
    std::string element;
    std::string severity;

    Rule(){ rex = NULL;};

    Rule(std::ifstream &f)
    {
        // try catch TODO
        cxxtools::JsonDeserializer json(f);
        json.deserialize();
        const cxxtools::SerializationInfo *si = json.si();
        si->getMember("evaluation") >>= lua_code;
        si->getMember("rule_name") >>= rule_name;
        si->getMember("severity") >>= severity;
        if ( si->findMember("in") ) {
            si->getMember("in") >>= in;
            si->getMember("element") >>= element;
        }
        else {
            if ( si->findMember("in_rex") ) {
                si->getMember("in_rex") >>= rex_str;
                rex = zrex_new(rex_str.c_str());
            }
        }
        // can in and in_rex be both at the same file?
        // what should we do if file is broken somehow?
    };

    lua_State* setContext (const MetricList &metricList, const std::string &lastTopic) const
    {
        if ( rex != NULL )
        {
            MetricInfo m;
            // TODO no error handling for now
            metricList.getMetricInfo (lastTopic, m);
            return setContextRegex (m);
        }
        else
        {
            return setContextNormal (metricList);
        }
    };
private:
    lua_State* setContextRegex(const MetricInfo &metricInfo) const
    {
        lua_State *lua_context = lua_open();
        lua_pushnumber(lua_context, metricInfo._value);
        lua_setglobal(lua_context, "value");
        lua_pushstring(lua_context, metricInfo._element_name.c_str());
        lua_setglobal(lua_context, "element");
        return lua_context;
    };


    lua_State* setContextNormal(const MetricList &metricList) const
    {
        lua_State *lua_context = lua_open();
        for ( const auto &neededTopic : in) {
            double neededValue = metricList.find(neededTopic);
            if ( isnan (neededValue) ) {
                zsys_info("Do not have everything for '%s' yet\n", rule_name.c_str());
                lua_close (lua_context);
                return NULL;
            }
            std::string var = neededTopic;
            var[var.find('@')] = '_';
            zsys_info("Setting variable '%s' to %lf\n", var.c_str(), neededValue);
            lua_pushnumber(lua_context, neededValue);
            lua_setglobal(lua_context, var.c_str());
        }
        // we are here -> all variables were found
        return lua_context;
    };
};

struct Alert {
    Rule rule;
    int status ; // on Off ack
    int64_t timestamp;
    std::string description;

    Alert (const Rule &r, int s, int64_t tm, const std::string &descr){
        rule = r;
        status = s;
        timestamp = tm;
        description = descr;
    };
};

std::vector<Alert> alerts{};

std::vector<Alert>::iterator isAlertOngoing(const std::string &rule_name, const std::string &element)
{
    for(std::vector<Alert>::iterator it = alerts.begin(); it != alerts.cend(); ++it)
    {
        if ( ( it->rule.rule_name == rule_name ) && ( it->rule.element == element ) )
        {
            return it;
        }
    }
    return alerts.end();
};

class AlertConfiguration{
public:
    AlertConfiguration(){};
    ~AlertConfiguration(){};

    // returns list of topics to be consumed
    std::set <std::string> readConfiguration(void)
    {
        std::set <std::string> result;

        cxxtools::Directory d(".");
        for ( const auto &fn : d)
        {
            if ( fn.length() < 5 ) {
                continue;
            }
            if ( fn.compare(fn.length() - 5, 5, ".rule") != 0 ) {
                continue;
            }
            std::ifstream f(fn);
            Rule rule(f);
            rule.rule_name = fn;

            for ( const auto &interestedTopic : rule.in ) {
                result.insert (interestedTopic);
                _normalConfigs.push_back (rule);
            }
            if ( rule.rex != NULL )
            {
                result.insert (rule.rex_str);
                _regexConfigs.push_back(rule);
            }
        }
        return result;
    };

    std::vector<Rule> getNormalConfigs(const std::string &topic)
    {
        std::vector <Rule> result{};
        for ( const auto &rule: _normalConfigs )
        {
            if ( rule.in.count(topic) != 0 )
            {
                result.push_back(rule);
            }
        }
        return result;
    };

    std::vector <std::string> updateConfiguration(const Rule &rule);
    std::vector<Rule> _regexConfigs;
    int normalConfigSize(void){
        return _normalConfigs.size();
    };
    int regexConfigSize(void){
        return _regexConfigs.size();
    };

private:
    std::vector <Rule> _normalConfigs;
};

int main (int argc, char** argv) {

    mlm_client_t *client = mlm_client_new();
    mlm_client_connect (client, "ipc://@/malamute", 1000, argv[0]);
    mlm_client_set_producer(client, "ALERTS");

    AlertConfiguration alertConfiguration;
    std::set <std::string> subjectsToConsume = alertConfiguration.readConfiguration();
    zsys_info ("normal count: %d\n", alertConfiguration.normalConfigSize());
    zsys_info ("regexp count: %d\n", alertConfiguration.regexConfigSize());
    zsys_info ("subjectsToConsume count: %d\n", subjectsToConsume.size());
    // Subscribe to all subjects
    for ( const auto &interestedSubject : subjectsToConsume ) {
        mlm_client_set_consumer(client, "BIOS", interestedSubject.c_str());
        zsys_info("Registered to receive '%s'\n", interestedSubject.c_str());
    }

    MetricList cache;

    while(!zsys_interrupted) {
        zmsg_t *zmessage = mlm_client_recv(client);
        if ( zmessage == NULL ) {
            continue;
        }
        char *type = NULL;
        char *element_src = NULL;
        char *value = NULL;
        char *unit = NULL;
        int64_t timestamp = 0;
        int r = metric_decode (&zmessage, &type, &element_src, &value, &unit, &timestamp, NULL);
        if ( r != 0 ) {
            zsys_info ("cannot decode metric, ignore message\n");
            continue;
        }
        char *end;
        double dvalue = strtod (value, &end);
        if (errno == ERANGE) {
            errno = 0;
            zsys_info ("cannot convert to double, ignore message\n");
            continue;
        }
        else if (end == value || *end != '\0') {
            zsys_info ("cannot convert to double, ignore message\n");
            continue;
        }

        std::string topic = mlm_client_subject(client);
        zsys_info("Got message '%s' with value %s\n", topic.c_str(), value);

        // Update cache with new value
        MetricInfo m (element_src, type, unit, dvalue, timestamp, "");
        cache.addMetric (m);
        cache.removeOldMetrics();

        // Handle non-regex configs
        for ( const auto &rule : alertConfiguration.getNormalConfigs(m.generateTopic()))
        {
            lua_State *lua_context = rule.setContext(cache, type);
            if ( lua_context == NULL ) {
                // not possible to evaluate metric with current known Metrics
                continue;
            }
            int error = luaL_loadbuffer (lua_context, rule.lua_code.c_str(), rule.lua_code.length(), "line") ||
                lua_pcall(lua_context, 0, 3, 0);
            if ( error ) {
                // syntax error in evaluate
                zsys_info("ACE1: Syntax error: %s\n", lua_tostring(lua_context, -1));
                lua_pop(lua_context, 1);  // pop error message from the stack
            } else {
                // evaluation was successful, need to read the result
                if ( !lua_isstring(lua_context, -1) )
                {
                    zsys_info ("unexcpected returned value\n");
                    lua_close (lua_context);
                    continue;
                }
                // ok, it is string. What type is it?
                const char *status = lua_tostring(lua_context,-1);
                const char *description = lua_tostring(lua_context,-3);
                if ( streq (status, "IS") )
                {
                    // the rule is TRUE, now detect if it is already up
                    auto r = isAlertOngoing(rule.rule_name, rule.element);
                    if ( r == alerts.end() ) {
                        // first time the rule fired alert for the element
                        alerts.push_back(Alert(rule, ALERT_START, ::time(NULL), description));
                        r = alerts.end() - 1 ;
                        zsys_info("RULE '%s' : ALERT started for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    } else {
                        if ( r->status == ALERT_RESOLVED ) {
                            // alert is old. This is new one
                            r->status = ALERT_START;
                            r->timestamp = ::time(NULL);
                            r->description = description;
                            zsys_info("RULE '%s' : OLD ALERT starts again for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                        }
                        else {
                            // it is the same alert
                            zsys_info("RULE '%s' : ALERT is ALREADY ongoing for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                        }
                    }
                    alert_send (client, r->rule.rule_name.c_str(), r->rule.element.c_str(), r->timestamp, get_status_string(r->status), r->rule.severity.c_str(), r->description.c_str());
                }
                else if ( streq(status,"ISNT") )
                {
                    //the rule is FALSE, now detect if it is already down
                    auto r = isAlertOngoing(rule.rule_name, rule.element);
                    if ( r != alerts.end() ) {
                        // Now there is no alert, but I remember, that one moment ago I saw some info about this alert
                        if ( r->status != ALERT_RESOLVED ) {
                            r->status = ALERT_RESOLVED;
                            r->timestamp = ::time(NULL);
                            r->description = description;
                            zsys_info("RULE '%s' : ALERT is resolved for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                            alert_send (client, r->rule.rule_name.c_str(), r->rule.element.c_str(), r->timestamp, get_status_string(r->status), r->rule.severity.c_str(), r->description.c_str());
                        }
                    }
                }
                else
                {
                    zsys_info ("unexcpected string in returned value\n");
                    lua_close (lua_context);
                    continue;
                }
            }
            lua_close(lua_context);
        }
        // Handle regex configs
        for ( const auto &rule: alertConfiguration._regexConfigs ) {
            if ( !zrex_matches (rule.rex, topic.c_str())) {
                // metric doesn't satisfied the regular expression
                continue;
            }
            lua_State *lua_context = rule.setContext(cache, type);
            if ( lua_context == NULL ) {
                // not possible to evaluate metric with current known Metrics
                continue;
            }
            int error = luaL_loadbuffer(lua_context, rule.lua_code.c_str(), rule.lua_code.length(), "line") ||
                lua_pcall(lua_context, 0, 4, 0);

            if ( error ) {
                // syntax error in evaluate
                zsys_info("ACE2: Syntax error: %s\n", lua_tostring(lua_context, -1));
                lua_pop(lua_context, 1);  // pop error message from the stack
            }
            else
            {
                // evaluation was successful, need to read the result
                if ( !lua_isstring(lua_context, -1) )
                {
                    zsys_info ("unexcpected returned value\n");
                    lua_close (lua_context);
                    continue;
                }
                // ok, it is string. What type is it?
                const char *status = lua_tostring(lua_context,-1);
                const char *description = lua_tostring(lua_context,-3);
                if ( streq (status, "IS") )
                {
                    // the rule is TRUE, now detect if it is already up
                    auto r = isAlertOngoing(rule.rule_name, lua_tostring(lua_context, -4)); /// !!!! this line is different from previous
                    if ( r == alerts.end() ) {
                        // first time the rule fired alert for the element
                        alerts.push_back(Alert(rule, ALERT_START, ::time(NULL), description));
                        alerts.back().rule.element = lua_tostring(lua_context, -4); // this line is missing in previos one
                        r = alerts.end() - 1;
                        zsys_info("RULE '%s' : ALERT started for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    } else {
                        if ( r->status == ALERT_RESOLVED ) {
                            // alert is old. This is new one
                            r->status = ALERT_START;
                            r->timestamp = ::time(NULL);
                            r->description = strdup(description);
                            zsys_info("RULE '%s': OLD ALERT starts again for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                        }
                        else {
                            // it is the same alert
                            zsys_info("RULE '%s' : ALERT is ALREADY ongoing for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                        }
                    }
                    alert_send (client, r->rule.rule_name.c_str(), r->rule.element.c_str(), r->timestamp, get_status_string(r->status), r->rule.severity.c_str(), r->description.c_str());
                }
                else if ( streq(status,"ISNT") )
                {
                    //the rule is FALSE, now detect if it is already down
                    auto r = isAlertOngoing(rule.rule_name, lua_tostring(lua_context, -4)); /// !!!! this line is different from previous
                    if ( r != alerts.end() ) {
                        // Now there is no alert, but I remember, that one moment ago I saw some info about this alert
                        if ( r->status != ALERT_RESOLVED ) {
                            r->status = ALERT_RESOLVED;
                            r->timestamp = ::time(NULL);
                            r->description = description;
                            zsys_info("RULE '%s' : ALERT is resolved for element '%s' with description '%s'\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                            alert_send (client, r->rule.rule_name.c_str(), r->rule.element.c_str(), r->timestamp, get_status_string(r->status), r->rule.severity.c_str(), r->description.c_str());
                        }
                    }
                }
                else
                {
                    zsys_info ("unexcpected string in returned value\n");
                    lua_close (lua_context);
                    continue;
                }
            }
            lua_close(lua_context);
        }
    }
    mlm_client_destroy(&client);
    return 0;
}
