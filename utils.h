/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds some utilities that I want to use in many different files

  Compile with modules as specified below:

  - Compile this file as

  	g++ -O2 -m64 -c -DZZNS=4 utils.cpp -lbn -lmiracl -lpairs -o utils.o

*/

//********* choose just one of these pairs **********
#define MR_PAIRING_BN    // AES-128 security
#define AES_SECURITY 128
//*********************************************


#include <sstream>
#include <string>
#include <iostream>
#include <ctime>
#include <assert.h>
#include "pairing_3.h"
#include <vector>

#define shRED "\e[1;31m"
#define shWHITE "\e[0;37m"
#define shGREEN "\e[1;32m"

#define NODEBUG

#ifdef NODEBUG
#  define DEBUG(x) do {} while (0)
#endif

#ifndef NODEBUG
#  define DEBUG(x) cout << shRED "[DEBUG:] " shWHITE  <<  x << "\n"
#endif



#define OUT(x) cout << shGREEN "[OUTPUT:] " shWHITE << x << "\n"

void print_test_result(int result, const string& name);
void guard(string s, bool b);
void test_diagnosis(const string& name, bool success, int& errors);
int contains(vector<int> set, int element);
