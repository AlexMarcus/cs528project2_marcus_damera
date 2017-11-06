#include "node.h"

using namespace std;
/*
class Node{

private:
  int id_, control_port_, data_port_;
  std::string hostname_;
  std::vector<Node> neighbors_;

public:
  Node(int id, int c_port, int d_port, std::string hname);
  int AddNeighbor(Node neighbor); //returns 0 on success, 1 on failure
  int RemoveNeighbor(int id); //returns 0 on success, 1 on failure

}
*/

Node::Node(int id, int c_port, int d_port, std::string hname){
  id_ = id;
  control_port_ = c_port;
  data_port_ = d_port;
  hostname_ = hname;

  cout << "Creating Node: " << id << " cport: " << c_port << " dport: " << d_port << " hostname: " << hname << endl;
}

int Node::AddNeighbor(Node *neighbor){
  neighbors_.push_back(neighbor);
  cout << "Added Neighbor" << endl;
  return 0;
}

int Node::RemoveNeighbor(int id){return 0;}
