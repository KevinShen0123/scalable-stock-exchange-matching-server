#include<iostream>
#include<string>
class Position{
	public:
		int position_id;
		std::string symbol_name;
		double amount;
		double account_number;
		Position(int position_id,std::string symbol_name,double amount,double account_number){
			this.position_id=position_id;
			this.symbol_name=symbol_name;
			this.amount=amount;
			this.account_number=account_number;
		}
};
