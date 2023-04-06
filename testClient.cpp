#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>


int main() {
  // read an XML file as a string
  std::ifstream xmlfile;
  std::string line;
  std::string xmlstring = "";
  xmlfile.open("test0.xml");
  if (xmlfile.is_open()) {
    while (getline(xmlfile, line)) {
      xmlstring += line;
    }
    // std::cout << "xmlstring: " << xmlstring << std::endl;
    xmlfile.close();
  }
  else {
    std::cout << "Unable to open the XML file." << std::endl;
  }
  std::cout << "string length: " << xmlstring.length() << std::endl;
  std::cout << "string size: " << sizeof(xmlstring) << std::endl;

  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = "127.0.0.1";
  const char *port = "12345";
  
  // if (argc < 2) {
  //     std::cout << "Syntax: client <hostname>\n" << std::endl;
  //     return 1;
  // }

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  
  std::cout << "Connecting to " << hostname << " on port " << port << "..." << std::endl;
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  } 

  const char *message = xmlstring.c_str();
  send(socket_fd, message, strlen(message), 0);

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}