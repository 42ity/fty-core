#include "agentstate_hl.h"
#include "dbpath.h"
#include "log.h"

int save_agent_info(tntdb::Connection &conn, const std::string &agent_name, const std::string &data)
{
    uint16_t rows;
    
    return update_agent_info( conn, agent_name, (void *)data.c_str(), data.size(), rows);
}

int save_agent_info(const std::string &agent_name, const std::string &data)
{
    int result = 1;
    try {
        auto connection = tntdb::connect(url);
        result = save_agent_info(connection, agent_name, data );
        connection.close();
    } catch( const std::exception &e ) {
        log_error("Cannot save agent %s state: %s", agent_name.c_str(), e.what() );
    }
    return result;
}

std::string load_agent_info(tntdb::Connection &conn, const std::string &agent_name)
{
    char *data, *data2;
    size_t size;
    std::string result;

    if( select_agent_info(conn, agent_name, (void **)&data, size) == 0 ) {
        if( ! data ) return result;
        data2 = (char *)realloc( data, size + 1 );
        if( data2 ) {
            data2[size] = 0;
            result = data2;
            free(data2);
        } else {
            free(data);
        }
    }
    return result;
}


std::string load_agent_info(const std::string &agent_name)
{
    std::string result;
    try {
        auto connection = tntdb::connect(url);
        result = load_agent_info(connection, agent_name);
        connection.close();
    } catch( const std::exception &e ) {
        log_info("Cannot load agent %s state: %s", agent_name.c_str(), e.what() );
    }
    return result;
}
