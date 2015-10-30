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


#define ALERT_DOWN 0
#define ALERT_UP 1

#define ALERT_START 1
#define ALERT_ACK1  2
#define ALERT_ACK2  3
#define ALERT_ACK3  4
#define ALERT_ACK4  5
#define ALERT_RESOLVED 6
struct Rule {
    std::vector<std::string> in;
    zrex_t *rex;
    std::string rex_str;
    std::string lua_code;
    std::string out;
    std::string rule_name;
    std::string element;
    std::string severity;
};


struct Alert {
    Rule rule;
    int status ; // on Off ack
    int64_t timestamp;
    std::string description;

    Alert (Rule r, int s, int64_t tm, std::string descr){
        rule = r;
        status = s;
        timestamp = tm;
        description = descr;
    };
};

std::vector<Alert> alerts;

std::map<std::string, double> cache;

std::map<std::string, std::vector<Rule> > configs;

std::vector<Rule> r_configs;

Rule read_rule(std::ifstream &f) {
    // try catch TODO
    Rule rule;
    cxxtools::JsonDeserializer json(f);
    json.deserialize();
    const cxxtools::SerializationInfo *si = json.si();
    si->getMember("evaluation") >>= rule.lua_code;
    si->getMember("rule_name") >>= rule.rule_name;
    si->getMember("severity") >>= rule.severity;
    if ( si->findMember("in") ) {
        si->getMember("in") >>= rule.in;
        si->getMember("out") >>= rule.out;
        si->getMember("element") >>= rule.element;
    }
    // can un and in_rex be both at the same file?
    else {
        if ( si->findMember("in_rex") ) {
            si->getMember("in_rex") >>= rule.rex_str;
            rule.rex = zrex_new(rule.rex_str.c_str());
        }
    }
    si->getMember("out") >>= rule.out;
    return rule;
};

std::vector<Alert>::iterator isAlertOngoing( std::string rule_name, std::string element)
{
    for(std::vector<Alert>::iterator it = alerts.begin(); it != alerts.cend(); ++it)
    {
        if ( it->rule.rule_name == rule_name && it->rule.element == element )
        {
            return it;
        }
    }
    return alerts.end();
};

