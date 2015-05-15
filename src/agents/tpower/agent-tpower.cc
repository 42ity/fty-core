#include <stdio.h>

#include "agent-tpower.h"
#include "str_defs.h"
#include "log.h"
#include "cleanup.h"

#include <iostream>
#include <string>
#include <errno.h>

void TotalPowerAgent::configuration( )
{
    _racks["R1"] = TPRack();
    _racks["R1"].name("R1");
    _racks["R1"].addPowerDevice("UPS1-LAB");
    _affects["UPS1-LAB"]="R1";
    _racks["R1"].addPowerDevice("UPS2-LAB");
    _affects["UPS2-LAB"]="R1";
}

std::string TotalPowerAgent::device()
{
    std::string subj = subject();
    size_t i = subj.find('@');
    if( i == std::string::npos ) return "";
    return subj.substr(i + 1);
}

void TotalPowerAgent::onStart( )
{
    //FIXME: remove static config
    configuration();
    _timeout = TPOWER_POLLING_INTERVAL;
}

void TotalPowerAgent::onSend( ymsg_t **message ) {
    std::cout << device() << "\n";
    std::string dev = device();
    Measurement M = *message;
    M.print();

    auto afit = _affects.find( M.deviceName() );
    if( afit != _affects.end() ) {
        // this device affects something
        auto rackit = _racks.find( afit->second );
        if( rackit != _racks.end() ) {
            // affected rack/dc found
            rackit->second.setMeasurement(M);
        }
        onPoll();
    }
    // ymsg_print(*message);
}

void TotalPowerAgent::onPoll() {
    for( auto &rack : _racks ) {
        std::cout << "rack " << rack.first << " changed: " << rack.second.changed() <<
            " known: " << rack.second.realpowerIsKnown() << "\n";
        if( rack.second.advertise() ) {
            std::cout << "sending\n";
            _scoped_ymsg_t *message = rack.second.measurementMessage();
            if( message ) {
                ymsg_print(message);
                std::string topic = "measurement.realpower.default@" + rack.second.name();
                send( topic.c_str(), &message );
                rack.second.advertised();
            }
        }
    }
}

int main(int argc, char *argv[]){
    if( argc > 0 ) {}; // silence compiler warnings
    if( argv ) {};  // silence compiler warnings
    
    int result = 1;
    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("tpower agent started");
    TotalPowerAgent agent("TPOWER");
    if( agent.connect(MLM_ENDPOINT, bios_get_stream_main(), "^measurement\\.realpower\\.default@") ) {
        result = agent.run();
    }
    log_info ("tpower agent exited with code %i\n", result);
    return result;
}
