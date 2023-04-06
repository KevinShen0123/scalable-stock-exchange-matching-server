#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

void testCilent(std::string filename) {
  // read an XML file as a string
  std::ifstream xmlfile;
  std::string line;
  std::string xmlstring = "";
  xmlfile.open(filename);
  if (xmlfile.is_open()) {
    while (getline(xmlfile, line)) {
      xmlstring += line + "\n";
    }
    // std::cout << "xmlstring: " << xmlstring << std::endl;
    xmlfile.close();
  }
  else {
    std::cout << "Unable to open the XML file." << std::endl;
  }
  // std::cout << "string length: " << xmlstring.length() << std::endl;
  // std::cout << "string size: " << sizeof(xmlstring) << std::endl;

  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = "127.0.0.1";
  const char *port = "12345";

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return;
  }

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return;
  }
  
  std::cout << "Connecting to " << hostname << " on port " << port << "..." << std::endl;
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return;
  } 

  const char *message = xmlstring.c_str();
  send(socket_fd, message, strlen(message), 0);

  int buffer_size = 65536;
  std::vector<char> buff;
  int recvLen;
  int dataLen = 0;
  while (1) {
    buff.resize(dataLen + buffer_size);
    recvLen = recv(socket_fd, &buff.data()[dataLen], 65536, 0);
    dataLen += recvLen;
    buff.resize(dataLen);
    if (std::string(buff.begin(), buff.end()).find("</results>") != std::string::npos) {
      break;
    }
    // std::cout << std::string(buff.begin(), buff.end()) << std::endl;
  }

  std::cout << std::string(buff.begin(), buff.end()) << std::endl;

  freeaddrinfo(host_info_list);
  close(socket_fd);
}

int main() {
  testCilent("functest/test0.xml");
  testCilent("functest/test1.xml");
  testCilent("functest/test2.xml");

  return 0;
}