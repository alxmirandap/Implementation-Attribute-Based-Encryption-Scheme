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
#include <assert.h>
#include "pairing_3.h"
#include <vector>



using namespace std;

#define DEBUG_BUILD

#define shRED "\e[1;31m"
#define shWHITE "\e[0;37m"
#define shGREEN "\e[1;32m"

#ifdef DEBUG_BUILD
#  define DEBUG(x) cout << shRED "[DEBUG:] " shWHITE  <<  x << "\n"
#else
#  define DEBUG(x) do {} while (0)
#endif

#ifdef DEBUG_BUILD
#define OUT(x) cout << shGREEN "[OUTPUT:] " shWHITE << x << "\n"
#else
#  define OUT(x) do {} while (0)
#endif

extern void guard(string s, bool b);


