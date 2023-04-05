#include "XML.hpp"
#include "querys.hpp"
#include<string>
std::string parseCreateXML(connection* C,pugi::xml_node node) {
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
                    //std::cout << "Account info: " << stock_acct_info[0] << ", " << stock_acct_info[1] << ", " << stock_acct_info[2] << ", " << stock_acct_info[3] << std::endl;
                }
                else {
                    //std::cout << "Incorrect attibute name in account node!" << std::endl;
                    return "ERROR!!!!";
                }
            }
            double ACCOUNT_ID=std::stod(stock_acct_info[1]);
            double BALANCE=std::stod(stock_acct_info[3]);
            Account*account=find_account(C,ACCOUNT_ID);
            if(account==NULL){//Account  NOT EXIST //Left INSERT EXCEPTION
                 add_account(C,ACCOUNT_ID,BALANCE);
            	 pugi::xml_node acct_created = create_result.append_child("created");
                 acct_created.append_attribute("id") =stock_acct_info[1].c_str();
			}else{//Already Exist
				pugi::xml_node acct_error = create_result.append_child("error");
                acct_error.append_attribute("id") = stock_acct_info[1].c_str();
                std::string acct_error_msg = "ACCOUNT ALREADY EXISTS!";
                acct_error.append_child(pugi::node_pcdata).set_value(acct_error_msg.c_str());
			}
        }
        else if (std::string(nxt_node.name()) == "symbol") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "sym") {
                    sym_info.push_back(std::string(attr.name()));
                    sym_info.push_back(std::string(attr.value()));
                   // std::cout << "Symbol info: " << sym_info[0] << ", " << sym_info[1] << std::endl;
                    bool noERROR=false;
                    double share_number=0;
                    for (pugi::xml_node sym_acct = nxt_node.first_child(); sym_acct; sym_acct = sym_acct.next_sibling()) {
                    	std::vector<std::string> sym_acct_info;
                        if (std::string(sym_acct.name()) == "account") {
                        	noERROR=true;
                            for (pugi::xml_attribute acct_attr = sym_acct.first_attribute(); acct_attr; acct_attr = acct_attr.next_attribute()) {
                                if (std::string(acct_attr.name()) == "id") {
                                    sym_acct_info.push_back(acct_attr.name());
                                    sym_acct_info.push_back(acct_attr.value());
                                   // std::cout << "Symbol account info: " << sym_acct_info[0] << ", " << sym_acct_info[1] << std::endl;
                                    std::string share_num = std::string(sym_acct.child_value());
                                    share_number=std::stod(share_num);
                                    //std::cout << "Account number of shares: " << share_num << std::endl;
                                }
                                else {
                                    //std::cout << "Incorrect XML format!" << std::endl;
                                    return "Incorrect format!!!!!!!";
                                }
                            }
							double ACCOUNT_NUMBER=std::stod(sym_acct_info[1]);
                    	Account* account=find_account(C,ACCOUNT_NUMBER);
                    	if(account!=NULL){
                            //std::cout<<"yes"<<std::endl;
                    	  	std::string SYMBOL_NAME=sym_info[1];
                    	    double SHARE_AMOUNT=share_number;
                    	    add_position(C,ACCOUNT_NUMBER,SYMBOL_NAME,SHARE_AMOUNT);
                    	     pugi::xml_node sym_created = create_result.append_child("created");
                            std::string create_sym = SYMBOL_NAME;    // symbol from database
                           std::string create_sym_id = sym_acct_info[1];    // account id from database
                           sym_created.append_attribute("sym") = create_sym.c_str();
                           sym_created.append_attribute("id") = create_sym_id.c_str();	
						}else{
                            // std::cout<<"no"<<std::endl;
							pugi::xml_node sym_error = create_result.append_child("error");
                           std::string error_sym = "SYM";    // symbol from database
                           std::string error_sym_id = sym_acct_info[1];    // account id from database
                           sym_error.append_attribute("sym") = error_sym.c_str();
                           sym_error.append_attribute("id") = error_sym_id.c_str();
                           std::string sym_error_msg = "Failed to create symbol!";
                           sym_error.append_child(pugi::node_pcdata).set_value(sym_error_msg.c_str());
						} 
                        }
                    }
                    if(noERROR==false){
                    	std::cerr<<"NOT SPECIFIED CORRECT XML FORMAT"<<std::endl;
						return "ERROR!!!!!!!!!!!!";
					}
                }
                else {
                    std::cerr << "Incorrect attibute name in symbol node!" << std::endl;
                    return "ERROR!!!!!!!!!!!!";
                }
            }
        }
        else {
            std::cerr << "Incorrect XML format!" << std::endl;
        }
    }
    std::stringstream trans;
    create_response_doc.save(trans);
    std::string trans_response = trans.str();
    std::cout<<trans_response<<std::endl;
    return trans_response;
}

