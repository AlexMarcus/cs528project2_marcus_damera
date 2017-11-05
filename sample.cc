#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>

using namespace std;
int main(int argc,char **argv){

  int id_, control_port_, data_port_;
  std::string hostname_;
  std::vector<int> neighbors_; //only filled out if this is the active node
  std::ifstream file(argv[1]);

  /*while(file >> id_ >>hostname_ >>  control_port_ >> data_port_ ){
    cout << id_ << hostname_ << control_port_ << data_port_ <<endl;
    int neighbor;
    while(file.peek() != '\n'){
      file >> neighbor;
      cout << neighbor << endl;
    }
  }

  */
  string line;
  while(getline(file,line)){
    neighbors_.clear();
    istringstream iss(line);
    int count =0;
    string token;
    while(getline(iss,token,'\t')){
      switch(count){
      case 0:
	id_ = stoi(token);
	cout << id_ << endl;
	break;
      case 1:
	hostname_ = token;
	cout << hostname_ << endl;
	break;
      case 2:
	control_port_ = stoi(token);
	cout << control_port_ << endl;
	break;
      case 3:
	data_port_ = stoi(token);
	cout << data_port_ << endl;
	break;
      default:
	neighbors_.push_back(stoi(token));
      }
      count++;
    }
    cout << id_ << hostname_ << control_port_ << data_port_ <<endl;
    int i;
    for(i=0;i<neighbors_.size();i++){
      cout << neighbors_[i] << " ";
    }
    cout << endl;
  }

return 0;

}
