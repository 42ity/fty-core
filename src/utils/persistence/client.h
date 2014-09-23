#pragma once

#ifndef MODULE_H_
#define MODULE_H_

#include "databaseobject.h"

namespace utils{

class Client: public DataBaseObject
{
    public:

        Client(std::string url);

        unsigned int selectByName(std::string name);

        unsigned int selectById(int id);
        
        std::string getName();

        void setName();

        ~Client();
        
        /**
         * \brief Selects an Id by name in the DB specified by url
         *
         * Selects an Id by name in the DB specified by url
         * \return an ID or -1 if nothing was found
         */
        static int selectId(std::string url, std::string name);

    protected:

    private:
        std::string name;
};
}

#endif //MODULE_H_
