////////////////////////////////////////////////////////////////////////
// src/web/src/augtool.cpp
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

log_define("component.augtool")

// <%pre>
#line 1 "src/web/src/augtool.ecpp"

#include <cxxtools/split.h>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/regex.h>
#include <vector>
#include <string>
#include <string.h>

#include "subprocess.h"
#include "cidr.h"

using namespace shared;


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

static tnt::ComponentFactoryImpl<_component_> Factory("augtool");

static const char* rawData = "\010\000\000\000+\000\000\000{ \"error\": \"Can't start augtool\" }\n";

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
  log_trace("augtool " << qparam.getUrl());

  tnt::DataChunks data(rawData);

#line 17 "src/web/src/augtool.ecpp"
  typedef Argv exe_type;
  TNT_THREAD_GLOBAL_VAR(exe_type, exe, "Argv exe", ( { "sudo", "augtool", "-e" }));   // <%thread> Argv exe( { "sudo", "augtool", "-e" })
#line 18 "src/web/src/augtool.ecpp"
  typedef SubProcess augtool_type;
  TNT_THREAD_GLOBAL_VAR(augtool_type, augtool, "SubProcess augtool", (exe, SubProcess::STDOUT_PIPE | SubProcess::STDIN_PIPE));   // <%thread> SubProcess augtool(exe, SubProcess::STDOUT_PIPE | SubProcess::STDIN_PIPE)
  // <%cpp>
#line 20 "src/web/src/augtool.ecpp"

std::string command;
std::string nil;

// Initialization of augtool subprocess if needed
if(!augtool.isRunning()) {
    augtool.run();
    command = "help\n";
    write(augtool.getStdin(), command.c_str(), command.length());
    nil = wait_read_all(augtool.getStdout());
    if(!augtool.isRunning() || nil.find("match") == nil.npos) {

  reply.out() << data[0]; // { "error": "Can't start augtool" }\n
#line 33 "src/web/src/augtool.ecpp"

        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

// Make sure we have clear state and fresh data
command = "\n";
write(augtool.getStdin(), command.c_str(), command.length());
nil = wait_read_all(augtool.getStdout());
command = "load\n";
write(augtool.getStdin(), command.c_str(), command.length());
nil = wait_read_all(augtool.getStdout());

return DECLINED;

  // <%/cpp>
  return HTTP_OK;
}

} // namespace
