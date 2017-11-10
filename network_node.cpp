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
#define DSIZE 1043



using namespace std;

//Global variables:
//vector of tuples representing each this nodes distance vector

vector<tuple<int, int, int>> distance_vectors;
Node * this_node;


void *DataPortThread(void *);
void *ControlPortThread(void *);

int main(int argc, char **argv){

  if(argc != 4){ cout << "Wrong Number of Args" << endl; exit(0);}

  int id = atoi(argv[1]);
  int cport = atoi(argv[2]);
  int dport = atoi(argv[3]);

  ifstream file("config.txt");

  file.clear();
  file.seekg(0, file.beg);

  //Node * this_node;
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
	  break;
	}
	count++;
      }
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
    //cout << "" << stoi(token) << endl;
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
      Node * neighbor_node = new Node(id_, control_port_, data_port_, hostname_);
      this_node->AddNeighbor(neighbor_node);
    }
  }

  pthread_t data_thread, control_thread;
  int rv;

  rv = pthread_create(&data_thread, NULL, &DataPortThread, NULL); assert(rv==0);
  rv = pthread_create(&control_thread, NULL, &ControlPortThread, NULL); assert(rv==0);

  rv = pthread_join(data_thread, NULL); assert(rv==0);
  rv = pthread_join(control_thread, NULL); assert(rv==0);  

  return 0;
}

void *DataPortThread(void *args){

  //Node *node_data = (Node *) args;
  Node * node_data = this_node;
  int sd, rv;

  sd = socket(AF_INET, SOCK_DGRAM, 0); assert(sd > 0);

  // bind socket to control port # from args

  struct sockaddr_in sa;

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(node_data->GetDataPort());

  rv = bind(sd, (struct sockaddr *) &sa, sizeof(sa)); assert(rv ==0);

  fd_set master, read_fds;
  int fdmax = sd;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  FD_SET(sd, &master);

  struct timeval tv;
  unsigned int address_length = sizeof(struct sockaddr);

  while(1){

    read_fds = master;

    tv.tv_sec = 5;
    tv.tv_usec = 500000;

    
    
    rv = select(fdmax+1, &read_fds, NULL, NULL, &tv);
    if(rv < 0) cout << "Select ERROR!" << endl;
    else if(rv == 0) cout << "...." << endl;
    else{
      ssize_t recsize;
      struct sockaddr_in node_addr;
      char recv_data[DSIZE];
      unsigned int address_length = sizeof(struct sockaddr);
      
      if(FD_ISSET(sd, &read_fds)){
	recsize = recvfrom(sd, recv_data, DSIZE, 0, (struct sockaddr *) &node_addr, &address_length);
	int source_id = (int) recv_data[0];
	int dest_id = (int) recv_data[1];
	char * path = ((char *) recv_data) + 4;
	int pid = (int) recv_data[2];
	int TTL = (int) recv_data[3];
	if(TTL > 0){
	 
	  if(dest_id == node_data->GetID()){
	    
	    char * data = ((char *) recv_data) + 4 + 15;
	    cout << "Source: " << source_id << ", Dest: " << dest_id << ", P-id: " << pid << ", TTL: " << TTL << endl;
	    int k;
	    cout << "Path: ";
	    for(k = 0; k < 15; k++){
	      if(path[k]){
		cout << "Node " << (int) path[k] << "--->";
	      }
	    }
	    cout <<"Node " << dest_id << endl;
	    
	    cout << "Data: " << data << endl;
	  }
	  else{
	    Node * forward_to;
	    int j,i;
	    for(j=0;j<distance_vectors.size();j++){
	      int to, next, hops;
	      tie(to, next, hops) = distance_vectors[j];
	      if(to == dest_id){
		for(i=0; i < node_data->GetNeighbors().size();i++){
		  if(node_data->GetNeighbors()[i]->GetID() == next){
		    forward_to = node_data->GetNeighbors()[i];
		  }
		}
	      }
	    }

	    struct sockaddr_in f_addr;
	    struct hostent *h = gethostbyname((const char *) forward_to->GetHostname().c_str());
	    memcpy(&f_addr.sin_addr.s_addr, h->h_addr, h->h_length);
	    f_addr.sin_family = AF_INET;
	    f_addr.sin_port = htons(forward_to->GetDataPort());
	    
	    path[15-TTL] = node_data->GetID();
	    recv_data[3] = TTL - 1;
	    
	    ssize_t sendsize = sendto(sd, recv_data, DSIZE, 0, (struct sockaddr *) &f_addr, address_length);
	    cout << "Forwarding" << endl;
	  }
	}
	else{
	  cout << "TTL indicated that the packet was stuck in a loop...\nPacket Dropped" << endl;
	}
      }
    }
  
  }

  cout << "DataportThread" << endl;
  
}



