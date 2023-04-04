#include "pugixml/pugixml.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <iostream>
#include <pqxx/pqxx>
#include<sstream>
#include<cstring>
#include<fstream>
#include <iomanip>
#include <string>
#include<vector>
#include "Order.hpp" 
#include "Account.hpp"
#include "Position.hpp"
#include<algorithm>
void parseCreateXML(connection*C,pugi::xml_node node);
void parseTransactionsXML(connection*C,pugi::xml_node node);
void parseXML(connection*C,std::string xmlstring);
