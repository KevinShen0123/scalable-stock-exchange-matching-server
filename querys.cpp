#include <iostream>
#include <pqxx/pqxx>
#include<sstream>
#include<cstring>
#include<fstream>
#include<time.h>
#include "querys.hpp"
#include<vector>
#include<iomanip>
#include<string>
#include<sstream>
#include<iostream>
#include <iomanip>
void add_account(connection*C,double account_number, double balance){
  work W(*C);
  std::stringstream sql;
  sql<<"INSERT INTO ACCOUNT(ACCOUNT_NUMBER,BALANCE) VALUES("<<account_number<<","<<balance<<");";
  W.exec(sql.str());
  W.commit();
}
void add_position(connection*C,double account_number,std::string symbol_name,double amount){
	work W(*C);
	std::stringstream sql; 
	sql<<"INSERT INTO POSITION(SYMBOL_NAME,AMOUNT,ACCOUNT_NUMBER) VALUES("<<symbol_name<<","<<amount<<","<<account_number<<");";
	W.exec(sql.str());
	W.commit();
}
void add_orders(connection*C,int transaction_id,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number){
	work W(*C);
	std::stringstream sql;
	sql<<"INSERT INTO ORDERS(TRANSACTION_ID,SYMBOL_NAME,AMOUNT,PRICE_LIMIT,EXECUTED_AMOUNT,CANCELED_AMOUNT,LAST_UPDATE,ACCOUNT_NUMBER) VALUES("<<transaction_id<<","<<W.quote(symbol_name)<<","<<amount<<","<<price_limit<<","<<executed_amount<<","<<canceled_amount<<","<<W.quote(last_update)<<","<<account_number<<");";
	W.exec(sql.str());
	W.commit();
}
Account* find_account(connection*C,double account_number){
	work W(*C);
	Account*A;
	std::stringstream sql;
	sql<<"SELECT * FROM ACCOUNT WHERE ACCOUNT_NUMBER="<<account_number<<";";
	W.commit();
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	int count=0;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    A->account_number=c[0].as<double>();
    A->balance=c[1].as<double>();
    count+=1;
    break;
  }
  if(count==0){
  	return NULL;
  }
  return A;
}
Position* find_position(connection*C,double account_number,std::string symbol_name){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM POSITION WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<"ACCOUNT_NUMBER="<<account_number<<";";
	W.commit();
	Position* P;
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	int count=0;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    P->position_id=c[0].as<int>();
    P->symbol_name=c[1].as<std::string>();
    P->amount=c[2].as<double>();
    P->account_number=c[3].as<double>();
    count+=1;
    break;
  }
  if(count==0){
  	return NULL;
  }
  return P;
}
Order*find_order(connection*C,int trans_id,double account_number,std::string symbol_name,double amount,double price_limit){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE  TRANSACTION_ID="<<trans_id<<" AND ACCOUNT_NUMBER="<<account_number<<" AND SYMBOL_NAME="<<W.quote(symbol_name)<<" AND AMOUNT="<<amount<<" AND PRICE_LIMIT="<<price_limit<<";";
	W.commit();
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        return new Order(c[0].as<int>(),c[1].as<int>(),c[2].as<std::string>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<double>(),c[7].as<std::string>(),c[8].as<double>());
    }
    return NULL;
}
std::vector<Order*> find_order_in_one_trans(connection*C,int trans_id){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE TRANSACTION_ID="<<trans_id<<";";
	W.commit();
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	std::vector<Order*> os;
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      os.push_back(new Order(c[0].as<int>(),c[1].as<int>(),c[2].as<std::string>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<double>(),c[7].as<std::string>(),c[8].as<double>()));
    }
    return os;
}
//match order, execute order, query order, cancel order
Order* match_order(connection*C, std::string symbol_name,double amount,double price_limit){
   //find match,same symbol name, amount1*amount2<0,executed_amount=0,price,
   std::string cmpString=">=";
   if(amount>=0){
   	cmpString="<";
   }
   std::string cmpPrice=">";
   if(amount<0){
   	cmpPrice="<";
   }
   work W(*C);
   std::stringstream sql;
   sql<<"SELECT * FROM ORDERS WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<" AND amount"<<cmpString<<"0"<<" AND PRICE_LIMIT "<<cmpPrice<<price_limit<<" AND EXECUTED_AMOUNT="<<0<<";";
   W.commit();
   nontransaction T(*C);
   result R(T.exec(sql.str()));
   std::vector<Order*> orders;
   for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      orders.push_back(new Order(c[0].as<int>(),c[1].as<int>(),c[2].as<std::string>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<double>(),c[7].as<std::string>(),c[8].as<double>()));
   }
   	int max_index=0;
   	double max_value=0;
   	for(int i=0;i<orders.size();i++){
   	 	Order* ord=orders[i];
   	 	if(ord->price_limit>=max_value){
   	 		max_value=ord->price_limit;
   	 		max_index=i;
		}
	}
	return orders[max_index];
}
void execute_order(connection*C,double account_number,std::string symbol_name,double amount,double price_limit,Order*y){
	if(amount<0){
		//account balance update   position belong to this account shares update    order execute amount update, 
		//first step determine execute amount
		double minA=amount;
		if(y->amount<=minA){
			minA=y->amount;
		}
		double execute_price=price_limit;
		Account* sellerAccount=find_account(C,account_number);
		Account* buyerAccount=find_account(C,y->account_number);
		sellerAccount->balance=sellerAccount->balance+(minA*execute_price);
		buyerAccount->balance=buyerAccount->balance-(minA*execute_price);
		Position*sellerPosition=find_position(C,account_number,symbol_name);
		Position*buyerPosition=find_position(C,y->account_number,symbol_name);
		bool need_position=false;
		if(buyerPosition==NULL){
			add_position(C,y->account_number,symbol_name,minA);
		}else{
			need_position=true;
			buyerPosition->amount+=minA;
			sellerPosition->amount-=minA;
		}
		work W(*C);
		std::stringstream sql;
		sql<<"UPDATE TABLE ACCOUNT SET BALANCE="<<sellerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
		sql<<"UPDATE TABLE ACCOUNT SET BALANCE="<<buyerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
		if(need_position){
			sql<<"UPDATE TABLE POSITION SET AMOUNT="<<sellerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
			sql<<"UPDATE TABLE POSITION SET AMOUNT="<<buyerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
		}
		Order* x=find_order(C,y->trans_id,account_number,symbol_name,amount,price_limit);
		x->executed_amount=minA;
		y->executed_amount=minA;
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<x->executed_amount<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<y->executed_amount<<" WHERE ORDER_ID="<<y->order_id<<";";
		W.exec(sql.str());
		W.commit();
	}else{
	   	double minA=amount;
		if(y->amount<=minA){
			minA=y->amount;
		}
		double execute_price=y->price_limit;
		Account* sellerAccount=find_account(C,y->account_number);
		Account* buyerAccount=find_account(C,account_number);
		sellerAccount->balance=sellerAccount->balance+(minA*execute_price);
		buyerAccount->balance=buyerAccount->balance-(minA*execute_price);
		Position*sellerPosition=find_position(C,y->account_number,symbol_name);
		Position*buyerPosition=find_position(C,account_number,symbol_name);
		bool need_position=false;
		if(buyerPosition==NULL){
			add_position(C,account_number,symbol_name,minA);
		}else{
			need_position=true;
			buyerPosition->amount+=minA;
			sellerPosition->amount-=minA;
		}
		work W(*C);
		std::stringstream sql;
		sql<<"UPDATE TABLE ACCOUNT SET BALANCE="<<sellerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
		sql<<"UPDATE TABLE ACCOUNT SET BALANCE="<<buyerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
		if(need_position){
			sql<<"UPDATE TABLE POSITION SET AMOUNT="<<sellerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
			sql<<"UPDATE TABLE POSITION SET AMOUNT="<<buyerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
		}
		Order*x=find_order(C,y->trans_id,account_number,symbol_name,amount,price_limit);
		x->executed_amount=minA;
		y->executed_amount=minA;
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<x->executed_amount<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<y->executed_amount<<" WHERE ORDER_ID="<<y->order_id<<";";
		W.exec(sql.str());
		W.commit();
	}
}
void cancel_order(connection*C, int trans_id){
	std::vector<Order*> orders=find_order_in_one_trans(C,trans_id);
	if(orders.size()==0){
		return;
	}
	for(Order* x:orders){
		x->canceled_amount=x->amount-x->executed_amount;
	}
	work W(*C);
	std::stringstream sql;
	for(int i=0;i<orders.size();i++){
		Account*my_account=find_account(C,orders[i]->account_number);
	    if(orders[i]->order_type==1){
	    	my_account->balance+=(orders[i]->price_limit)*(orders[i]->canceled_amount);
		}
		sql<<"UPDATE TABLE ACCOUNT SET BALANCE="<<my_account->balance<<" WHERE ACCOUNT_NUMBER="<<my_account->account_number<<";";
		sql<<"UPDATE TABLE ORDERS SET CANCELED_AMOUNT="<<orders[i]->canceled_amount<<" WHERE ORDER_ID="<<orders[i]->order_id<<";";
	}
	W.exec(sql.str());
	W.commit();
}
std::map<std::string,double> query_order(connection*C,int trans_id){
	std::vector<Order*> orders=find_order_in_one_trans(C,trans_id);
	std::map<std::string,double> order_map;
	if(orders.size()==0){
		return order_map;
	}
	order_map.insert(order_map.begin(),std::pair<std::string,int>("opened",0));
	order_map.insert(order_map.begin(),std::pair<std::string,int>("executed",0));
	order_map.insert(order_map.begin(),std::pair<std::string,int>("canceled",0));
	for(int i=0;i<orders.size();i++){
		Order* o=orders[i];
		order_map.find("opened")->second+=o->amount;
		order_map.find("executed")->second+=o->executed_amount;
		order_map.find("canceled")->second+=o->canceled_amount;
	}
	return order_map;
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



