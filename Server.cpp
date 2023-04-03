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
    int*fds=(int*)fd;
    int client_fd=fds[0];
	int size;
	if (recv(client_fd, &size, sizeof(size), 0) == -1) {
        std::cerr << "Failed to receive message size." << std::endl;
        exit(1);
    }
    int total_size=0;
    std::string message;
    while(total_size<size){
    	char* curMessage=new char[100];
    	int curSize=recv(client_fd,curMessage,sizeof(curMessage),0);
    	if(curSize==-1){
    		delete curMessage;
    		continue;
		}else{
			std::string xmls=std::string(curMessage,curSize);
			message+=xmls;
			total_size+=curSize;
			delete curMessage;
		}
	}
	//Now we get the whole XML message;
	return NULL;
}

int main(){
	int serverSocket=build_server("12345");
	while(true){
		int clientFd=server_accept(serverSocket);
		pthread_t thread;
		pthread_create(&thread,NULL,handle,&clientFd);
	}
	return 0; 
}
