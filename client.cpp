#include "node.h"
#include <iostream>
#include <string>
#include <tuple>
#include <pthread.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define CSIZE 1027

using namespace std;

int main(int argc, char **argv){

  if(argc != 4){ cout << "Wrong Number of Args" << endl; exit(0);}
  
  string command = argv[1];
  int node1 = atoi(argv[2]);
  int node2 = atoi(argv[3]);

  //get all nodes data

  ifstream file("config.txt");

  file.clear();
  file.seekg(0, file.beg);

  vector<Node *> nodes;
  string line;
  while(getline(file,line)){
    istringstream iss(line);
    string token;
    int id, cport, dport;
    string hostname;
    cout << line << endl;
    getline(iss,token,'\t');
    id = stoi(token);
    getline(iss,token,'\t');
    hostname = token;
    getline(iss,token,'\t');
    cport = stoi(token);
    getline(iss,token,'\t');
    dport = stoi(token);

    Node * temp = new Node(id, cport, dport, hostname);
    nodes.push_back(temp);
  }

  //create a socket on any node to send data to the correct 
  int sd, rv;

  sd = socket(AF_INET, SOCK_DGRAM, 0); assert(sd > 0);

  // bind socket to some port

  struct sockaddr_in sa;

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(0);

  rv = bind(sd, (struct sockaddr *) &sa, sizeof(sa)); assert(rv ==0);
  int i;
  if(command == "generate-packet"){
    for(i=0;i<nodes.size();i++){
      if(nodes[i]->GetID() == node1){

	struct sockaddr_in addr;
	struct hostent *h = gethostbyname((const char *) nodes[i]->GetHostname().c_str());

	memcpy(&addr.sin_addr.s_addr, h->h_addr, h->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nodes[i]->GetControlPort());

	ssize_t sendsize;
	unsigned int address_length = sizeof(struct sockaddr);

	char buf[CSIZE];
	buf[0] = node1;
	buf[1] = node2;
	buf[2] = 2;

	sendsize = sendto(sd, buf, CSIZE, 0, (struct sockaddr *) &addr, address_length);
      }
    }
    if(i==nodes.size()-1){cout << "No such node" << endl;}
  }

  return 0;
  
}
