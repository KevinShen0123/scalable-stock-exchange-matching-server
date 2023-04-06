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
#include<ctime>
void add_account(connection*C,double account_number, double balance){
  work W(*C);
  std::stringstream sql;
  sql<<"INSERT INTO ACCOUNT(ACCOUNT_NUMBER,BALANCE) VALUES("<<account_number<<","<<balance<<");";
  W.exec(sql.str());
  W.commit();
}
void add_position(connection*C,double account_number,std::string symbol_name,double amount){
	Position*p=find_position(C,account_number,symbol_name);
	if(p==NULL){
      work W(*C);
	  std::stringstream sql; 
	  sql<<"INSERT INTO POSITION(SYMBOL_NAME,AMOUNT,ACCOUNT_NUMBER) VALUES("<<W.quote(symbol_name)<<","<<amount<<","<<account_number<<");";
	  W.exec(sql.str());
	  W.commit();
	}else{
		  work W(*C);
	      std::stringstream sql; 
		  double total_amount=p->amount+amount;
	      sql<<"UPDATE POSITION SET AMOUNT="<<total_amount<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
	     W.exec(sql.str());
	     W.commit();
	}
}
int get_order_id(connection*C,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number,std::string execute_time,double execute_price){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<" AND AMOUNT="<<amount<<" AND PRICE_LIMIT="<<price_limit<<" AND EXECUTED_AMOUNT="<<executed_amount<<" AND CANCELED_AMOUNT="<<canceled_amount<<" AND LAST_UPDATE="<<W.quote(last_update)<<" AND ACCOUNT_NUMBER="<<account_number<<" AND EXECUTE_TIME="<<W.quote(execute_time)<<" AND EXECUTE_PRICE="<<execute_price<<" FOR UPDATE;";
	result R(W.exec(sql.str()));
	int order_id=0;
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
       order_id=c[0].as<int>();
     }
     W.commit();
     return order_id;
}
int add_orders(connection*C,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number,std::string execute_time,double execute_price){
	work W(*C);
	std::stringstream sql;
	sql<<"INSERT INTO ORDERS(SYMBOL_NAME,AMOUNT,PRICE_LIMIT,EXECUTED_AMOUNT,CANCELED_AMOUNT,LAST_UPDATE,ACCOUNT_NUMBER,EXECUTE_TIME,EXECUTE_PRICE) VALUES("<<W.quote(symbol_name)<<","<<amount<<","<<price_limit<<","<<executed_amount<<","<<canceled_amount<<","<<W.quote(last_update)<<","<<account_number<<","<<W.quote(execute_time)<<","<<execute_price<<")"<<";";
	W.exec(sql.str());
	W.commit();
	return get_order_id(C,symbol_name,amount,price_limit,executed_amount,canceled_amount,last_update,account_number,execute_time,execute_price);
}
Account* find_account(connection*C,double account_number){
	work W(*C);
	Account*A=new Account(0,0);
	std::stringstream sql;
	sql<<"SELECT * FROM ACCOUNT WHERE ACCOUNT_NUMBER="<<account_number<<" FOR UPDATE;";
    result R(W.exec(sql.str()));
	int count=0;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    A->account_number=c[0].as<double>();
    A->balance=c[1].as<double>();
    count+=1;
    break;
  }
  W.commit();
  if(count==0){
  	return NULL;
  }
  return A;
}
Position* find_position(connection*C,double account_number,std::string symbol_name){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM POSITION WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<" AND ACCOUNT_NUMBER="<<account_number<<" FOR UPDATE;";
    Position* P=new Position(0,"",0,0);
	result R(W.exec(sql.str()));
	int count=0;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    P->position_id=c[0].as<int>();
    P->symbol_name=c[1].as<std::string>();
    P->amount=c[2].as<double>();
    P->account_number=c[3].as<double>();
    count+=1;
    break;
  }
  W.commit();
  if(count==0){
  	return NULL;
  }
  return P;
}
Order*find_order(connection*C,int order_id){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE ORDER_ID="<<order_id<<" FOR UPDATE;";
	result R(W.exec(sql.str()));
	W.commit();
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        return new Order(c[0].as<int>(),c[1].as<std::string>(),c[2].as<double>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<std::string>(),c[7].as<double>(),c[8].as<std::string>(),c[9].as<double>());
    }
    return NULL;
}
std::vector<Order*> find_order_in_one_trans(connection*C,int trans_id){
	work W(*C);
	std::stringstream sql;
	sql<<"SELECT * FROM ORDERS WHERE ORDER_ID="<<trans_id<<" FOR UPDATE;";
	result R(W.exec(sql.str()));
	std::vector<Order*> os;
	for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      os.push_back(new Order(c[0].as<int>(),c[1].as<std::string>(),c[2].as<double>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<std::string>(),c[7].as<double>(),c[8].as<std::string>(),c[9].as<double>()));
    }
    W.commit();
    return os;
}
//match order, execute order, query order, cancel order
Order* match_order(connection*C, std::string symbol_name,double amount,double price_limit){
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
   sql<<"SELECT * FROM ORDERS WHERE SYMBOL_NAME="<<W.quote(symbol_name)<<" AND amount"<<cmpString<<0<<" AND PRICE_LIMIT "<<cmpPrice<<price_limit<<" AND CANCELED_AMOUNT="<<0<<" AND EXECUTED_AMOUNT<ABS(AMOUNT)  "<<" FOR UPDATE;";//open and execute differemce
   result R(W.exec(sql.str()));
   W.commit();
   std::vector<Order*> orders;
   for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
	  //std::cout<<"find matched order!!!!!!!!!!!!!!!!"<<std::endl;
      orders.push_back(new Order(c[0].as<int>(),c[1].as<std::string>(),c[2].as<double>(),c[3].as<double>(),c[4].as<double>(),c[5].as<double>(),c[6].as<std::string>(),c[7].as<double>(),c[8].as<std::string>(),c[9].as<double>()));
   }
    int finalIndex=0;
    if(amount>0){
    	double minValue=0;
    	int minIndex=0;
    	double minA=amount;
    	for(int i=0;i<orders.size();i++){
    		Order* o1=orders[i];
    		if((o1->amount)*(-1)<minA){
    			minA=(o1->amount)*(-1);
			}
    		if(i==0){
    			minValue=(o1->price_limit)*minA;
    			minIndex=i;
			}else{
				if((o1->price_limit)*minA<minValue){
					minValue=(o1->price_limit)*minA;
					minIndex=i;
				}
			}
		}
		finalIndex=minIndex;
	}else{
		double maxValue=0;
    	int maxIndex=0;
    	double minA=amount*(-1);
    	for(int i=0;i<orders.size();i++){
    		Order* o1=orders[i];
    		if(o1->amount<minA){
    			minA=amount;
			}
    		if(i==0){
    			maxValue=(o1->price_limit)*minA;
    			maxIndex=i;
			}else{
				if((o1->price_limit)*minA>maxValue){
					maxValue=(o1->price_limit)*minA;
					maxIndex=i;
				}
			}
		}
		finalIndex=maxIndex;
	}
	//std::cout<<"Match finished !!!!!!"<<std::endl;
	if(orders.size()==0){
		return NULL;
	}
	return orders[finalIndex];
}
void execute_order(connection*C,int my_id,double account_number,std::string symbol_name,double amount,double price_limit,Order*y){//paritially execute 
	//std::cout<<"EXECUTE is called!!!!!!!"<<std::endl;
	if(amount<0){
		//account balance update   position belong to this account shares update    order execute amount update, 
		//first step determine execute amount
		Order* x=find_order(C,my_id);
		double minA=(-1)*amount;
		double yAmount=y->amount-y->executed_amount;
		yAmount-=y->canceled_amount; 
		if(yAmount<=0){
			yAmount=0;
		}
		if(yAmount<=minA){
			minA=yAmount;
		}
		double execute_price=y->price_limit;
		Account* sellerAccount=find_account(C,account_number);
		Account* buyerAccount=find_account(C,y->account_number);
		sellerAccount->balance=sellerAccount->balance+(minA*execute_price);
		if(sellerAccount->balance<0){
			sellerAccount->balance=0;
		}
		double org_buy_balance=buyerAccount->balance;
		buyerAccount->balance=buyerAccount->balance-(minA*execute_price);
		if(buyerAccount->balance<0){
			return;
		}
		Position*sellerPosition=find_position(C,account_number,symbol_name);
		Position*buyerPosition=find_position(C,y->account_number,symbol_name);
		//std::cout<<"EXECUTE_FINISHED0A"<<std::endl;
		bool need_position=false;
		if(buyerPosition==NULL){
			add_position(C,y->account_number,symbol_name,minA);
		}else{
			need_position=true;
			buyerPosition->amount+=minA;
			sellerPosition->amount-=minA;
		}
		//std::cout<<"EXECUTE_FINISHED1A"<<std::endl;
		work W(*C);
		std::stringstream sql;
		sql<<"UPDATE ACCOUNT SET BALANCE="<<sellerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
		sql<<" UPDATE ACCOUNT SET BALANCE="<<buyerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
		if(need_position){
			sql<<" UPDATE POSITION SET AMOUNT="<<sellerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
			sql<<" UPDATE POSITION SET AMOUNT="<<buyerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
		}
		x->executed_amount=minA;
		y->executed_amount=minA;
		sql<<" UPDATE ORDERS SET EXECUTED_AMOUNT="<<x->executed_amount<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<" UPDATE ORDERS SET EXECUTED_AMOUNT="<<y->executed_amount<<" WHERE ORDER_ID="<<y->order_id<<";";
		time_t now=std::time(0);
        std::stringstream timeStream;
		timeStream<<now;
		std::string timeStr=timeStream.str();
		sql<<" UPDATE ORDERS SET EXECUTE_TIME="<<W.quote(timeStr)<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<" UPDATE ORDERS SET EXECUTE_TIME="<<W.quote(timeStr)<<" WHERE ORDER_ID="<<y->order_id<<";";
	    sql<<" UPDATE ORDERS SET EXECUTE_PRICE="<<execute_price<<" WHERE ORDER_ID="<<x->order_id<<";";
	    sql<<" UPDATE ORDERS SET EXECUTE_PRICE="<<execute_price<<" WHERE ORDER_ID="<<y->order_id<<";";
		W.exec(sql.str());
		//std::cout<<"EXECUTE_FINISHED2A"<<std::endl;
		W.commit();
		//std::cout<<"EXECUTE_FINISHEDA"<<std::endl;
	}else{
		Order*x=find_order(C,my_id);
	   	double minA=amount;
	   	double yAmount=(-1)*(y->amount);
	   	yAmount-=y->executed_amount;
	   	yAmount-=y->canceled_amount;
		if((yAmount)*(-1)<=minA){
			minA=(yAmount)*(-1);
		}
		double execute_price=y->price_limit;
		Account* sellerAccount=find_account(C,y->account_number);
		Account* buyerAccount=find_account(C,account_number);
		sellerAccount->balance=sellerAccount->balance+(minA*execute_price);
		buyerAccount->balance=buyerAccount->balance-(minA*execute_price);
		if(buyerAccount->balance<0){
			return;
		}
		Position*sellerPosition=find_position(C,y->account_number,symbol_name);
		Position*buyerPosition=find_position(C,account_number,symbol_name);
		//std::cout<<"EXECUTE_FINISHED0B"<<std::endl;
		bool need_position=false;
		if(buyerPosition==NULL){
			add_position(C,account_number,symbol_name,minA);
		}else{
			need_position=true;
			buyerPosition->amount+=minA;
			sellerPosition->amount-=minA;
		}
		//std::cout<<"EXECUTE_FINISHED1B"<<std::endl;
		work W(*C);
		//std::cout<<"EXECUTE_FINISHED5B"<<std::endl;
		std::stringstream sql;
		sql<<"UPDATE ACCOUNT SET BALANCE="<<sellerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
		sql<<" UPDATE ACCOUNT SET BALANCE="<<buyerAccount->balance<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
		if(need_position){
			sql<<" UPDATE POSITION SET AMOUNT="<<sellerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<y->account_number<<";";
			sql<<" UPDATE POSITION SET AMOUNT="<<buyerPosition->amount<<" WHERE ACCOUNT_NUMBER="<<account_number<<";";
		}
		std::cout<<"EXECUTE_FINISHED4B"<<(x==NULL)<<std::endl;
		x->executed_amount=minA;
		y->executed_amount=minA;
		sql<<" UPDATE ORDERS SET EXECUTED_AMOUNT="<<x->executed_amount<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<" UPDATE ORDERS SET EXECUTED_AMOUNT="<<y->executed_amount<<" WHERE ORDER_ID="<<y->order_id<<";";
		time_t now=std::time(0);
		std::stringstream timeStream;
		timeStream<<now;
		std::string timeStr=timeStream.str();
		sql<<" UPDATE ORDERS SET EXECUTE_TIME="<<W.quote(timeStr)<<" WHERE ORDER_ID="<<x->order_id<<";";
		sql<<" UPDATE ORDERS SET EXECUTE_TIME="<<W.quote(timeStr)<<" WHERE ORDER_ID="<<y->order_id<<";";
		sql<<" UPDATE ORDERS SET EXECUTE_PRICE="<<execute_price<<" WHERE ORDER_ID="<<x->order_id<<";";
	    sql<<" UPDATE ORDERS SET EXECUTE_PRICE="<<execute_price<<" WHERE ORDER_ID="<<y->order_id<<";";
		std::cout<<"EXECUTE_FINISHED3B"<<std::endl;
		W.exec(sql.str());
		//std::cout<<"EXECUTE_FINISHED2B"<<std::endl;
		W.commit();
		//std::cout<<"EXECUTE_FINISHEDB"<<std::endl;
	}
}
void cancel_order(connection*C, int trans_id){
	Order*this_order=find_order(C,trans_id);
	//std::cout<<"Cancel is Called!!!!!!!"<<std::endl;
	if(this_order==NULL){
		return;
	}
    this_order->canceled_amount=this_order->amount-this_order->executed_amount;
	Account*my_account=find_account(C,this_order->account_number);
	if(my_account==NULL){
		return;
	}
	work W(*C);
	//std::cout<<"Cancel started!!!!!!!!!"<<std::endl;
	std::stringstream sql;
	if(this_order->order_type==1){
	    my_account->balance+=(this_order->price_limit)*(this_order->canceled_amount);
	}
	sql<<"UPDATE ACCOUNT SET BALANCE="<<my_account->balance<<" WHERE ACCOUNT_NUMBER="<<my_account->account_number<<";";
	sql<<"UPDATE ORDERS SET CANCELED_AMOUNT="<<this_order->canceled_amount<<" WHERE ORDER_ID="<<this_order->order_id<<";";
	time_t now=std::time(0);
	std::stringstream  timeStream;
	timeStream<<now;
	std::string timeStr=timeStream.str();
	sql<<"UPDATE ORDERS SET LAST_UPDATE="<<W.quote(timeStr)<<" WHERE ORDER_ID="<<this_order->order_id<<";";
	W.exec(sql.str());
	//std::cout<<"Work exec!!!!!!"<<std::endl;
	W.commit();
	//std::cout<<"Cancel success!!!!!!!!!!!!!!!!!!!"<<std::endl;
}
std::map<std::string,std::string> query_order(connection*C,int trans_id){
	Order*order=find_order(C,trans_id);
	std::map<std::string,std::string> order_map;
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("open","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("executed","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("canceled","0"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("executed-time","Not been executed"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("canceled-time","Not been canceled"));
	order_map.insert(order_map.begin(),std::pair<std::string,std::string>("execute-price","Not been executed"));
	if(order==NULL){
		return order_map;
	}
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
    if(order->executed_amount==0&&order->canceled_amount==0){
    	return order_map;
	}else if(order->executed_amount==0&&order->canceled_amount!=0){
		order_map.find("canceled-time")->second=order->last_update;
	}else{
		order_map.find("executed-time")->second=order->execute_time;
		order_map.find("canceled-time")->second=order->last_update;
		order_map.find("execute-price")->second=order->execute_price;
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
connection* database_init(std::string fileName){
	connection*C=connect_database();
	//std::cout<<"Hello C"<<std::endl;
	create_database(C,fileName);
	//std::cout<<"Hello D"<<std::endl;
	return C;
}