void *ControlPortThread(void *args){

  Node *node_data = this_node;
  //create socket

  int sd, rv;

  sd = socket(AF_INET, SOCK_DGRAM, 0); assert(sd > 0);

  // bind socket to control port # from args

  struct sockaddr_in sa;
  
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(node_data->GetControlPort()); 
  
  rv = bind(sd, (struct sockaddr *) &sa, sizeof(sa)); assert(rv ==0);

  //periodically send this nodes distance vector every <interval> seconds
  int interval = 5;  
  time_t now, prev;
  prev = time(NULL);

  //Initialize the FD Sets used for select()
  fd_set master, read_fds;
  int fdmax = sd;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  FD_SET(sd, &master);

  struct timeval tv;
  unsigned int address_length = sizeof(struct sockaddr);

  //add myself to the DV
  auto t1 = make_tuple(node_data->GetID(), -1, 0);
  distance_vectors.push_back(t1);
  
  string sv_str = "";
  stringstream sv_ss(sv_str);
  int j;
  for(j=0;j<distance_vectors.size();j++){
    int to, next, hops;
    tie(to, next, hops) = distance_vectors[j];
    sv_ss << to << "," << next << "," << hops << "\n";
    //if(j != sendvec.size()-1) sv_ss << "\n";
  }
  int packet_id = 0;
  while(1){
    
    read_fds = master;
    
    tv.tv_sec = 5;
    tv.tv_usec = 500000;
    
    now = time(NULL);
    if(now - prev > interval){
      int i;
      for(i=0; i < node_data->GetNeighbors().size();i++){
	struct sockaddr_in neighbor_addr;
	struct hostent *h = gethostbyname((const char *) node_data->GetNeighbors()[i]->GetHostname().c_str());

	memcpy(&neighbor_addr.sin_addr.s_addr, h->h_addr, h->h_length);
	neighbor_addr.sin_family = AF_INET;
	neighbor_addr.sin_port = htons(node_data->GetNeighbors()[i]->GetControlPort());

	ssize_t sendsize;

	char buf[CSIZE];
	buf[0] = node_data->GetID();
	buf[1] = node_data->GetNeighbors()[i]->GetID();
	buf[2] = 1;
       
	char * buf_ptr = (char *) buf;
	buf_ptr += 3;
	sv_ss.str("");
	//memcpy(buf_ptr, (void *) &sendvec, sizeof(sendvec));
	for(j=0;j<distance_vectors.size();j++){
 	  int to, next, hops;
	  tie(to, next, hops) = distance_vectors[j];
	  sv_ss << to << "," << next << "," << hops << "\n";
	  //if(j != sendvec.size()-1) sv_ss << "\n";
	}

	
	
	sprintf(buf_ptr, "%s", sv_ss.str().c_str());
	sendsize = sendto(sd, buf, CSIZE, 0, (struct sockaddr *) &neighbor_addr, address_length);
      }
      prev = now;
    }
    
    rv = select(fdmax+1, &read_fds, NULL, NULL, &tv);
    if(rv < 0) cout << "Select ERROR!" << endl;
    else if(rv == 0) cout << "----" << endl;
    else{
      ssize_t recsize;
      struct sockaddr_in node_addr;
      char recv_data[CSIZE];

      if(FD_ISSET(sd, &read_fds)){
	recsize = recvfrom(sd, recv_data, CSIZE, 0, (struct sockaddr *) &node_addr, &address_length);
	//cout << "\nSize of message " << recsize << endl;
	int source_id = (int) recv_data[0];
	int dest_id = (int) recv_data[1];
	int type = (int) recv_data[2];
	char * data = ((char *) recv_data) + 3;

	//type = 2;
	//cout << "Source: " << source_id << ", Dest: " << dest_id << ", Type: " << type << endl;
	//cout << "Data: " << data << endl;
	if(type == 1){
	  cout << "Updating Distance Vectors" << endl;	
	  //INDICTAES THIS NODE RECEIVED A DISTANCE VECTOR
	  vector<tuple<int, int, int>> recvec;
	  
	  istringstream dv_ss(data);
	  string line;
	  //	  int to, next, hops;
	  while(getline(dv_ss, line)){
	    string sto, snext, shops;
	    istringstream line_ss(line);
	    getline(line_ss, sto,',');
	    getline(line_ss, snext,',');
	    getline(line_ss, shops, ',');
	    auto t = make_tuple(stoi(sto), stoi(snext), stoi(shops));
	    recvec.push_back(t);
	  }
	  
	  for(j=0;j<recvec.size();j++){
	    int to, next, hops;
	    tie(to, next, hops) = recvec[j];
	    int in = 0;
	    int k;
	    for(k = 0; k < distance_vectors.size();k++){
	      int todv, nextdv, hopsdv;
	      tie(todv, nextdv, hopsdv) = distance_vectors[k];
	      tuple<int,int,int> temp;
	      if(todv == to){
		if(nextdv == next){
		  //temp = make_tuple(to, next ,hops+1);
		  //distance_vectors[k] = temp;
		}
		else{
		  if(hopsdv > hops+1){
		    //cout << "In here where to: " << to << ", nextdv "  << nextdv << endl;
		
		    temp = make_tuple(to, source_id, hops+1);
		    distance_vectors[k] = temp;
		  }
		}
		in = 1;
	      }
	    }
	    if(in == 0){
	      auto tup = make_tuple(to, source_id, hops+1);
	      distance_vectors.push_back(tup);
	    }
	  }
	  /*
	    cout << "DV: " << endl;
	  for(j=0;j<distance_vectors.size();j++){
	    int to, next, hops;
	    tie(to, next, hops) = distance_vectors[j];
	    cout << to << "," << next << "," << hops << "\n";
	  
	  }
	  */
	}
	else if(type == 2){
	  //generate packet
	  //Send a packet with the correct to this nodes data port: it will handle forwarding
	  struct sockaddr_in neighbor_addr;
	  struct hostent *h = gethostbyname((const char *) node_data->GetHostname().c_str());
	    
	  memcpy(&neighbor_addr.sin_addr.s_addr, h->h_addr, h->h_length);
	  neighbor_addr.sin_family = AF_INET;
	  neighbor_addr.sin_port = htons(node_data->GetDataPort());
	  
	  ssize_t sendsize;
	  
	  char buf[DSIZE] = "";;
	  buf[0] = node_data->GetID();
	  buf[1] = dest_id;
	  buf[2] = packet_id++;
	  buf[3] = 15;
	  
	  char * buf_ptr = (char *) buf;
	  buf_ptr += 19;
	  
	  sv_ss.str("");

	  sv_ss << "GENERATED PACKET>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	    //memcpy(buf_ptr, (void *) &sendvec, sizeof(sendvec));
	  for(j=0;j<900;j++){
	    sv_ss << (char) ((j%90) + ' ');
	  }
	  
	  //printf("%s", sv_ss.str().c_str());
	  sprintf(buf_ptr, "%s", sv_ss.str().c_str());
	  sendsize = sendto(sd, buf, DSIZE, 0, (struct sockaddr *) &neighbor_addr, address_length);
	}
	
	
	else if(type == 3){
	  ifstream file("config.txt");

	  file.clear();
	  file.seekg(0, file.beg);

	  string line;
	  while(getline(file, line)){
	    int id_, control_port_, data_port_;
	    string hostname_;

	    istringstream iss(line);

	    string token;
	    getline(iss,token,'\t');
	    //cout << "" << stoi(token) << endl;
	    id_ = stoi(token);
	    if(id_ == dest_id){
	      
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
	      Node * neighbor_node = new Node(id_, control_port_, data_port_, hostname_);
	      rv = this_node->AddNeighbor(neighbor_node);
	      if(rv == 0){
		cout << "Added Link to Node " << dest_id << endl;
	      }
	      else{ cout << "A link already exists between this node and Node " << dest_id << endl; }
	    }
	  }
	}
	else if(type == 4){
	  
	  rv = node_data -> RemoveNeighbor(dest_id);
	  if(rv == 0) cout << "Link to Node "<<dest_id << " Removed" << endl;
	  else cout << "Link to Node "<< dest_id << " does not exist" << endl;

	  //remove from DV
	  int l;	
	  for(l=0;l<distance_vectors.size();l++){
	    int t, n, h;
	    tie(t,n,h) = distance_vectors[l];
	    if(t == dest_id){
	      distance_vectors.erase(distance_vectors.begin() + l);
	    }
	  }
	  
	  
	}
	else{
	  cout << "Control Could Not recognize packet type" << endl;
	  continue;
	} 
	

      }//IF FD_ISSET
    }//ELSE
  }//WHILE(1)
  
  //wait for incoming messages
  //initially every .5 seconds each node will send its distsnce vectors for initialization
  //after 10 seconds, they will send their distsance vectors every 5 seconds
  
  
  cout << "ControlPortThread" << endl;  

}
