#ifndef SRC_DRIVERS_NUT_NUT_ACTOR_H_
#define SRC_DRIVERS_NUT_NUT_ACTOR_H_

#include <czmq.h>

namespace drivers
{
namespace nut
{

/**
 * \brief Actor for NUT driver
 */
void nut_actor(zsock_t *pipe, void *args);

} // namespace drivers::nut
} // namespace drivers
    
#endif // SRC_DRIVERS_NUT_ACTOR_H_

