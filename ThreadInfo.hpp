#include <iostream>
#include <pqxx/pqxx>
#include<sstream>
#include<cstring>
#include<fstream>
#include <iomanip>
#include <string>
#include<vector>
#include<algorithm>
class ThreadInfo{
	public:
	 connection *C;
	 int clientFd;
	 ThreadInfo(connection*C,int clientFd){
	 	this->C=C;
	 	this->clientFd=clientFd;
	 }
};
