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

// #include <climits>
// #include <ctime>
// #include <error.h>
// #include <mutex>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <unistd.h>


void parseXML(std::string xmlstring) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xmlstring.c_str());
    if (!result) {
        std::cout << "Parsing failed." << std::endl;
    }
    

    if (doc.child("create")) {
        pugi::xml_node node_create = doc.child("create");
        std::vector<std::pair<std::string, std::string> > res;
        for (pugi::xml_node node = node_create.first_child(); node; node = node.next_sibling()) {
            std::vector<std::string> acct_info;
            std::vector<std::string> sym_info;
            for (pugi::xml_attribute attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "id") {
                    acct_info.push_back(std::string(attr.name()));
                    acct_info.push_back(std::string(attr.value()));
                }
                else if (std::string(attr.name()) == "balance") {
                    acct_info.push_back(std::string(attr.name()));
                    acct_info.push_back(std::string(attr.value()));
                    std::cout << "Account info: " << acct_info[0] << ", " << acct_info[1] << ", " << acct_info[2] << ", " << acct_info[3] << std::endl;
                }
                else if (std::string(attr.name()) == "sym") {
                    sym_info.push_back(std::string(attr.name()));
                    sym_info.push_back(std::string(attr.value()));
                    std::cout << "Symbol info: " << sym_info[0] << ", " << sym_info[1] << std::endl;
                }
                else {
                    std::cout << "Incorrect XML format!" << std::endl;
                }
            }
        }
    }
    

        

}

int main() {
    // read an XML file as a string
    std::ifstream xmlfile;
    std::string line;
    std::string xmlstring = "";
    xmlfile.open("xmlcreate.txt");
    if (xmlfile.is_open()) {
        while (getline(xmlfile, line)) {
            xmlstring += line;
        }
        // std::cout << xmlstring << std::endl;
        parseXML(xmlstring);
        xmlfile.close();
    }
    else {
        std::cout << "Unable to open the XML file." << std::endl;
    }
    
    

    return 0;
}
