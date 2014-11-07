#ifndef DBINIT_H_
#define DBINIT_H_

#include "../src/persist/databaseobject.h"
#include "dbpath.h"

static const std::string osnew       = persist::objectStatetoString(persist::ObjectState::OS_NEW);
static const std::string osdeleted   = persist::objectStatetoString(persist::ObjectState::OS_DELETED);
static const std::string osupdated   = persist::objectStatetoString(persist::ObjectState::OS_UPDATED);
static const std::string osselected  = persist::objectStatetoString(persist::ObjectState::OS_SELECTED);
static const std::string osinserted  = persist::objectStatetoString(persist::ObjectState::OS_INSERTED);


#endif //DBINIT_H_
