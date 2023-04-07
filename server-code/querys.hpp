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
using namespace pqxx;
Order* find_account_order(connection*C, int order_id,double account_number);
void add_account(connection*C,double account_number,double balance);
void add_position(connection*C,double account_number,std::string symbol_name,double amount);
int add_orders(connection*C,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number,std::string execute_time,double execute_price);
Account* find_account(connection*C,double account_number); 
Position* find_position(connection*C,double account_number,std::string symbol_name);
Order*find_order(connection*C,int trans_id);
std::vector<Order*> find_executed_order(connection*C,int trans_id);
void add_executed_orders(connection*C,int order_id,std::string symbol_name,double executed_amount,double account_number,std::string execute_time,double execute_price);
std::vector<Order*> find_order_in_one_trans(connection*C,int trans_id);
Order* match_order(connection*C, std::vector<Order*> total_orders,int my_id,std::string symbol_name,double amount,double price_limit);
void execute_order(connection*C,int my_id,double account_number,std::string symbol_name,double amount,double price_limit,Order*y);
void cancel_order(connection*C, int trans_id);
std::map<std::string,std::string> query_order(connection*C,int trans_id); 
std::map<std::string,std::vector<std::string>> query_execute(connection*C,int order_id);
void check_for_match_and_execute(connection*C,int order_id);
std::vector<Order*> get_all_orders(connection*C);
void create_database(connection*C, std::string fileName);
connection* connect_database();
connection* database_init(std::string fileName);


