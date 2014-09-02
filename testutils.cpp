/*
 * testutils.cpp
 *
 *  Created on: 18 Aug 2014
 *      Author: uxah005
 */

#ifndef DEF_UTILS
#include "utils.h"
#endif

int testContains() {
  vector<int> set;
  vector<std::string> sset;
  int errors = 0;

  for (int i = 0; i < 5; i++) {
    set.push_back(5+i);
    sset.push_back(convertIntToStr(5+i));
  }
  int n = contains(set, 8);
  test_diagnosis("Test Contains - element is present", (n == 3), errors);
  n = contains(set, 10);
  test_diagnosis("Test Contains - missing element", (n == -1), errors);
  n = contains(set, 5);
  test_diagnosis("Test Contains - bounds elements: first", (n == 0), errors);
  n = contains(set, 9);
  test_diagnosis("Test Contains - bounds elements: last", (n == 4), errors);

  n = contains(sset, std::string("8"));
  test_diagnosis("String Test Contains - element is present", (n == 3), errors);
  n = contains(sset, std::string("10"));
  test_diagnosis("String Test Contains - missing element", (n == -1), errors);
  n = contains(sset, std::string("5"));
  test_diagnosis("String Test Contains - bounds elements: first", (n == 0), errors);
  n = contains(sset, std::string("9"));
  test_diagnosis("String Test Contains - bounds elements: last", (n == 4), errors);

  return errors;
}

int testAddVector() {
  int errors = 0;
  unsigned int nbase = 10;
  unsigned int nadd = 5;
  vector<int> base_vecint;
  vector<int> add_vecint;

  std::vector<std::string> base_vecstr;
  std::vector<std::string> add_vecstr;

  base_vecstr.push_back("abc");
  base_vecstr.push_back("bcd");
  base_vecstr.push_back("cde");
  base_vecstr.push_back("def");

  add_vecstr.push_back("ghi");
  add_vecstr.push_back("jkl");
  add_vecstr.push_back("mno");
  add_vecstr.push_back("pqr");
  add_vecstr.push_back("stu");
  add_vecstr.push_back("vwx");

  unsigned int nbase_s = base_vecstr.size();
  unsigned int nadd_s = add_vecstr.size();

  int j;
  j = 0;
  // j is used to add a signed int to the vector, not an unsigned one
  for (unsigned int i = 0; i < nbase; i++,j++) {
    base_vecint.push_back(j);
  }
  for (unsigned int i = 0; i < nadd; i++) {
    add_vecint.push_back(j);
  }


  addVector<int>(base_vecint, add_vecint);
  addVector<std::string>(base_vecstr, add_vecstr);

  DEBUG("base_vecint size: " << base_vecint.size());
  DEBUG("nbase + nadd: " << nbase + nadd);
  test_diagnosis("Test AddVector - resulting integer vector should have 15 elements", base_vecint.size() == nbase + nadd, errors);
  test_diagnosis("Test AddVector - added integer vector should have 5 elements", add_vecint.size() == nadd, errors);
  test_diagnosis("Test AddVector - resulting string vector should have 10 elements", base_vecstr.size() == nbase_s + nadd_s, errors);
  test_diagnosis("Test AddVector - added integer vector should have 5 elements", add_vecstr.size() == nadd_s, errors);

  stringstream ss;

  j = 0;
  for (unsigned int i = 0; i < nbase; i++,j++) {
    ss.str("");
    ss << "Test AddVector - comparison final vector and base: pos " << i;
    test_diagnosis(ss.str(), base_vecint[i] == j, errors);  // extra variable j is because the contents of the vector are not necessarily unsigned,
    // and I want to avoid casts to make sure I'm not hiding any error. j must be unsigned, so can not be declared inside the for
  }
  for (unsigned int i = 0; i < nadd; i++) {
    ss.str("");
    ss << "Test AddVector - comparison final vector and add: pos " << i + nbase;
    test_diagnosis(ss.str(), base_vecint[nbase + i] == add_vecint[i], errors);
  }
  return errors;
}

int testConvertIntToStr() {
	int errors = 0;

	std::string s;
	std::string base = "Test ConvertIntToStr - ";

	int n = 23;
	s = convertIntToStr(n);
	test_diagnosis(base + "23", s == "23", errors);

	n = 142;
	s = convertIntToStr(n);
	test_diagnosis(base + "142", s == "142", errors);

	n = -218;
	s = convertIntToStr(n);
	test_diagnosis(base + "-218", s == "-218", errors);

	return errors;
}

