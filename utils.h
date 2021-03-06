/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file holds some utilities that are used in the rest of the source code.
  These utilities are mainly used for tests and debug.

  The line #include "utils_impl.tcc" holds implementations for the template functions, in order for these functions to be available still in the header file.

*/

//********* choose just one of these pairs **********
#define MR_PAIRING_BN    // AES-128 security
#define AES_SECURITY 128
//*********************************************

#define UTILS_H

#define DEF_UTILS

#include <sstream>
#include <string>
#include <iostream>
#include <ctime>
#include <assert.h>
#include "pairing_3.h"
#include <vector>
#include <map>
#include <typeinfo> // For std::bad_cast
#include <stdexcept>
#include <memory> // for smart pointers
#include <cmath>

#define shRED "\x1b[1;31m"
#define shLRED "\x1b[0;31m"
#define shWHITE "\x1b[0;37m"
#define shBWHITE "\x1b[1;37m"
#define shYELLOW "\x1b[1;33m"
#define shCYAN "\x1b[0;36m"
#define shGREEN "\x1b[1;32m"

#define NODEBUG

#ifdef NODEBUG
#  define DEBUG(x) do {} while (0)
#  define ENHDEBUG(x) do {} while (0)
#endif

#ifndef NODEBUG
#  define DEBUG(x) cout << shRED "[DEBUG:] " shWHITE  <<  x << "\n"
#  define ENHDEBUG(x) cout << shBWHITE "[DEBUG:] " shYELLOW  <<  x << shWHITE "\n"
#endif

#define OUT(x) cout << shGREEN "[OUTPUT:] " shWHITE << x << "\n"
#define ENHOUT(x) cout << shGREEN "[OUTPUT:] " shBWHITE << x << "\n"
#define REPORT(x) cout << shLRED "[REPORT:] " shCYAN << x << shWHITE "\n"

#define ERR_BAD_POLICY "BAD_POLICY"
#define ERR_BAD_TREE "BAD_TREE"
#define ERR_CONVERSION "ERR_CONV"
#define ERR_BAD_SHARE "BAD_SHARE"

const std::string op_OR = "OR";
const std::string op_AND = "AND";
const std::string op_THR = "THR";

void print_test_result(int result, const string& name);
void guard(string s, bool b);
void test_diagnosis(const string& name, bool success, int& errors);

template<typename T> int contains(const vector<T> &set, const T element);
template<typename T> void addVector(vector<T> &storage, const vector<T> &data);
template<typename T> void debugVector(std::string text, vector<T> list);
template<typename T> void debugVectorObj(std::string text, vector<T> list);
template<typename T> void outVector(vector<T> list, std::string text);
// implementation of the template function goes in the next file
#include "utils_impl.tcc"

int convertStrToInt(std::string s);
std::string convertIntToStr(int n);
void exprTokenize(const std::string &expr, vector<std::string> &tokens, const std::string delim, const std::string context_begin, const std::string context_end);
std::string trim(std::string s);
bool isSuffix(std::string& s1, std::string& s2);


