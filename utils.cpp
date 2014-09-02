/*
 * utils.cpp
 *
 *  Created on: 18 Aug 2014
 *      Author: uxah005
 */
/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds some utilities that I want to use in many different files

  Compile with modules as specified below:

  - Compile this file as

  	g++ -O2 -m64 -c -DZZNS=4 utils.cpp -lbn -lmiracl -lpairs -o utils.o

*/


#ifndef DEF_UTILS
#include "utils.h"
#endif


void print_test_result(int result, const string& name){
  if (result == 0) {
    OUT( name << ": Tests " shGREEN "successful!" shWHITE);
  } else {
    OUT( name << ": " << result << " tests have " shRED "failed..." shWHITE);
  }
}

void guard(string s, bool b){
  if (!b) {
    REPORT(s);
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

int convertStrToInt(std::string s) {
  std::stringstream convert(s);
  int n;
  char rest;

  bool b = convert >> n;
  b = b && !(convert >> rest); // this shows that the string still had a char element after it had exhausted the digits. Thus, it was not a number.
  if (!b) {
    stringstream ss(ERR_CONVERSION);
    ss << ": Could not convert string to int: " << s << std::endl;
    throw std::runtime_error(ss.str());
  }
  return n;
}

std::string convertIntToStr(int n) {
  return std::to_string(n);
}

void exprTokenize(const std::string &expr, vector<std::string> &tokens, const std::string delim, const std::string context_begin, const std::string context_end) {
  // this tokenizer divides an expression into its constituent parts. These are separated by a delimiter delim
  // up to this point, it is like a regular tokenizer. however, there is a difference
  // its arguments can have nested expressions, which the tokenizer must not dive into. For example, in an expression f(a,b,g(x,y),h(a,f(z,w))), the constituent
  // parts should be: [a], [b], [g(x,y)] and [f(z,w)]. Although a comma is present inside two of these constituents, it should not be used as a token delimiter
  // therefore, I need the notion of context, which is marked two delimiters: context_begin and context_end. If the counter context_count is greater than 0, then
  // the delimiter is not examining the top-level expression and so should not be used as a token-breaker. If the counter ever becomes negative, it is an error.
  // There is an extra complication: the context delimiter may start after an introduction of some kind, the name of the function. This must be included as part
  // of the token also.
  // Finally, we have to recognize the last token. This is what remains from the last delimiter until the end of the expression.

  int context_count = 0;
  unsigned int pos = 0;
  unsigned int token_start = 0;
  unsigned int token_finish = 0;
  const unsigned int deliml = delim.length();
  const unsigned int cbegl = context_begin.length();
  const unsigned int cendl = context_end.length();
  std::string token;

  tokens.clear();

  DEBUG("Expression: " << expr);
  while ( pos < expr.length() ) {
    //    DEBUG("Inside Loop. Position " << pos);
    if (expr.compare(pos,cbegl,context_begin) == 0) {
      //      DEBUG("Context Change UP");
      context_count++;
      pos += cbegl;
    } else
    if (expr.compare(pos,cendl,context_end) == 0) {
      //      DEBUG("Context Change DOWN");
      context_count--;
      std::string ss;
      ss = "expression tokenizer received a malformed expression, with unbalanced " + context_begin + " and " + context_end;
      guard(ss, context_count >= 0);
      pos += cendl;
    } else
    if ( (expr.compare(pos, deliml, delim) == 0) && (context_count == 0) ) {
      token_finish = pos-1;
      token = expr.substr(token_start, token_finish - token_start + 1);
      tokens.push_back(trim(token));

//       DEBUG("DELIM FOUND");
//       DEBUG("Position: " << pos);
//       DEBUG("token_start: " << token_start);
//       DEBUG("token_finish: " << token_finish);
//       DEBUG("length: " << token_finish - token_start + 1);
//       DEBUG("Found Token: " << token);
//
      pos += deliml;
      token_start = pos;
    } else
    pos++;
    //    DEBUG("Current pos: " << pos);
    //    DEBUG("--------Ending loop iteration---------");
  }
  //  DEBUG("Ended loop");
  token_finish = expr.size()-1;
  token = expr.substr(token_start, token_finish - token_start + 1);
  tokens.push_back(trim(token));
  //  DEBUG("Found Token: " << token);
}



std::string trim(std::string s) {
  std::string whitespaces (" \t\f\v\n\r");

  size_t start = s.find_first_not_of(whitespaces);
  if (start == std::string::npos) {
    return ""; // string is all whitespaces
  }
  size_t end = s.find_last_not_of(whitespaces); // no check needed, because it can only fail if the previous one had failed also
  return s.substr(start, end-start+1);
}

// tests if s2 is a suffix of s1
bool isSuffix(std::string& s1, std::string& s2) {
  if (s2.length() < s1.length()) {
    DEBUG("Distction in sizes: " << s2 << " is shorter than " << s1);
    return false;
  }
  for (unsigned int i = 0; i < s1.length(); i++) {
    if (s1[i] != s2[i]) {
      DEBUG("Distinction at position: " << i << ": " << s1[i] << "; " << s2[i]);
      return false;
    }
  }
  return true;
}
