#ifndef SRC_SHARED_SAMPLE_AGENT_H
#define SRC_SHARED_SAMPLE_AGENT_H

#include <sys/types.h>

#include "bios_agent.h"
#include "agents.h"

struct sample_agent {
    const char* agent_name;   //!< Name of the measuring agent
    int (*init)();            //!< Constructor 
    int (*close)();           //!< Destructor
    char **variants;          //!< Various sources to iterate over
    const char* measurement;  /*!< Printf formated string for what are we
                                   measuring, %s will be filled with source */
    const char* at;           /*!< Printf formated string for where are we
                                   measuring, %s will be filled with hostname */
    int32_t diff;             /*!< Minimum difference required for publishing */
    ymsg_t* (*get_measurement)(char* what); //!< Measuring itself
};

// extern sample_agent agent;
#endif // SRC_SHARED_SAMPLE_AGENT_H
