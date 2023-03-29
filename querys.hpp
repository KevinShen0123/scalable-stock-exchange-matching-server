#include <iostream>
#include <pqxx/pqxx>
#include<sstream>
#include<cstring>
#include<fstream>
#include <iomanip>
#include <string>
using namespace pqxx;
void create_database(connection*C, std::string fileName);
connection* connect_database();
void database_init(std::string fileName);