#ifndef DBINIT_H_
#define DBINIT_H_

#include "persistence/databaseobject.h"
#include "dbpath.h"

static const std::string osnew       = utils::db::objectStatetoString(utils::db::ObjectState::OS_NEW);
static const std::string osdeleted   = utils::db::objectStatetoString(utils::db::ObjectState::OS_DELETED);
static const std::string osupdated   = utils::db::objectStatetoString(utils::db::ObjectState::OS_UPDATED);
static const std::string osselected  = utils::db::objectStatetoString(utils::db::ObjectState::OS_SELECTED);
static const std::string osinserted  = utils::db::objectStatetoString(utils::db::ObjectState::OS_INSERTED);


#endif //DBINIT_H_
