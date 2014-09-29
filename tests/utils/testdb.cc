#include "db.h"
#include <stdio.h>
#include <iostream>
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include "log.h"

int main()
{
    log_open();
    std::string url="mysql:db=box_utf8;user=root;";
    utils::Computer comp(url);
    comp.selectById(1);   
    
    std::cout << comp.getName() << std::endl;
//    utils::ComputerDetail CD(url);
//    std::cout << CD.getUrl() << std::endl;
   // utils::DataBase mydb;//mydb
   // utils::DataBase mydb1("mytt","root","");
    log_close();
    return 0;
}
