/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds some utilities that I want to use in many different files

  Compile with modules as specified below:

  - Compile this file as

  	g++ -O2 -m64 -c -DZZNS=4 utils.cpp -lbn -lmiracl -lpairs -o utils.o

*/

#include "utils.h"

void print_test_result(int result, const string& name){
  if (result == 0) {
    OUT( name << "Tests " shGREEN "successful!" shWHITE);
  } else {
    OUT( name << result << " tests have " shRED "failed..." shWHITE);
  }
}

void guard(string s, bool b){
  if (!b) {
    DEBUG(s);
  }
  assert(b);  
}

void test_diagnosis(const string& name, bool success, int& errors){
  string message;
  if (!success) {
    errors++;
    message = name + " has " shRED "failed" shWHITE;
  } else {
    message = name + " has " shGREEN "passed" shWHITE;
  }
  OUT(message);
}

int contains(vector<int> set, int element){
  for (int i = 0; i < set.size(); i++){
     if (element == set[i]) {
       return i;
     }
  }
  return -1;
}



