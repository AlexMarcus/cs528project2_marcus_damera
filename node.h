#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <string>
#include <vector>

class Node{

 private:
  int id_, control_port_, data_port_;
  std::string hostname_;
  std::vector<Node *> neighbors_; //only filled out if this is the active node

 public:
  Node(int id, int c_port, int d_port, std::string hname);
  int AddNeighbor(Node *neighbor); //returns 0 on success, 1 on failure
  int RemoveNeighbor(int id); //returns 0 on success, 1 on failure
  std::string GetHostname(){return hostname_;};
  int GetControlPort(){return control_port_;};
  int GetDataPort(){return data_port_;};
  int GetID(){return id_;};
  std::vector<Node *> GetNeighbors(){return neighbors_;};
};

#endif
