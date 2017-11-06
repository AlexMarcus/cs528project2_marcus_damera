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

using namespace std;

//Global variables:
//vector of tuples representing each this nodes distance vector

vector<tuple<int, int, int>> distance_vectors;
pthread_mutex_t mutex; //used whenever accessing/updates any value in distance_vectors



struct NetworkData {
  int port;
  string hostname;
};

void *DataPortThread(void *);
void *ControlPortThread(void *);

int main(int argc, char **argv){

  if(argc != 4){ cout << "Wrong Number of Args" << endl; exit(0);}

  int id = atoi(argv[1]);
  int cport = atoi(argv[2]);
  int dport = atoi(argv[3]);

  ifstream file("config.txt");

  Node *this_node;
  vector<int> neighbors;
  //figures out info about THIS node
  
  string line;
  while(getline(file,line)){
    istringstream iss(line);
    
    string token;
    getline(iss,token,'\t');

    if(id == stoi(token)){
      getline(iss,token,'\t');
      this_node =  new Node(id, cport, dport, token);
      int count = 2;
      while(getline(iss,token,'\t')){
	switch(count){
	case 2:
	  assert(cport == stoi(token));
	  break;
	case 3:
	  assert(dport == stoi(token));
	  break;
	default:
	  neighbors.push_back(stoi(token));
	}
	count++;
      }
    }

    file.clear();
    file.seekg(0, file.beg); //points to beginning of file to get info about neighbors

    while(getline(file,line)){
      int id_, control_port_, data_port_;
      string hostname_;

      istringstream iss(line);

      string token;
      getline(iss,token,'\t');
      if(find(neighbors.begin(), neighbors.end(), stoi(token)) != neighbors.end()){
	id_ = stoi(token);
	int count = 1;
	while(getline(iss,token,'\t')){
	  switch(count){
	  case 1:
	    hostname_ = token;
	    break;
	  case 2:
	    control_port_ = stoi(token);
	    break;
	  case 3:
	    data_port_ = stoi(token);
	    break;
	  }
	  count++;
	}
	Node *neighbor_node = new Node(id_, control_port_, data_port_, hostname_);
	this_node->AddNeighbor(neighbor_node);
      }     
    }
  }
  
  pthread_t data_thread, control_thread;
  int rv;

  rv = pthread_create(&data_thread, NULL, &DataPortThread, (void *) &this_node); assert(rv==0);
  rv = pthread_create(&control_thread, NULL, &ControlPortThread, (void *) &this_node); assert(rv==0);

  rv = pthread_join(data_thread, NULL); assert(rv==0);
  rv = pthread_join(control_thread, NULL); assert(rv==0);  

  return 0;
}

void *DataPortThread(void *args){
  Node *node_data = (Node *) args;



  
  cout << "DataportThread" << endl;
  
}

void *ControlPortThread(void *args){

  Node *node_data = (Node *) args;
  //create socket

  int sd, rv;

  sd = socket(AF_INET, SOCK_DGRAM, 0); assert(sd > 0);

  // bind socket to control port # from args

  struct sockaddr_in sa;
  
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(node_data->GetControlPort()); 
  
  rv = bind(sd, (struct sockaddr *) &sa, sizeof(sa)); assert(rv ==0);

  int interval = 1;
  
  time_t start_time, now, prev;
  start_time = time(NULL);
  prev = start_time;
  
  while(1){
    now = time(NULL);
    if(now - start_time > 10){interval = 5;}
    if(now - prev > interval){
      cout << "Do something" << endl;
      prev = now;
    }
  }
  
  //wait for incoming messages
  //initially every .5 seconds each node will send its distsnce vectors for initialization
  //after 10 seconds, they will send their distsance vectors every 5 seconds
  
  
  cout << "ControlPortThread" << endl;  

}