int testConvertStrToInt() {
  int errors = 0;
  int n;

  std::string ss;
  std::string base = "Test ConvertStrToInt - ";

  std::string s;

  s = "145";
  ss = "";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, n == 145, errors);
  } catch (std::exception &e) {
  }


  s = "145a";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, false, errors);
  } catch (std::exception &e) {
    test_diagnosis(ss, true, errors);
  }

  s = "abc";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, false, errors);
  } catch (std::exception &e) {
    test_diagnosis(ss, true, errors);
  }

  s = "b637";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, false, errors);
  } catch (std::exception &e) {
    test_diagnosis(ss, true, errors);
  }

  s = "124.24";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, false, errors);
  } catch (std::exception &e) {
    test_diagnosis(ss, true, errors);
  }

  s = "-234";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, n == -234, errors);
  } catch (std::exception &e) {
  }

  s = "0";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, n == 0, errors);
  } catch (std::exception &e) {
  }


  s = "14546377";
  ss = base + s;
  try {
    n = convertStrToInt(s);
    test_diagnosis(ss, n == 14546377, errors);
  } catch (std::exception &e) {
  }

  return errors;
}

int testExprTokenize() {
  int errors = 0;

  vector<std::string> s;
  vector< vector<std::string> > result_tokens;

  std::string ss;
  ss = "a1,a2,a3";
  s.push_back(ss);
  vector<std::string> tokens;
  tokens.push_back("a1");
  tokens.push_back("a2");
  tokens.push_back("a3");
  result_tokens.push_back(tokens);

  ss = " a4 \v,\ra5 ";
  s.push_back(ss);
  tokens.clear();
  tokens.push_back("a4");
  tokens.push_back("a5");
  result_tokens.push_back(tokens);

  ss = ",a2,a4,,";
  s.push_back(ss);
  tokens.clear();
  tokens.push_back("");
  tokens.push_back("a2");
  tokens.push_back("a4");
  tokens.push_back("");
  tokens.push_back("");
  result_tokens.push_back(tokens);



  std::string temp = op_AND + "(a7)";
  ss =  "a4,a5," + temp;
  s.push_back(ss);
  tokens.clear();
  tokens.push_back("a4");
  tokens.push_back("a5");
  tokens.push_back(temp);
  result_tokens.push_back(tokens);

  std::string temp1 = op_AND + "(a1 ,a2 ,a3)";
  std::string temp2 = op_OR + "(a1\t,a2)";
  ss = "a1 , a2, " + temp1 + "\t , a4 , " + temp2 + ", a5";
  s.push_back(ss);
  tokens.clear();
  tokens.push_back("a1");
  tokens.push_back("a2");
  tokens.push_back(temp1);
  tokens.push_back("a4");
  tokens.push_back(temp2);
  tokens.push_back("a5");
  result_tokens.push_back(tokens);

  std::string base = "Test ExprTokenize - ";
  vector<std::string> result;
  for (unsigned int i = 0; i < s.size(); i++) {
    ss = base + s[i];
    exprTokenize(s[i], result, ",", "(", ")");
    test_diagnosis(ss, result == result_tokens[i], errors);
  }



  return errors;
}


int testTrim() {
  int errors = 0;

  vector<std::string> s;
  vector<std::string> r;

  std::string temp;

  stringstream ss;
  std::string base = "Test Trim - ";

  s.push_back(" \t\f\r\v \t\n ");
  r.push_back("");

  temp = "abcd";
  s.push_back(temp);
  r.push_back(temp);

  temp = "ab \t\n\r cd";
  s.push_back(temp);
  r.push_back(temp);

  s.push_back(" \t abcd \v ");
  r.push_back("abcd");

  s.push_back("\r  abcd");
  r.push_back("abcd");

  s.push_back("abcd \n ");
  r.push_back("abcd");

  s.push_back("  \t\n\rab cd  \t\n\r");
  r.push_back("ab cd");

  for (unsigned int i = 0; i < s.size(); i++) {
    ss.str("");
    ss << base << "\"" << s[i] << "\"";
    test_diagnosis(ss.str(), trim(s[i]) == r[i], errors);
  }
  return errors;
}

int testIsSuffix(){
  int errors = 0;
  std::string s1 = "abcdef";
  std::string s2 = "abcdefghij";
  std::string s3 = "abcdjklefghij";
  std::string s4 = "xafd";

  test_diagnosis("testIsSuffix: " + s1 + "; " + s2, isSuffix(s1, s2), errors);
  test_diagnosis("testIsSuffix: " + s1 + "; " + s3, !isSuffix(s1, s3), errors);
  test_diagnosis("testIsSuffix: " + s1 + "; " + s4, !isSuffix(s1, s4), errors);
  test_diagnosis("testIsSuffix: " + s2 + "; " + s1, !isSuffix(s2, s1), errors);
  test_diagnosis("testIsSuffix: " + s1 + "; " + s1, isSuffix(s1, s1), errors);

  return errors;
}

int runTests(std::string &testName) {
  testName = "Test Utils";
  int errors = 0;

  errors += testContains();
  errors += testAddVector();
  errors += testConvertStrToInt();
  errors += testExprTokenize();
  errors += testTrim();
  errors += testIsSuffix();
  return errors;
}

int main() {
  std::string test_name;
  int result = runTests(test_name);
  print_test_result(result,test_name);

  return 0;
}
