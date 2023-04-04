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
    // generate response XML
    pugi::xml_document create_response_doc;
    pugi::xml_node create_result = create_response_doc.append_child("results");

    for (pugi::xml_node nxt_node = node.first_child(); nxt_node; nxt_node = nxt_node.next_sibling()) {
        std::vector<std::string> stock_acct_info;
        std::vector<std::string> sym_info;
        if (std::string(nxt_node.name()) == "account") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "id") {
                    stock_acct_info.push_back(std::string(attr.name()));
                    stock_acct_info.push_back(std::string(attr.value()));
                }
                else if (std::string(attr.name()) == "balance")  {
                    stock_acct_info.push_back(std::string(attr.name()));
                    stock_acct_info.push_back(std::string(attr.value()));
                    std::cout << "Account info: " << stock_acct_info[0] << ", " << stock_acct_info[1] << ", " << stock_acct_info[2] << ", " << stock_acct_info[3] << std::endl;
                }
                else {
                    std::cout << "Incorrect attibute name in account node!" << std::endl;
                }
            }
            
            // on success, add <created> node for account create
            pugi::xml_node acct_created = create_result.append_child("created");
            std::string create_acct_id = "0123455";    // account id from database
            acct_created.append_attribute("id") = create_acct_id.c_str();

            // on failure, add <error> node for account create
            pugi::xml_node acct_error = create_result.append_child("error");
            std::string error_acct_id = "0123455";    // account id from database
            acct_error.append_attribute("id") = error_acct_id.c_str();
            std::string acct_error_msg = "Failed to create account!";
            acct_error.append_child(pugi::node_pcdata).set_value(acct_error_msg.c_str());
        }
        else if (std::string(nxt_node.name()) == "symbol") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
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

            // on success, add <created> node for symbol create
            pugi::xml_node sym_created = create_result.append_child("created");
            std::string create_sym = "SYM";    // symbol from database
            std::string create_sym_id = "0123455";    // account id from database
            sym_created.append_attribute("sym") = create_sym.c_str();
            sym_created.append_attribute("id") = create_sym_id.c_str();

            // on failure, add <error> node for symbol create
            pugi::xml_node sym_error = create_result.append_child("error");
            std::string error_sym = "SYM";    // symbol from database
            std::string error_sym_id = "0123455";    // account id from database
            sym_error.append_attribute("sym") = error_sym.c_str();
            sym_error.append_attribute("id") = error_sym_id.c_str();
            std::string sym_error_msg = "Failed to create symbol!";
            sym_error.append_child(pugi::node_pcdata).set_value(sym_error_msg.c_str());
        }
        else {
            std::cout << "Incorrect XML format!" << std::endl;
        }
    }
    std::stringstream create;
    create_response_doc.save(create);
    std::string create_response = create.str();
}

