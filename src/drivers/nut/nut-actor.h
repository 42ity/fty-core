#ifndef _SRC_DRIVERS_NUT_ACTOR_H_
#define _SRC_DRIVERS_NUT_ACTOR_H_

#include <czmq.h>

/**
 * \brief Actor for NUT driver
 */
void nut_actor(zsock_t *pipe, void *args);

    
#endif // _SRC_DRIVERS_NUT_ACTOR_H_
