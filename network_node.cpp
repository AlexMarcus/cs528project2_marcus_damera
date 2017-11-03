#include "node.h"
#include <iostream>
#include <string>
#include <tuple>
#include <pthread.h>
#include <assert.h>

using namespace std;

//Global variables:
//vector of tuples representing each this nodes distance vector

vector<tuple<int, int, int>> distance_vectors;
pthread_mutex_t mutex; //used whenever accessing/updates any value in distance_vectors

void *DataPortThread(void *);
void *ControlPortThread(void *);

int main(){

  pthread_t data_thread, control_thread;
  int rv;

  rv = pthread_create(&data_thread, NULL, &DataPortThread, NULL); assert(rv==0);
  rv = pthread_create(&control_thread, NULL, &ControlPortThread, NULL); assert(rv==0);

  rv = pthread_join(data_thread, NULL); assert(rv==0);
  rv = pthread_join(control_thread, NULL); assert(rv==0);
  

  return 0;

}

void *DataPortThread(void *args){
  cout << "DataportThread" << endl;
}

void *ControlPortThread(void *args){
  cout << "ControlPortThread" << endl;  
}
