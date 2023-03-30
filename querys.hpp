#include <iostream>
#include <pqxx/pqxx>
#include<sstream>
#include<cstring>
#include<fstream>
#include <iomanip>
#include <string>
using namespace pqxx;
void add_account(connection*C,double account_number,double balance);
void add_position(connection*C,double account_number,std::string symbol_name,double amount);
void add_orders(connection*C,int transaction_id,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number);
void create_database(connection*C, std::string fileName);
connection* connect_database();
void database_init(std::string fileName);
