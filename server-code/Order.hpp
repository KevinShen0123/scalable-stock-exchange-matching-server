#include<iostream>
#include<string>
#include<cstring>
class Order{
	public:
		int order_id;
		double execute_price;
		std::string symbol_name;
		double amount;
		double price_limit;
		double executed_amount;
		double canceled_amount;
		std::string last_update;
		double account_number;
		int order_type;//0 for sell order, 1 for buy order 
		std::string execute_time;
		Order(int order_id,std::string symbol_name,double amount,double price_limit,double executed_amount,double canceled_amount,std::string last_update,double account_number,std::string execute_time, double execute_price){
			this->order_id=order_id;
			this->execute_price=execute_price;
			this->symbol_name=symbol_name;
			this->amount=amount;
			this->price_limit=price_limit;
			this->executed_amount=executed_amount;
			this->canceled_amount=canceled_amount;
			this->last_update=last_update;
			this->account_number=account_number;
			this->order_type=0;
			if(amount>=0){
				this->order_type=1;
			}
			this->execute_time=execute_time;
		}
};
