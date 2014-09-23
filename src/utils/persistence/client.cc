#include "client.h"
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <tntdb/value.h>

namespace utils {
/////////////////////////////////////////////////////
////////////        Client    ///////////////////////
/////////////////////////////////////////////////////

int Client::selectId(std::string url, std::string name)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(url);

    tntdb::Statement st = conn.prepareCached(
        "select "
        "v.id"
        "from"
        "v_client v"
        "where v.name = :name"
        );
          
    unsigned int tmp = -1;
    // can throw a exception of type tntdb::NotFound, if the query returns no rows at all.
    try{
        tntdb::Value result = st.setString("name", name).selectValue();
        result.get(tmp);
    }
    catch (const std::exception& e){
        tmp = -1;
    }

    return tmp;
}
}