int main (int argc, char** argv) {
    char buff[256];
    int error;

    mlm_client_t *client = mlm_client_new();
    mlm_client_connect (client, "ipc://@/malamute", 1000, argv[0]);

    // Read configuration
    cxxtools::Directory d(".");
    for(auto fn : d) {
        if (fn.length() < 5)
            continue;
        if (fn.compare(fn.length() - 5, 5, ".rule") !=0 )
            continue;
        std::ifstream f(fn);
        Rule rule = read_rule (f);
        rule.rule_name = fn;

        // Subscribe to all streams
        // put a copy of configuration for every interested topic
        for ( const auto &interestedTopic : rule.in ) {
            mlm_client_set_consumer(client, "BIOS", interestedTopic.c_str());
            printf("Registered to receive '%s'\n", interestedTopic.c_str());
            if ( configs.find ( interestedTopic.c_str() ) == configs.cend () ) {
                configs.emplace (interestedTopic.c_str(), std::vector<Rule>({}));
            }
            auto cit = configs.find( interestedTopic.c_str());
            cit->second.push_back(rule);
        }
        // Subscribe to all streams
        if ( rule.rex != NULL ) {
            mlm_client_set_consumer(client, "BIOS", rule.rex_str.c_str());
            printf("Registered to receive '%s'\n", rule.rex_str.c_str());
            r_configs.push_back(rule);
        }
    }
    mlm_client_set_producer(client, "ALERTS");

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
            printf ("cannot decode metric, ignore message\n");
            continue;
        }
        char *end;
        double dvalue = strtod (value, &end);
        if (errno == ERANGE) {
            errno = 0;
            printf ("cannot convert to double, ignore message\n");
            continue;
        }
        else if (end == value || *end != '\0') {
            printf ("cannot convert to double, ignore message\n");
            continue;
        }

        std::string topic = mlm_client_subject(client);
        printf("Got message '%s' with value %s\n", topic.c_str(), value);

        // Update cache with new value
        auto it = cache.find (topic);
        if ( it != cache.cend() ) {
            it->second = dvalue;
        }
        else {
            cache.emplace (topic, dvalue);
        }

        // Handle non-regex configs
        // co kdyz neni nalezeno?
        for ( const auto &rule : configs.find(topic)->second) {
            // Compute
            lua_State *lua_context = lua_open();
            bool haveAll = true;
            for ( const auto &neededTopic : rule.in) {
                auto it = cache.find(neededTopic);
                if ( it == cache.cend() ) {
                    printf("Do not have everything for '%s' yet\n", rule.rule_name.c_str());
                    haveAll = false;
                    break;
                }
                std::string var = neededTopic;
                var[var.find('@')] = '_';
                printf("Setting variable '%s' to %lf\n", var.c_str(), it->second);
                lua_pushnumber(lua_context, it->second);
                lua_setglobal(lua_context, var.c_str());
            }
            if ( !haveAll ) {
                lua_close (lua_context);
                continue;
            }

            error = luaL_loadbuffer (lua_context, rule.lua_code.c_str(), rule.lua_code.length(), "line") ||
                       lua_pcall(lua_context, 0, 2, 0);

            if ( error ) {
                // syntax error in evaluate
                fprintf(stderr, "%s", lua_tostring(lua_context, -1));
                lua_pop(lua_context, 1);  /* pop error message from the stack */
            } else if( lua_isnumber(lua_context, -1) ){
                // the rule is TRUE, now detect if it is already up
                auto r = isAlertOngoing(rule.rule_name, rule.element);
                if ( r == alerts.end() ) {
                    // first time the rule fired alert for the element
                    fprintf(stdout, "RULE %s : ALERT IS UP (%s = %lf), %s\n", rule.rule_name.c_str(), rule.out.c_str(), lua_tonumber(lua_context, -1), lua_tostring(lua_context, -2));
                    alerts.push_back(Alert(rule, ALERT_START, ::time(NULL), lua_tostring(lua_context, -2)));
                } else {
                    if ( r->status == ALERT_RESOLVED ) {
                        // alert is old. This is new one
                        r->status = ALERT_START;
                        r->timestamp = ::time(NULL);
                        r->description = lua_tostring(lua_context, -2);
                        fprintf(stdout, "RULE %s : OLD ALERT starts again for element %s with description %s\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    }
                    else {
                        // it is the same alert
                        fprintf(stdout, "RULE %s : ALERT is ALREADY ongoing for element %s with description %s\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    }
                }
            } else {
                //the rule is FALSE, now detect if it is already down
                auto r = isAlertOngoing(rule.rule_name, rule.element);
                if ( r != alerts.end() ) {
                    // Now there is no alert, but I remember, that one moment ago I saw some info about this alert
                    if ( r->status != ALERT_RESOLVED ) {
                        r->status = ALERT_RESOLVED;
                        r->timestamp = ::time(NULL);
                        fprintf(stdout, "RULE %s : ALERT is resolved for element %s\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    }
                }
            }
            lua_close(lua_context);
        }

        // Handle regex configs
        for ( const auto &rule: r_configs ) {
            if ( !zrex_matches (rule.rex, topic.c_str())) {
                // metric doesn't satisfied the regular expression
                continue;
            }
            // Compute
            lua_State *lua_context = lua_open();
            lua_pushnumber(lua_context, dvalue);
            lua_setglobal(lua_context, "value");
            lua_pushstring(lua_context, topic.c_str());
            lua_setglobal(lua_context, "topic");
            lua_pushstring(lua_context, element_src);
            lua_setglobal(lua_context, "element");
            error = luaL_loadbuffer(lua_context, rule.lua_code.c_str(), rule.lua_code.length(), "line") ||
                lua_pcall(lua_context, 0, 3, 0);

           if ( error ) {
                // syntax error in evaluate
                fprintf(stderr, "%s", lua_tostring(lua_context, -1));
                lua_pop(lua_context, 1);  /* pop error message from the stack */
            } else if( lua_isnumber(lua_context, -1) ){
                // the rule is TRUE, now detect if it is already up
                auto r = isAlertOngoing(rule.rule_name, lua_tostring(lua_context, -4)); /// !!!! this line is different from previous
                if ( r == alerts.end() ) {
                    // first time the rule fired alert for the element
                    fprintf(stdout, "RULE %s : ALERT IS UP (%s = %lf), %s\n", rule.rule_name.c_str(), rule.out.c_str(), lua_tonumber(lua_context, -1), lua_tostring(lua_context, -2));
                    alerts.push_back(Alert(rule, ALERT_START, ::time(NULL), lua_tostring(lua_context, -2)));
                    alerts.back().rule.element = lua_tostring(lua_context, -4); // this line is missing in previos one
                } else {
                    if ( r->status == ALERT_RESOLVED ) {
                        // alert is old. This is new one
                        r->status = ALERT_START;
                        r->timestamp = ::time(NULL);
                        r->description = lua_tostring(lua_context, -2);
                        fprintf(stdout, "RULE %s : OLD ALERT starts again for element %s with description %s\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    }
                    else {
                        // it is the same alert
                        fprintf(stdout, "RULE %s : ALERT is ALREADY ongoing for element %s with description %s\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    }
                }
            } else {
                //the rule is FALSE, now detect if it is already down
                auto r = isAlertOngoing(rule.rule_name, rule.element);
                if ( r != alerts.end() ) {
                    // Now there is no alert, but I remember, that one moment ago I saw some info about this alert
                    if ( r->status != ALERT_RESOLVED ) {
                        r->status = ALERT_RESOLVED;
                        r->timestamp = ::time(NULL);
                        fprintf(stdout, "RULE %s : ALERT is resolved for element %s\n", r->rule.rule_name.c_str(), r->rule.element.c_str(), r->description.c_str());
                    }
                }
            }
            lua_close(lua_context);
        }

    }
    mlm_client_destroy(&client);
    return 0;
}
