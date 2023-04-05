#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <climits>
#include <ctime>
#include <error.h>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include<pthread.h>
#include "XML.hpp" 
#include "querys.hpp"
#include "ThreadInfo.hpp" 
 int build_server(const char * port) {
  const char * hName = NULL;
  struct addrinfo hInfo;
  struct addrinfo * hInfoList;
  int status;
  int socket_fd;

  memset(&hInfo, 0, sizeof(hInfo));
  hInfo.ai_family = AF_UNSPEC;
  hInfo.ai_socktype = SOCK_STREAM;
  hInfo.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hName, port, &hInfo, &hInfoList);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hName << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (strcmp(port, "") == 0) {
    struct sockaddr_in * addr_in = (struct sockaddr_in *)(hInfoList->ai_addr);
    addr_in->sin_port = 0;
  }

  socket_fd = socket(hInfoList->ai_family,
                     hInfoList->ai_socktype,
                     hInfoList->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hName << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, hInfoList->ai_addr, hInfoList->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hName << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  status = listen(socket_fd, 100);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hName<< "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(hInfoList);
  return socket_fd;
}
int server_accept(int socket_fd){
	 struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connect_fd;

  client_connect_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connect_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  return client_connect_fd;
}
void* handle(void* fd){
    ThreadInfo* myInfo=(ThreadInfo*)fd;
    int client_fd=myInfo->clientFd;
    connection*C=myInfo->C;
    std::string temp="";
    std::string message="";
    char* messageSize=new char[100];
    int recvLen=recv(client_fd,messageSize,sizeof(messageSize),0);
    if(recvLen==-1){
      std::cerr<<"receive error!!!!!!!"<<std::endl;
    }
	  std::string messageSizeNUM=std::string(messageSize,recvLen);
    int expectSize=atoi(messageSizeNUM.c_str());
    int curSize=0;
    delete messageSize;
    std::string totalMessage="";
    std::cout<<"Message size!!!!!!"<<expectSize<<std::endl;
    while(curSize<expectSize){
      char* msize=new char[1];
      int recvLength=recv(client_fd,msize,sizeof(msize),0);
      if(recvLength==-1){
         std::cerr<<"error!!!"<<std::endl;
         break;
      }
      totalMessage+=std::string(msize,recvLength);
      curSize+=recvLength;
      std::cout<<"Length is"<<curSize<<std::endl;
      std::cout<<totalMessage<<std::endl;
      std::string last_str=totalMessage.substr(totalMessage.length()-1,1);
      int index=totalMessage.find("</create>");
      int indexTwo=totalMessage.find("</transactions>");
      if(index!=std::string::npos||indexTwo!=std::string::npos){
        break;
      }
      delete msize;
    }
    // char*total_size=new char[expectSize];
    // int recvLength=recv(client_fd,total_size,sizeof(total_size),0);
    // totalMessage=std::string(total_size,recvLength);
    std::cout<<"Message:::"<<totalMessage<<std::endl;
	//Now we get the whole XML message;
	std::string response=parseXML(C,totalMessage);
	std::cout<<"Parse task finished@!!!!!!"<<std::endl; 
  send(client_fd,response.c_str(),sizeof(response),0);
  std::cout<<"Parse taskB finished@!!!!!!"<<std::endl; 
  //return response;
    close(client_fd);
    std::cout<<"Parse taskA finished@!!!!!!"<<std::endl; 
	return NULL;
}
int main(){
	int serverSocket=build_server("12345");
	std::cout<<"SUCCESSS!!!!!!!!!!!"<<std::endl;
	while(true){
    connection* db=database_init("start.sql");
		int clientFd=server_accept(serverSocket);
		pthread_t thread;
		ThreadInfo*myInfo=new ThreadInfo(db,clientFd);
		pthread_create(&thread,NULL,handle,myInfo);
	}
	close(serverSocket);
	return 0; //start!
}