void parseTransactionsXML(pugi::xml_node node) {
    // generate response XML
    pugi::xml_document trans_response_doc;
    pugi::xml_node trans_result = trans_response_doc.append_child("results");

    if (std::string(node.first_attribute().name()) == "id") {
        std::string acct_id = std::string(node.first_attribute().value());
        std::cout << "Transaction account id: " << acct_id << std::endl;
    }
    else {
        std::cout << "Incorrect XML format!" << std::endl;
    }
    
    for (pugi::xml_node nxt_node = node.first_child(); nxt_node; nxt_node = nxt_node.next_sibling()) {
        std::vector<std::string> order_info;
        std::vector<std::string> query_info;
        std::vector<std::string> cancel_info;
        if (std::string(nxt_node.name()) == "order") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "sym") {
                    order_info.push_back(std::string(attr.name()));
                    order_info.push_back(std::string(attr.value()));
                }
                else if (std::string(attr.name()) == "amount") {
                    order_info.push_back(std::string(attr.name()));
                    order_info.push_back(std::string(attr.value()));
                }
                else if (std::string(attr.name()) == "limit") {
                    order_info.push_back(std::string(attr.name()));
                    order_info.push_back(std::string(attr.value()));
                    std::cout << "Order info: " << order_info[0] << ", " << order_info[1] << ", "<< order_info[2] << ", "<< order_info[3] << ", "<< order_info[4] << ", "<< order_info[5] << std::endl;
                }
                else {
                    std::cout << "Incorrect XML format!" << std::endl;
                }
            }

            // on success, add <opened> node for open order
            pugi::xml_node order_opened = trans_result.append_child("opened");
            std::string opened_sym = "SYM";    // symbol from database
            std::string opened_amount = "100";    // amount id from database
            std::string opened_limit = "125";    // limit id from database
            std::string opened_trans_id = "1";    // transaction id from database
            order_opened.append_attribute("sym") = opened_sym.c_str();
            order_opened.append_attribute("amount") = opened_amount.c_str();
            order_opened.append_attribute("limit") = opened_limit.c_str();
            order_opened.append_attribute("id")  = opened_trans_id.c_str();

            // on failure, add <error> node for open order
            pugi::xml_node order_error = trans_result.append_child("error");
            std::string error_sym = "SYM";    // symbol from database
            std::string error_amount = "100";    // amount id from database
            std::string error_limit = "125";    // limit id from database
            order_error.append_attribute("sym") = error_sym.c_str();
            order_error.append_attribute("amount") = error_amount.c_str();
            order_error.append_attribute("limit") = error_limit.c_str();
            std::string order_error_msg = "Failed to open the order!";
            order_error.append_child(pugi::node_pcdata).set_value(order_error_msg.c_str());
        }
        else if (std::string(nxt_node.name()) == "query") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "id") {
                    query_info.push_back(std::string(attr.name()));
                    query_info.push_back(std::string(attr.value()));
                    std::cout << "Query info: " << query_info[0] << ", " << query_info[1] << std::endl;
                }
                else {
                    std::cout << "Incorrect XML format!" << std::endl;
                }
            }

            // on success, add <status> node for query
            pugi::xml_node query = trans_result.append_child("status");
            std::string query_trans_id = "1";    // transaction id from database
            std::string query_open_share = "100";    // from database
            std::string query_cancel_share = "100";    // from database
            std::string query_cancel_time = "123456789";    // from database
            std::string query_execute_share = "100";    // from database
            std::string query_execute_price = "125";    // from database
            std::string query_execute_time = "123456789";    // from database
            query.append_attribute("id") = query_trans_id.c_str();
            pugi::xml_node open = query.append_child("open");
            open.append_attribute("shares") = query_open_share.c_str();
            pugi::xml_node query_canceled = query.append_child("canceled");
            query_canceled.append_attribute("shares") = query_cancel_share.c_str();
            query_canceled.append_attribute("time") = query_cancel_time.c_str();
            pugi::xml_node query_executed = query.append_child("executed");
            query_executed.append_attribute("shares") = query_execute_share.c_str();
            query_executed.append_attribute("price") = query_execute_price.c_str();
            query_executed.append_attribute("time") = query_execute_time.c_str();

            // on failure, add <error> node for query
            pugi::xml_node query_error = trans_result.append_child("error");
            std::string error_query_trans_id = "1";    // transaction id from database
            std::string query_error_msg = "Failed to query the order!";
            query_error.append_child(pugi::node_pcdata).set_value(query_error_msg.c_str());
        }
        else if (std::string(nxt_node.name()) == "cancel") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "id") {
                    cancel_info.push_back(std::string(attr.name()));
                    cancel_info.push_back(std::string(attr.value()));
                    std::cout << "Cancel info: " << cancel_info[0] << ", " << cancel_info[1] << std::endl;
                }
                else {
                    std::cout << "Incorrect XML format!" << std::endl;
                }
            }

            // on success, add <canceled> node for cancel
            pugi::xml_node cancel = trans_result.append_child("canceled");
            std::string cancel_trans_id = "1";    // transaction id from database
            std::string cancel_cancel_share = "100";    // from database
            std::string cancel_cancel_time = "123456789";    // from database
            std::string cancel_execute_share = "100";    // from database
            std::string cancel_execute_price = "125";    // from database
            std::string cancel_execute_time = "123456789";    // from database
            cancel.append_attribute("id") = cancel_trans_id.c_str();
            pugi::xml_node cancel_canceled = cancel.append_child("canceled");
            cancel_canceled.append_attribute("shares") = cancel_cancel_share.c_str();
            cancel_canceled.append_attribute("time") = cancel_cancel_time.c_str();
            pugi::xml_node cancel_executed = cancel.append_child("executed");
            cancel_executed.append_attribute("shares") = cancel_execute_share.c_str();
            cancel_executed.append_attribute("price") = cancel_execute_price.c_str();
            cancel_executed.append_attribute("time") = cancel_execute_time.c_str();

            // on failure, add <error> node for cancel
            pugi::xml_node cancel_error = trans_result.append_child("error");
            std::string error_cancel_trans_id = "1";    // transaction id from database
            std::string cancel_error_msg = "Failed to cancel the order!";
            cancel_error.append_child(pugi::node_pcdata).set_value(cancel_error_msg.c_str());
        }
    }
    std::stringstream trans;
    trans_response_doc.save(trans);
    std::string trans_response = trans.str();
    std::cout << trans_response << std::endl;
}

void parseXML(std::string xmlstring) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xmlstring.c_str());
    // std::cout << "result: " << result << std::endl;
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
    xmlfile.open("xmltransactions.xml");
    if (xmlfile.is_open()) {
        while (getline(xmlfile, line)) {
            xmlstring += line;
        }
        // std::cout << "xmlstring: " << xmlstring << std::endl;
        parseXML(xmlstring);
        xmlfile.close();
    }
    else {
        std::cout << "Unable to open the XML file." << std::endl;
    }
    
    

    return 0;
}
