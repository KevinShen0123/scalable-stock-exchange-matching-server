#include <iostream>
#include <pqxx/pqxx>
#include<sstream>
#include<cstring>
#include<fstream>
#include<time.h>
#include "querys.hpp"
#include<vector>
void add_position(connection*C, std::string symbol_name, double amount){
	work W(*C);
	std::string sql;
	sql="INSERT INTO POSITION(SYMBOL_NAME,AMOUNT) VALUES(";
	sql+=W.quote(symbol_name);
	sql+=",";
	sql+=amount;
	sql+=");";
	W.exec(sql);
	W.commit();
}
void add_account(connection*C,std::string account_number, std::string balance, int position_count){
	work W(*C);
	std::string sql;
	sql="INSERT INTO ACCOUNT(ACCOUNT_NUMBER,BALANCE,POSITION_COUNT) VALUES(";
	sql+W.quote(account_number);
	sql+=",";
	sql+=W.quote(balance);
	sql+=",";
	sql+=position_count;
	sql+=");";
	W.exec(sql);
	W.commit();
}
void add_account_position_relation(connection*C,std::string account_number,int position_ID){
	work W(*C);
	std::string sql;
	sql="INSERT INTO ACCOUNT_POSITION(ACCOUNT_NUMBER,POSITION_ID) VALUES("+W.quote(account_number)+","+position_ID+")"+";";
	W.exec(sql);
	W.commit();
}
void add_orders(connection*C,std::string symbol_name,double amount,double price_limit){
	std::string current_time=ctime(&(std::time(0)));
	work W(*C);
	std::string sql;
	sql="INSERT INTO ORDERS(SYMBOL_NAME,AMOUNT,PRICE_LIMIT,EXECUTED_AMOUNT,CANCELD_AMOUNT,LAST_UPDATE) VALUES("+W.quote(symbol_name)+","+amount+","+price_limit+","+0+","+0+","+current_time+");";
	W.exec(sql);
	W.commit();
}
void add_transaction(connection*C,std::string account_id,int transaction_status){
	work W(*C);
	std::string sql;
	sql="INSERT INTO TRANSACTION(ACCOUNT_ID,TRANSACTION_STATUS) VALUES("+W.quote(account_id)+","transaction_status+");";
	W.exec(sql);
	W.commit();
}
void add_transaction_order_relation(connection*C,int transaction_id, int order_id){
	work W(*C);
	std::string sql;
	sql="INSERT INTO TRANSACTION_ORDER(TRANSACTION_ID,ORDER_ID) VALUES("+transaction_id+","+order_id");";
	W.exec(sql);
	W.commit();
}
int get_position_id(connection*C,std::string symbol_name,double amount){
	work W(*C);
	std::string sql;
	sql="SELECT POSITION_ID FROM POSITION WHERE SYMBOL_NAME="+W.quote(symbol_name)+" AND"+" AMOUNT="+amount+";";
	W.commit();
	nontransaction R(*C);
	result S(R.exec(sql));
	int position_id=0; 
	for (result::const_iterator c = S.begin(); c != S.end(); ++c) {
        position_id=c[0].as<int>();
        break;
     }
     return position_id;
}
int get_order_id(connection*C,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update){
	work W(*C);
	std::string sql;
	sql="SELECT ORDER_ID FROM ORDERS WHERE SYMBOL_NAME="+W.quote(symbol_name)+" AND AMOUNT="+amount+" AND PRICE_LIMIT="+price_limit+" AND EXECUTED_AMOUNT="+executed_amount+" AND CANCELED_AMOUNT="+canceled_amount+" AND LAST_UPDATE="+W.quote(last_update)+";";
	W.commit();
	nontransaction R(*C);
	result S(R.exec(sql));
	int order_id=0;
	 for (result::const_iterator c = S.begin(); c != S.end(); ++c) {
        order_id=c[0].as<int>();
        break;
     }
     return order_id;
}
int get_transaction_id(connection*C,int transaction_status,std::string account_id){
	work W(*C);
	std::string sql;
	sql="SELECT TRANSACTION_ID FROM TRANSACTION WHERE ACCOUNT_ID="+W.quote(account_id)+" AND TRANSACTION_STATUS="+transaction_status+";";
	W.commit();
	nontransaction R(*C);
	result S(R.exec(sql));
	int transaction_id=0;
	 for (result::const_iterator c = S.begin(); c != S.end(); ++c) {
        transaction_id=c[0].as<int>();
        break;
     }
     return transaction_id;
}
std::vector<int> match_order(connection*C,std::string symbol_name,double amount,double price_limit){
	 std::string query_str="<=";
	if(amount>0){
		query_str="<=";
	}else{
		query_str=">=";
	}
	work W(*C);
	std::string sql="SELECT ORDER_ID FROM ORDERS WHERE SYMBOL_NAME="+W.quote(symbol_name)+" AND AMOUNT="+(-1)*amount+" AND PRICE_LIMIT"+W.quote(query_str)+price_limit+";";
	W.commit();
	nontransaction R(*C);
	result S(R.exec(sql));
	std::vector<int> orders;
	 for (result::const_iterator c = S.begin(); c != S.end(); ++c) {
         orders.push_back(c[0].as<int>());
     }
     return orders;
}


void execute_order(int this_account_id,int target_account_id,int this_order_id,int that_order_id){
	work W(*C);
	std::string sql="SELECT AMOUNT FROM ORDERS WHERE ORDER_ID="+this_order_id;
	W.commit();
	nontransaction R(*C);
	result S(R.exec(sql));
	bool is_sell=true;
	for (result::const_iterator c = S.begin(); c != S.end(); ++c) {
         int amount_value=c[0].as<int>();
         if(amount_value>0){
         	is_sell=false;
		 }
		 break;
     }
     //execute order as required,change order executed_amount
}
void cancel_order(int account_id,int order_id){
	//change order canceled amount
}
void transaction_query(int transaction_id){
	//search each order in the transaction by examine amount, transaction amount, executed amount and output info as required.
}

void create_database(connection*C,std::string fileName){
	std::ifstream o1(fileName.c_str());
	if(o1.is_open()){
		std::string line;
		std::string sql;
		while(std::getline(o1,line)){
			sql+=line;
		}
		work W(*C);
		W.exec(sql);
		W.commit();
	}else{
		std::cerr<<"file Not Exist!!!!!!"<<std::endl;
		return;
	}
}
connection* connect_database(){
  connection *C;
  try{
    C = new connection("dbname=match user=postgres password=passw0rd hostaddr=127.0.0.1");
    if (C->is_open()) {
      std::cout << "Opened database successfully: " << C->dbname() <<std::endl;
    } else {
      std::cout << "Can't open database" << std::endl;
      return NULL;
    }
    return C;
  } catch (const std::exception &e){
    std::cerr << e.what() << std::endl;
    return NULL;
  }
  return NULL;
}
void database_init(std::string fileName){
	connection*C=connect_database();
	create_database(C,fileName);
}
int main(){
	database_init("start.sql");
	return 0;
}



