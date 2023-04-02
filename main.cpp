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

void parseCreateXML(pugi::xml_node node) {
    for (pugi::xml_node nxt_node = node.first_child(); nxt_node; nxt_node = nxt_node.next_sibling()) {
        if (std::string(nxt_node.name()) == "account") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                std::vector<std::string> stock_acct_info;
                if (std::string(attr.name()) == "id") {
                    stock_acct_info.push_back(std::string(attr.name()));
                    stock_acct_info.push_back(std::string(attr.value()));
                    std::cout << "Account info: " << stock_acct_info[0] << ", " << stock_acct_info[1] << std::endl;
                }
                else if (std::string(attr.name()) == "balance")  {
                    stock_acct_info.push_back(std::string(attr.name()));
                    stock_acct_info.push_back(std::string(attr.value()));
                    std::cout << "Account info: " << stock_acct_info[0] << ", " << stock_acct_info[1] << std::endl;
                }
                else {
                    std::cout << "Incorrect attibute name in account node!" << std::endl;
                }
            }
        }
        else if (std::string(nxt_node.name()) == "symbol") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                std::vector<std::string> sym_info;
                if (std::string(attr.name()) == "sym") {
                    sym_info.push_back(std::string(attr.name()));
                    sym_info.push_back(std::string(attr.value()));
                    std::cout << "Symbol info: " << sym_info[0] << ", " << sym_info[1] << std::endl;
                    for (pugi::xml_node sym_acct = nxt_node.first_child(); sym_acct; sym_acct = sym_acct.next_sibling()) {
                        std::vector<std::string> sym_acct_info;
                        if (std::string(sym_acct.name()) == "account") {
                            for (pugi::xml_attribute acct_attr = sym_acct.first_attribute(); acct_attr; acct_attr = acct_attr.next_attribute()) {
                                if (std::string(acct_attr.name()) == "id") {
                                    sym_acct_info.push_back(acct_attr.name());
                                    sym_acct_info.push_back(acct_attr.value());
                                    std::cout << "Symbol account info: " << sym_acct_info[0] << ", " << sym_acct_info[1] << std::endl;
                                    std::string share_num = std::string(sym_acct.child_value());
                                    std::cout << "Account number of shares: " << share_num << std::endl;
                                }
                                else {
                                    std::cout << "Incorrect XML format!" << std::endl;
                                }
                            } 
                        }
                    }
                }
                else {
                    std::cout << "Incorrect attibute name in symbol node!" << std::endl;
                }
            } 
        }
        else {
            std::cout << "Incorrect XML format!" << std::endl;
        }
        }    
}

void parseTransactionsXML(pugi::xml_node node) {
    
}

void parseXML(std::string xmlstring) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xmlstring.c_str());
    if (!result) {
        std::cout << "Parsing failed!" << std::endl;
    }
    
    if (doc.child("create")) {
        pugi::xml_node node_create = doc.child("create");
        parseCreateXML(node_create);
    }
    else if (doc.child("transactions")) {
        pugi::xml_node node_transactions = doc.child("transactions");
        parseTransactionsXML(node_transactions);
    }
    else {
        std::cout << "Incorrect XML format!" << std::endl;
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
        parseXML(xmlstring);
        xmlfile.close();
    }
    else {
        std::cout << "Unable to open the XML file." << std::endl;
    }
    
    

    return 0;
}
