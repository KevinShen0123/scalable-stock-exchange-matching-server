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
int get_order_id(connection*C,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDER WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<" AND AMOUNT="<<amount<<" AND PRICE_LIMIT="<<price_limit<<" AND EXECUTED_AMOUNT="<<executed_amount<<" AND CANCELED_AMOUNT="<<canceled_amount<<" AND LAST_UPDATE="<<W.quote(last_update)<<" AND ACCOUNT_NUMBER="<<account_number<<";";
	W.commit();
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	int order_id=0;
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
       order_id=c[0].as<int>();
     }
     return order_id;
}
int add_orders(connection*C,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number){
	work W(*C);
	std::stringstream sql;
	sql<<"INSERT INTO ORDERS(SYMBOL_NAME,AMOUNT,PRICE_LIMIT,EXECUTED_AMOUNT,CANCELED_AMOUNT,LAST_UPDATE,ACCOUNT_NUMBER) VALUES("<<W.quote(symbol_name)<<","<<amount<<","<<price_limit<<","<<executed_amount<<","<<canceled_amount<<","<<W.quote(last_update)<<","<<account_number<<");";
	W.exec(sql.str());
	W.commit();
	return get_order_id(C,symbol_name,amount,price_limit,executed_amount,canceled_amount,last_update,account_number);
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
Order*find_order(connection*C,int order_id){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE ORDER_ID="<<order_id<<";";
	W.commit();
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        return new Order(c[0].as<int>(),c[1].as<std::string>(),c[2].as<double>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<std::string>(),c[7].as<double>());
    }
    return NULL;
}
std::vector<Order*> find_order_in_one_trans(connection*C,int trans_id){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE ORDER_ID="<<trans_id<<";";
	W.commit();
	nontransaction T(*C);
	result R(T.exec(sql.str()));
	std::vector<Order*> os;
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      os.push_back(new Order(c[0].as<int>(),c[1].as<std::string>(),c[2].as<double>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<std::string>(),c[7].as<double>()));
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
   std::string cmpPrice="<=";
   if(amount<0){
   	cmpPrice=">=";
   }
   work W(*C);
   std::stringstream sql;
   sql<<"SELECT * FROM ORDERS WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<" AND amount"<<cmpString<<"0"<<" AND PRICE_LIMIT "<<cmpPrice<<price_limit<<" AND CANCELED_AMOUNT="<<0<<" AND EXECUTED_AMOUNT<AMOUNT"<<";";//open and execute differemce
   W.commit();
   nontransaction T(*C);
   result R(T.exec(sql.str()));
   std::vector<Order*> orders;
   for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      orders.push_back(new Order(c[0].as<int>(),c[1].as<std::string>(),c[2].as<double>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<std::string>(),c[7].as<double>()));
   }
    int finalIndex=0;
    if(amount>0){
    	int minValue=0;
    	int minIndex=0;
    	for(int i=0;i<orders.size();i++){
    		Order* o1=orders[i];
    		if(i==0){
    			minValue=o1->price_limit;
    			minIndex=i;
			}else{
				if(o1->price_limit<=minValue){
					minValue=o1->price_limit;
					minIndex=i;
				}
			}
		}
		finalIndex=minIndex;
	}else{
		int maxValue=0;
    	int maxIndex=0;
    	for(int i=0;i<orders.size();i++){
    		Order* o1=orders[i];
    		if(i==0){
    			maxValue=o1->price_limit;
    			maxIndex=i;
			}else{
				if(o1->price_limit>=maxValue){
					maxValue=o1->price_limit;
					maxIndex=i;
				}
			}
		}
		finalIndex=maxIndex;
	}
	if(orders.size()==0){
		return NULL;
	}
	return orders[finalIndex];
}
void execute_order(connection*C,double account_number,std::string symbol_name,double amount,double price_limit,Order*y){
	if(amount<0){
		//account balance update   position belong to this account shares update    order execute amount update, 
		//first step determine execute amount
		double minA=(-1)*amount;
		if(y->amount<=minA){
			minA=y->amount;
		}
		double execute_price=y->price_limit;
		
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
		Order* x=find_order(C,y->order_id);
		x->executed_amount=minA;
		y->executed_amount=minA;
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<x->executed_amount<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<y->executed_amount<<" WHERE ORDER_ID="<<y->order_id<<";";
		time_t now=std::time(0);
		sql<<"UPDATE TABLE ORDERS SET LAST_UPDATE="<<std::string(std::asctime(std::localtime(&(now))))<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<"UPDATE TABLE ORDERS SET LAST_UPDATE="<<std::string(std::asctime(std::localtime(&(now))))<<" WHERE ORDER_ID="<<y->order_id<<";";
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
		Order*x=find_order(C,y->order_id);
		x->executed_amount=minA;
		y->executed_amount=minA;
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<x->executed_amount<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<"UPDATE TABLE ORDERS SET EXECUTED_AMOUNT="<<y->executed_amount<<" WHERE ORDER_ID="<<y->order_id<<";";
		time_t now=std::time(0);
		sql<<"UPDATE TABLE ORDERS SET LAST_UPDATE="<<std::string(std::asctime(std::localtime(&(now))))<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<"UPDATE TABLE ORDERS SET LAST_UPDATE="<<std::string(std::asctime(std::localtime(&(now))))<<" WHERE ORDER_ID="<<y->order_id<<";";
		W.exec(sql.str());
		W.commit();
	}
}
void cancel_order(connection*C, int trans_id){
	Order*this_order=find_order(C,trans_id);
	if(this_order==NULL){
		return;
	}
    this_order->canceled_amount=this_order->amount-this_order->executed_amount;
	work W(*C);
	std::stringstream sql;
	Account*my_account=find_account(C,this_order->account_number);
	if(this_order->order_type==1){
	    	my_account->balance+=(this_order->price_limit)*(this_order->canceled_amount);
	}
	sql<<"UPDATE TABLE ACCOUNT SET BALANCE="<<my_account->balance<<" WHERE ACCOUNT_NUMBER="<<my_account->account_number<<";";
	sql<<"UPDATE TABLE ORDERS SET CANCELED_AMOUNT="<<this_order->canceled_amount<<" WHERE ORDER_ID="<<this_order->order_id<<";";
	time_t now=std::time(0);
	sql<<"UPDATE TABLE ORDERS SET LAST_UPDATE="<<std::string(std::asctime(std::localtime(&(now))))<<" WHERE ORDER_ID="<<this_order->order_id<<";";
	W.exec(sql.str());
	W.commit();
}
std::map<std::string,std::string> query_order(connection*C,int trans_id){
	Order*order=find_order(C,trans_id);
	std::map<std::string,std::string> order_map;
	if(order==NULL){
		return order_map;
	}
	Order*matched_order=match_order(C,order->symbol_name,order->amount,order->price_limit);
	if(matched_order!=NULL){
		if(order->executed_amount!=0){
			order->execute_price=matched_order->execute_price;
		}
	}
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("open","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("executed","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("canceled","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("executed-time","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("canceled-time","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("execute-price","0"));
	double curOpen=std::stod(order_map.find("open")->second);
	curOpen+=order->amount-order->executed_amount;
	curOpen-=order->canceled_amount;
	std::stringstream ss1;
	ss1<<curOpen;
	order_map.find("open")->second=ss1.str();
	double curExe=std::stod(order_map.find("executed")->second);
	curExe+=order->executed_amount;
	std::stringstream ss2;
	ss2<<curExe;
	order_map.find("executed")->second=ss2.str();
	double curCan=std::stod(order_map.find("canceled")->second);
	curCan+=order->canceled_amount;
	std::stringstream ss3;
	ss3<<curCan;
	order_map.find("canceled")->second=ss3.str();
     if(order->canceled_amount!=0){
		order_map.find("canceled-time")->second=order->last_update;
	}else{
		order_map.find("executed-time")->second=order->last_update;
	}
	order_map.find("execute-price")->second=order->execute_price;
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
connection* database_init(std::string fileName){
	connection*C=connect_database();
	create_database(C,fileName);
	return C;
}