std::string parseTransactionsXML(connection*C,pugi::xml_node node) {
    // generate response XML
    pugi::xml_document trans_response_doc;
    pugi::xml_node trans_result = trans_response_doc.append_child("results");
    std::string account_id;
    if (std::string(node.first_attribute().name()) == "id") {
        std::string acct_id = std::string(node.first_attribute().value());
        account_id=acct_id;
        //std::cout << "Transaction account id: " << acct_id << std::endl;
    }
    else {
       // std::cout << "Incorrect XML format!" << std::endl;
        return "ERROR!!!!!!!!!!!!";
    }
    Account* tempAccount=find_account(C,std::stod(account_id));
    if(tempAccount==NULL){
    	//account not exist
    		pugi::xml_node order_error = trans_result.append_child("error");
//               std::string error_sym = order_info[1];    // symbol from database
//               std::string error_amount = order_info[3];    // amount id from database
//               std::string error_limit = order_info[5];    // limit id from database
//               order_error.append_attribute("sym") = error_sym.c_str();
//               order_error.append_attribute("amount") = error_amount.c_str();
//            order_error.append_attribute("limit") = error_limit.c_str();
            std::string order_error_msg = "ACCOUNT NOT EXIST!!!!!!";
            order_error.append_child(pugi::node_pcdata).set_value(order_error_msg.c_str());
	}else{
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
                    //std::cout << "Order info: " << order_info[0] << ", " << order_info[1] << ", "<< order_info[2] << ", "<< order_info[3] << ", "<< order_info[4] << ", "<< order_info[5] << std::endl;
                }
                else {
                    //std::cout << "Incorrect XML format!" << std::endl;
                    return "ERROR!!!!!!!";
                }
            }
            time_t curTime=std::time(0);
            std::string timeNow=std::string(std::asctime(std::localtime(&curTime)));
             std::cout<<"failed reason!!!!"<<std::endl;
              int order_id=add_orders(C,order_info[1],std::stod(order_info[3]),std::stod(order_info[5]),0,0,timeNow,std::stod(account_id),timeNow,0);
              std::cout<<"Why failed!!!!!!"<<std::endl;
			  Order*matched_order=match_order(C,order_info[1],std::stod(order_info[3]),std::stod(order_info[5]));
			  if(matched_order!=NULL){
                   //std::cout<<"find matched order!!!!!!!!!!!!"<<std::endl;
			  		execute_order(C,order_id,std::stod(account_id),order_info[1],std::stod(order_info[3]),std::stod(order_info[5]),matched_order);
			  }
              pugi::xml_node order_opened = trans_result.append_child("opened");
              std::string opened_sym = order_info[1];    // symbol from database
              std::string opened_amount = order_info[3];    // amount id from database
              std::string opened_limit = order_info[5];
			  std::stringstream ss;
			  ss<<order_id;
			  std::string transaction_id=ss.str();    // limit id from database
              order_opened.append_attribute("sym") = opened_sym.c_str();
              order_opened.append_attribute("amount") = opened_amount.c_str();
              order_opened.append_attribute("limit") = opened_limit.c_str();
              order_opened.append_attribute("id")  = transaction_id.c_str();
            // try{
            //   std::cout<<"failed reason!!!!"<<std::endl;
            //   int order_id=add_orders(C,order_info[1],std::stod(order_info[3]),std::stod(order_info[5]),0,0,timeNow,std::stod(account_id),timeNow,0);
            //   std::cout<<"Why failed!!!!!!"<<std::endl;
			//   Order*matched_order=match_order(C,order_info[1],std::stod(order_info[3]),std::stod(order_info[5]));
			//   if(matched_order!=NULL){
			//   		execute_order(C,std::stod(account_id),order_info[1],std::stod(order_info[3]),std::stod(order_info[5]),matched_order);
			//   }
            //   pugi::xml_node order_opened = trans_result.append_child("opened");
            //   std::string opened_sym = order_info[1];    // symbol from database
            //   std::string opened_amount = order_info[3];    // amount id from database
            //   std::string opened_limit = order_info[5];
			//   std::stringstream ss;
			//   ss<<order_id;
			//   std::string transaction_id=ss.str();    // limit id from database
            //   order_opened.append_attribute("sym") = opened_sym.c_str();
            //   order_opened.append_attribute("amount") = opened_amount.c_str();
            //   order_opened.append_attribute("limit") = opened_limit.c_str();
            //   order_opened.append_attribute("id")  = transaction_id.c_str();
			// }catch(std::exception e){
            //     std::cout<<"ADD ORDER FAILED!!!!!!!!!!!!!!!"<<"The reason is"<<e.what()<<std::endl;
			// 	pugi::xml_node order_error = trans_result.append_child("error");
            //    std::string error_sym = order_info[1];    // symbol from database
            //    std::string error_amount = order_info[3];    // amount id from database
            //    std::string error_limit = order_info[5];    // limit id from database
            //    order_error.append_attribute("sym") = error_sym.c_str();
            //    order_error.append_attribute("amount") = error_amount.c_str();
            //    order_error.append_attribute("limit") = error_limit.c_str();
            // std::string order_error_msg = std::string(e.what());
            // order_error.append_child(pugi::node_pcdata).set_value(order_error_msg.c_str());
			// }
        }
        else if (std::string(nxt_node.name()) == "query") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "id") {
                    query_info.push_back(std::string(attr.name()));
                    query_info.push_back(std::string(attr.value()));
                    //std::cout << "Query info: " << query_info[0] << ", " << query_info[1] << std::endl;
                }
                else {
                    //std::cout << "Incorrect XML format!" << std::endl;
                }
            }
            std::string query_trans_id = query_info[1]; // transaction id from database
            int query_id=(int)std::stod(query_trans_id);
            std::map<std::string,std::string> query_result=query_order(C,query_id);
            // add <status> node for query
            pugi::xml_node query = trans_result.append_child("status");
            Order* a_order=find_order(C,query_id);
            if(a_order!=NULL){
            	std::string query_open_share = query_result.find("open")->second;    // from database
            std::string query_cancel_share = query_result.find("canceled")->second;    // from database
            std::string query_cancel_time = query_result.find("canceled-time")->second;    // PLease ADD time!!!!
            std::string query_execute_share = query_result.find("executed")->second;    // from database
            std::string query_execute_price = query_result.find("execute-price")->second;    // Price!!!!!!!!!!!!!
            std::string query_execute_time = query_result.find("executed-time")->second;    // Time!!!!!!
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
			}else{
				pugi::xml_node query_error = trans_result.append_child("error");
              std::string error_query_trans_id = query_trans_id;    // transaction id from database
              std::string query_error_msg = "Failed to query the order!";
              query_error.append_child(pugi::node_pcdata).set_value(query_error_msg.c_str());
			}
        }
        else if (std::string(nxt_node.name()) == "cancel") {
            for (pugi::xml_attribute attr = nxt_node.first_attribute(); attr; attr = attr.next_attribute()) {
                if (std::string(attr.name()) == "id") {
                    cancel_info.push_back(std::string(attr.name()));
                    cancel_info.push_back(std::string(attr.value()));
                    //std::cout << "Cancel info: " << cancel_info[0] << ", " << cancel_info[1] << std::endl;
                }
                else {
                    //std::cout << "Incorrect XML format!" << std::endl;
                }
            }
            //transaction id not exist@!!!!!!!!
            cancel_order(C,atoi(cancel_info[1].c_str()));
            std::map<std::string,std::string> map=query_order(C,atoi(cancel_info[1].c_str()));
            // add <canceled> node for cancel
            pugi::xml_node cancel = trans_result.append_child("canceled");
            std::string cancel_trans_id = cancel_info[1];    // transaction id from database
            int c_order_id=(int)std::stod(cancel_trans_id);
            Order* c_order=find_order(C,c_order_id);
            if(c_order!=NULL){
                 std::stringstream s4;
            s4<<map.find("canceled")->second;
            std::string cancel_cancel_share =s4.str();    // from database
            std::stringstream s5;
            s5<<map.find("executed")->second;
            std::string cancel_cancel_time = map.find("canceled-time")->second;    // Time!!!!!!
            std::string cancel_execute_share = s5.str();    // from database
            std::string cancel_execute_price = map.find("execute-price")->second;    // Price!!!!!!
            std::string cancel_execute_time = map.find("executed-time")->second;    // Time!!!!!!!!
            cancel.append_attribute("id") = cancel_trans_id.c_str();
            pugi::xml_node cancel_canceled = cancel.append_child("open");
            cancel_canceled.append_attribute("shares") = cancel_cancel_share.c_str();
            cancel_canceled.append_attribute("time") = cancel_cancel_time.c_str();
            pugi::xml_node cancel_executed = cancel.append_child("executed");
            cancel_executed.append_attribute("shares") = cancel_execute_share.c_str();
            cancel_executed.append_attribute("price") = cancel_execute_price.c_str();
            cancel_executed.append_attribute("time") = cancel_execute_time.c_str();	
			}else{
				pugi::xml_node cancel_error = trans_result.append_child("error");
              std::string error_cancel_trans_id = "1";    // transaction id from database
              std::string cancel_error_msg = "Failed to cancel the order!";
              cancel_error.append_child(pugi::node_pcdata).set_value(cancel_error_msg.c_str());
			}
        }
    }
	}
    // trans_response_doc.save_file("output.xml");
    std::stringstream trans;
    trans_response_doc.save(trans);
    std::string trans_response = trans.str();
    std::cout<<trans_response<<std::endl;
    return trans_response;
}

std::string parseXML(connection*C,std::string xmlstring) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xmlstring.c_str());
    // std::cout << "result: " << result << std::endl;
    if (!result) {
        //std::cout << "Parsing failed!" << std::endl;
        return "Incorrect format!!!!!!!";
    }
    
    if (doc.child("create")) {
        pugi::xml_node node_create = doc.child("create");
        return  parseCreateXML(C,node_create);
    }
    else if (doc.child("transactions")) {
        pugi::xml_node node_transactions = doc.child("transactions");
        return parseTransactionsXML(C,node_transactions);
    }
    else {
        return "Incorrect format";
    }
}

