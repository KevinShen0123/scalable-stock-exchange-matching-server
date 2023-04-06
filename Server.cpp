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
pthread_mutex_t lock;
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
    int buffer_size=65536;
    char*buffer=new char[buffer_size];
    int recvLength=recv(client_fd,buffer,65536,0);
    if(recvLength==-1){
      std::cerr<<"Error!!!!!"<<std::endl;
      exit(1);
    }
    std::string first_receive=std::string(buffer,recvLength);
    std::cout << first_receive << std::endl;
    delete buffer; 
    int index=first_receive.find_first_of("\n");
    if(index==-1){
      std::cerr<<"Incorrect format!!!!!"<<std::endl;
      exit(1);
    }
    std::string number_str=first_receive.substr(0,index);
    int expectSize=atoi(number_str.c_str());
    std::string left_str=first_receive.substr(index+1,first_receive.length()-index-1);
    expectSize-=left_str.length();
    int curSize=0;
    std::string totalMessage=left_str;
    while(curSize<expectSize){
      char*num=new char[100];
      int recvSize=recv(client_fd,num,100,0);
      if(recvSize==-1){
        delete num;
        exit(1);
      }else{
          curSize+=std::string(num,recvSize).length();
          totalMessage+=std::string(num,recvSize);
          delete num; 
      }
    }
	  std::string response=parseXML(C,totalMessage);
    std::cout << response << std::endl;
    send(client_fd,response.c_str(),response.length(),0);
    C->disconnect();
    close(client_fd);
	return NULL;
}
int main(){
	int serverSocket=build_server("12345");
  connection* db=connect_database();
  create_database(db,"start.sql");
  db->disconnect();
	while(true){
  //  pthread_mutex_lock(&lock);
    db=connect_database();
		int clientFd=server_accept(serverSocket);
		pthread_t thread;
		ThreadInfo*myInfo=new ThreadInfo(db,clientFd);
    //pthread_mutex_unlock(&lock);
		pthread_create(&thread,NULL,handle,myInfo);
	}
	close(serverSocket);
	return 0; //start!
}
