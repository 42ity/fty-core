#ifndef DBINIT_H_
#define DBINIT_H_

#include "databaseobject.h"
#include "dbpath.h"

static const std::string osnew       = utils::objectStatetoString(utils::ObjectState::OS_NEW);
static const std::string osdeleted   = utils::objectStatetoString(utils::ObjectState::OS_DELETED);
static const std::string osupdated   = utils::objectStatetoString(utils::ObjectState::OS_UPDATED);
static const std::string osselected  = utils::objectStatetoString(utils::ObjectState::OS_SELECTED);
static const std::string osinserted  = utils::objectStatetoString(utils::ObjectState::OS_INSERTED);


#endif //DBINIT_H_
