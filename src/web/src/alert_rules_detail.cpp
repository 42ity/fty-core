////////////////////////////////////////////////////////////////////////
// src/web/src/alert_rules_detail.cpp
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

log_define("component.alert_rules_detail")

// <%pre>
#line 25 "src/web/src/alert_rules_detail.ecpp"

#include <cxxtools/jsondeserializer.h>
#include <cxxtools/regex.h>
#include <vector>
#include <string>
#include <malamute.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "log.h"
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

static tnt::ComponentFactoryImpl<_component_> Factory("alert_rules_detail");

static const char* rawData = "\010\000\000\000\t\000\000\000\n";

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
  log_trace("alert_rules_detail " << qparam.getUrl());

  tnt::DataChunks data(rawData);

  // <%cpp>
#line 38 "src/web/src/alert_rules_detail.ecpp"

// sanity check
if (!request.isMethodGET ())
    http_die ("method-not-allowed", request.getMethod ().c_str ());

std::string name = request.getArg ("name");
if (name.empty ())
    http_die ("request-param-bad", "name", "empty string",
        "non-empty valid rule name. For a list of all available rule "
        "names please perform GET alerts/rules.");

log_debug ("requested rule name = '%s'.", name.c_str ());
// connect to malamute
mlm_client_t *client = mlm_client_new ();
if (!client) {
    log_critical ("mlm_client_new() failed.");
    http_die ("internal-error", "mlm_client_new() failed.");
}

std::string client_name ("alert_rules.");
client_name.append (std::to_string (getpid ())).append (".").append (std::to_string (syscall (SYS_gettid)));
log_debug ("malamute client name = '%s'.", client_name.c_str ());

int rv = mlm_client_connect (client, MLM_ENDPOINT, 1000, client_name.c_str ());
if (rv == -1) {
    log_critical ("mlm_client_connect (endpoint = '%s', timeout = '%d', address = '%s') failed.",
                    MLM_ENDPOINT, 1000, client_name.c_str ());
    http_die ("internal-error", "mlm_client_connect() failed.");
}

// prepare rfc-evaluator-rules GET message
zmsg_t *send_msg = zmsg_new ();
if (!send_msg) {
    log_critical ("zmsg_new() failed.");
    http_die ("internal-error", "zmsg_new() failed.");
}
zmsg_addstr (send_msg, "GET");
zmsg_addstr (send_msg, name.c_str ());

// send it
if (mlm_client_sendto (client, BIOS_AGENT_NAME_ALERT_AGENT, "rfc-evaluator-rules", NULL, 1000, &send_msg) != 0) {
    log_debug ("mlm_client_sendto (address = '%s', subject = '%s', tracker = NULL, timeout = '%d') failed.",
        BIOS_AGENT_NAME_ALERT_AGENT, "rfc-evaluator-rules", 1000);
    zmsg_destroy (&send_msg);
    mlm_client_destroy (&client);
    http_die ("internal-error", "mlm_client_sendto() failed.");
}

zmsg_t *recv_msg = NULL;
// TODO: blocking mlm_client_recv
while (true) {
    recv_msg = mlm_client_recv (client);
    if (!recv_msg) {
        log_error ("mlm_client_recv() failed.");
        mlm_client_destroy (&client);
        http_die ("internal-error", "mlm_client_recv() failed.");
    }
    if (streq (mlm_client_sender (client), BIOS_AGENT_NAME_ALERT_AGENT))
        break;
    zmsg_destroy (&recv_msg);
}
// Got it
// Check subject
if (!streq (mlm_client_subject (client), "rfc-evaluator-rules")) {
    log_error ("Unexpected reply from '%s'. Subject expected = '%s', received = '%s'.",
        mlm_client_sender (client), "rfc-evaluator-rules", mlm_client_subject (client));
    zmsg_destroy (&recv_msg);
    mlm_client_destroy (&client);
    http_die ("internal-error", "Bad message.");
}
// Check command. Can be OK or ERROR
char *part = zmsg_popstr (recv_msg);
if (streq (part, "OK")) {
    free (part);
    part = zmsg_popstr (recv_msg);
    if (!part) {
        log_error ("Unexpected reply from '%s'. Expected OK/json. Got OK/(null).", mlm_client_sender (client));
        zmsg_destroy (&recv_msg);
        mlm_client_destroy (&client);
        http_die ("internal-error", "Bad message.");
    }
// Note: Assumption: EVALUATOR returns valid json

#line 121 "src/web/src/alert_rules_detail.ecpp"
  reply.out() << ( part );
  reply.out() << data[0]; // \n
#line 122 "src/web/src/alert_rules_detail.ecpp"

    free (part);
    zmsg_destroy (&recv_msg);
    mlm_client_destroy (&client);
    return HTTP_OK;
}
if (streq (part, "ERROR")) {
    free (part);
    part = zmsg_popstr (recv_msg);
    if (!part) {
        log_error ("Unexpected reply from '%s'. Expected ERROR/reason. Got ERROR/(null).", mlm_client_sender (client));
        zmsg_destroy (&recv_msg);
        mlm_client_destroy (&client);
        http_die ("internal-error", "Bad message.");
    }
    if (streq (part, "NOT_FOUND")) {
        free (part);
        log_error ("Rule name '%s' does not exist.", name.c_str ());
        zmsg_destroy (&recv_msg);
        mlm_client_destroy (&client);
        http_die ("not-found", std::string ("Rule name '").append (name).append ("'").c_str ());
    }
    log_error ("%s", part);
    std::string reason = part;
    free (part);
    zmsg_destroy (&recv_msg);
    mlm_client_destroy (&client);
    http_die ("internal-error",
        std::string ("Error while retrieving details of rule name = '").append (name).append ("': ").
        append (reason).append(".").c_str ());
}
// Message does not conform to protocol
free (part);
log_error ("Unexptected reply from  '%s'. Does not conform to rfc-evaluator-rules.",
    mlm_client_sender (client));
zmsg_destroy (&recv_msg);
mlm_client_destroy (&client);
http_die ("internal-error", "Bad message.");

  // <%/cpp>
  return HTTP_OK;
}

} // namespace
