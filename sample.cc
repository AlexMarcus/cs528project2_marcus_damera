#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
using namespace std;
int main(int argc,char **argv){

  int id_, control_port_, data_port_;
  std::string hostname_;
//  std::vector<int> neighbors_; //only filled out if this is the active node
  std::ifstream file(argv[1]);

  while(file >> id_ >>hostname_ >>  control_port_ >> data_port_ ){
 
         cout << id_ << hostname_ << control_port_ << data_port_ <<endl;
  }

return 0;

}
