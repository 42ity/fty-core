////////////////////////////////////////////////////////////////////////
// src/web/src/thresholds.cpp
// generated with ecppc
//

#include <tnt/ecpp.h>
#include <tnt/convert.h>
#include <tnt/httprequest.h>
#include <tnt/httpreply.h>
#include <tnt/httpheader.h>
#include <tnt/http.h>
#include <tnt/data.h>
#include <tnt/componentfactory.h>
#include <cxxtools/log.h>
#include <stdexcept>

log_define("component.thresholds")

// <%pre>
#line 25 "src/web/src/thresholds.ecpp"

#include <cxxtools/jsondeserializer.h>
#include <cxxtools/regex.h>
#include <vector>
#include <string>
#include <malamute.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "utils_web.h"
#include "str_defs.h"


// </%pre>

namespace
{
class _component_ : public tnt::EcppComponent
{
    _component_& main()  { return *this; }

  protected:
    ~_component_();

  public:
    _component_(const tnt::Compident& ci, const tnt::Urlmapper& um, tnt::Comploader& cl);

    unsigned operator() (tnt::HttpRequest& request, tnt::HttpReply& reply, tnt::QueryParams& qparam);
};

static tnt::ComponentFactoryImpl<_component_> Factory("thresholds");

static const char* rawData = "(\000\000\000\?\000\000\000G\000\000\000H\000\000\000T\000\000\000]\000\000\000b\000\000"
  "\000m\000\000\000r\000\000\000s\000\000\000{\n    \"thresholds\" : [\n        \n    ]\n}\n        OK:  \n        ERRO"
  "R: \n    \n";

// <%shared>
// </%shared>

// <%config>
// </%config>

#define SET_LANG(lang) \
     do \
     { \
       request.setLang(lang); \
       reply.setLocale(request.getLocale()); \
     } while (false)

_component_::_component_(const tnt::Compident& ci, const tnt::Urlmapper& um, tnt::Comploader& cl)
  : EcppComponent(ci, um, cl)
{
  // <%init>
  // </%init>
}

_component_::~_component_()
{
  // <%cleanup>
  // </%cleanup>
}

unsigned _component_::operator() (tnt::HttpRequest& request, tnt::HttpReply& reply, tnt::QueryParams& qparam)
{
  log_trace("thresholds " << qparam.getUrl());

  tnt::DataChunks data(rawData);

  // <%cpp>
#line 38 "src/web/src/thresholds.ecpp"


if (request.getMethod () != "GET" && request.getMethod () != "POST" && request.getMethod () != "PUT")
    http_die ("internal-error", "TODO method not allowed");    // TODO: Unallowed method

mlm_client_t *client = mlm_client_new ();
if (!client) {
    log_fatal ("mlm_client_new() failed.");
    http_die ("internal-error", "mlm_client_new () failed.");
}

std::string client_name ("thresholds.");
client_name.append (std::to_string (getpid ())).append (".").append (std::to_string (syscall (SYS_gettid)));

int rv = mlm_client_connect (client, MLM_ENDPOINT, 1000, client_name.c_str ());
if (rv == -1) {
// TODO rewrite
//    log_error ("mlm_client_connect (endpoint = '%s', timeout = '%d', address = '%s'",
//                MLM_ENDPOINT, 1000, client_name.c_str ());
    http_die ("internal-error", "mlm_client_connect () failed.");
}

zmsg_t *send_msg = zmsg_new ();
if (!send_msg) {
    log_fatal ("zmsg_new() failed.");
    http_die ("internal-error", "zmsg_new () failed.");
}

std::string request_op;

if (request.isMethodGET ()) {
    request_op = "LIST";
    zmsg_addstr (send_msg, request_op.c_str ());

}
else {
    std::string name = request.getArg ("name");
    cxxtools::SerializationInfo si;
    try {
        std::stringstream input (request.getBody(), std::ios_base::in); 
        cxxtools::JsonDeserializer deserializer (input);
        deserializer.deserialize (si);        
    }
    catch (const std::exception& e) {
        // TODO: http_die
        http_die ("internal-error", "Error deserializing request document.");
    }
    // json is valid
    std::string json_name, severity, metric, type, element, value;
    try {
        si.getMember ("name") >>= json_name;
        si.getMember ("severity") >>= severity;
        si.getMember ("metric") >>= metric;
        si.getMember ("type") >>= type;
        si.getMember ("element") >>= element;
        si.getMember ("value") >>= value;
        if (name.empty () || severity.empty () || metric.empty () ||
            type.empty () || element.empty () || value.empty ())
            throw std::exception ();
    }
    catch (const std::exception& e) {
        // TODO: http_die
        http_die ("internal-error", "Request document not according to rfc-11.");
    }
    if (request.getMethod () == "PUT" && !name.empty () && name.compare (json_name) != 0) {
        // TODO: http_die names are not equal
        http_die ("internal-error", "Request document not according to rfc-11.");
    }
    request_op = "ADD";
    zmsg_addstr (send_msg, request_op.c_str ());
    zmsg_addstr (send_msg, request.getBody().c_str ());

}

if (mlm_client_sendto (client, "alert_agent", "rfc-thresholds", NULL, 1000, &send_msg) != 0) { // TODO rewrite address to str_defs
// TODO rewrite
//        log_debug ("mlm_client_sendto (address = '%s', subject = '%s', tracker = NULL, timeout = '%d') failed.",
//                    "alert_agent", "rfc-thresholds", 1000);
    zmsg_destroy (&send_msg);
    mlm_client_destroy (&client);
    http_die ("internal-error", "mlm_client_sendto () failed.");
}
// wait for reply; TODO: wait in cycle and check sender throw away potential troll senders
zmsg_t *recv_msg = mlm_client_recv (client);
if (!recv_msg) {
    log_error ("mlm_client_recv () failed.");
    mlm_client_destroy (&client);
    http_die ("internal-error", "mlm_client_recv () failed.");
}
// Got it
char *part = zmsg_popstr (recv_msg);
if (streq (part, "LIST")) {
    
  reply.out() << data[0]; // {\n    "thresholds" : [\n
#line 133 "src/web/src/thresholds.ecpp"
    while (part) {

  reply.out() << data[1]; //         
#line 134 "src/web/src/thresholds.ecpp"
  reply.out() << ( part );
  reply.out() << data[2]; // \n
#line 135 "src/web/src/thresholds.ecpp"
       free (part);

#line 136 "src/web/src/thresholds.ecpp"
       part = zmsg_popstr (recv_msg);

#line 137 "src/web/src/thresholds.ecpp"
    }

  reply.out() << data[3]; //     ]\n}\n    
#line 140 "src/web/src/thresholds.ecpp"

} else if (streq (part, "OK"))  {
    
  reply.out() << data[4]; //     OK:  
#line 143 "src/web/src/thresholds.ecpp"
  reply.out() << ( zmsg_popstr (recv_msg) );
  reply.out() << data[5]; // \n    
#line 144 "src/web/src/thresholds.ecpp"

} else if (streq (part, "ERROR")) {
    
  reply.out() << data[6]; //     ERROR: 
#line 147 "src/web/src/thresholds.ecpp"
  reply.out() << ( zmsg_popstr (recv_msg) );
  reply.out() << data[7]; // \n    
#line 148 "src/web/src/thresholds.ecpp"

} else {
    http_die ("internal-error", "Unexpected message."); // TODO http_die
}
return HTTP_OK;

  reply.out() << data[8]; // \n
  // <%/cpp>
  return HTTP_OK;
}

} // namespace
