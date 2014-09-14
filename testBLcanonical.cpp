/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file holds tests for the classes implemented in BLcanonical.cpp.
  The tests are divided in 2 groups: the access policy and the secret sharing scheme.
*/

#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_BL_CANON
#include "BLcanonical.h"
#endif

int testParseExpression() {
  int errors = 0;

  BLAccessPolicy testPolicy;
  std::string base = "testParseExpression - [";
  
  std::string expr = "";
  vector<vector<int> > minimalSets = testPolicy.parseFromExpression(0,expr);
  vector<vector<int> > emptySet; 
    test_diagnosis(base + expr + "]", minimalSets == emptySet, errors);
    
  expr = "a1"; // literals must be integers, not strings
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }


  expr =  op_OR + "(1,2,"; // missing closing )
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr = op_AND + "(1,2,3),4"; // extra content beyond )
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr = op_AND + "(1,2,3)"; // DNF must start with an OR )
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

    expr = op_OR + "(" + op_OR + "(1,2) ,3)"; // DNF must not contain OR at lower leve )
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr = op_THR + "(3,1,2)"; // THR is not a recognized operator in BL
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

    expr = op_OR + "(" + op_AND + "(3,1)," + op_AND + "(1, " + op_AND + "(3,4) ) )"; // DNF can not have more than two levels
  try {
    minimalSets = testPolicy.parseFromExpression(0,expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

    // valid expressions:

    vector<vector<int> > verifSet; 
      vector<int> minSet; 

   expr = "1";
   verifSet.clear();
   minSet.clear();
   minSet.push_back(1);
   verifSet.push_back(minSet);
try {
   minimalSets = testPolicy.parseFromExpression(0,expr);
   test_diagnosis(base + expr + "]", minimalSets == verifSet, errors);
} catch (std::exception &e) {
 test_diagnosis(base + expr + "]", false, errors);
}


  expr = op_OR + "(1)";
      verifSet.clear();
      minSet.clear();
      minSet.push_back(1); 
      verifSet.push_back(minSet);
  try {
      minimalSets = testPolicy.parseFromExpression(0,expr);
      test_diagnosis(base + expr + "]", minimalSets == verifSet, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }

      expr = op_OR + "(1,2,3)"; 
      verifSet.clear();
      minSet.clear();
      minSet.push_back(1); 
      verifSet.push_back(minSet);
      minSet.clear();
      minSet.push_back(2); 
      verifSet.push_back(minSet);
      minSet.clear();
      minSet.push_back(3); 
      verifSet.push_back(minSet);

  try {
      minimalSets = testPolicy.parseFromExpression(0,expr);
      test_diagnosis(base + expr + "]", minimalSets == verifSet, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }

  expr = op_OR + "(1," + op_AND + "(2,3))";
  verifSet.clear();
  minSet.clear();
  minSet.push_back(1);
  verifSet.push_back(minSet);
  minSet.clear();
  minSet.push_back(2);
  minSet.push_back(3);
  verifSet.push_back(minSet);

try {
  minimalSets = testPolicy.parseFromExpression(0,expr);
  test_diagnosis(base + expr + "]", minimalSets == verifSet, errors);
} catch (std::exception &e) {
test_diagnosis(base + expr + "]", false, errors);
}

 
      expr = op_OR + "(" + op_AND + "(3,4,5)," + op_AND + "(1,2)," + op_AND + "(4,1,6,5))";
      verifSet.clear();
      minSet.clear();
      minSet.push_back(3); 
      minSet.push_back(4); 
      minSet.push_back(5); 
      verifSet.push_back(minSet);
      minSet.clear();
      minSet.push_back(1); 
      minSet.push_back(2); 
      verifSet.push_back(minSet);
      minSet.clear();
      minSet.push_back(4); 
      minSet.push_back(1); 
      minSet.push_back(6); 
      minSet.push_back(5); 
      verifSet.push_back(minSet);

  try {
      minimalSets = testPolicy.parseFromExpression(0,expr);
      test_diagnosis(base + expr + "]", minimalSets == verifSet, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }

    return errors;
}


int testVectors(int errors, vector<vector<ShareTuple> > testRun, vector<vector<ShareTuple> > witnessRun, BLAccessPolicy pol, std::string expr) {
  ENHDEBUG("===============");
  ENHDEBUG("Testing vectors");

  vector<ShareTuple> witnessVector;
  for (unsigned int i = 0; i < testRun.size(); i++) {
    witnessVector.clear();
    for (unsigned int j = 0; j < testRun[i].size(); j++) {
      ENHDEBUG("Shares vector: " << j << " -- " << testRun[i][j].to_string());
    }   
    for (unsigned int j = 0; j < witnessRun[i].size(); j++) {
      ENHDEBUG("Verif vector: " << j << " -- " << witnessRun[i][j].to_string());
    }

    bool b = pol.evaluate(testRun[i], witnessVector);
    DEBUG("Returned from evaluate: " << b);

    DEBUG("Printing witnessVector");
    for (unsigned int j = 0; j < witnessVector.size(); j++) {
      ENHDEBUG("Witness vector: " << j << " -- " << witnessVector[j].to_string());
    }
    DEBUG("Finished Printing witnessVector");

    bool verif_b;
    stringstream ss("");
    ss << "testEvaluate " << expr << ": " << i;
    if (witnessRun[i].empty()) {
      verif_b = false;
      DEBUG("Witness vector is empty. Target: " << verif_b);
    } else {
      verif_b = true;
      DEBUG("Witness vector has elements. Target: " << verif_b);
    }
    test_diagnosis(ss.str(), b == verif_b, errors);
    test_diagnosis(ss.str() + " - Witness vector", witnessVector == witnessRun[i], errors);

  }

  return errors;
}


int testExpr1(int errors) {

  std::string expr1 = op_OR + "(1,2,3)";
  BLAccessPolicy pol1(expr1, 5);

  ShareTuple s11(1,0,"1:1");
  ShareTuple s12(2,0,"2:2");
  ShareTuple s13(3,0,"3:3");
  ShareTuple s14(4,0,"4:4");
  ShareTuple s15(5,0,"5:5");

  vector<ShareTuple> ex1Test1;
  vector<ShareTuple> ex1Test2;
  vector<ShareTuple> ex1Test3;
  vector<ShareTuple> ex1Test4;
  vector<ShareTuple> ex1Test5;
  vector<ShareTuple> ex1Test42;
  vector<ShareTuple> ex1Test23;
  vector<ShareTuple> ex1Test231;

  vector<ShareTuple> ex1Witness1;
  vector<ShareTuple> ex1Witness2;
  vector<ShareTuple> ex1Witness3;
  vector<ShareTuple> ex1Witness4;
  vector<ShareTuple> ex1Witness5;
  vector<ShareTuple> ex1Witness42;
  vector<ShareTuple> ex1Witness23;
  vector<ShareTuple> ex1Witness231;

  ex1Test1.push_back(s11);
  ex1Test2.push_back(s12);
  ex1Test3.push_back(s13);
  ex1Test4.push_back(s14);
  ex1Test5.push_back(s15);
  ex1Test42.push_back(s14);
  ex1Test42.push_back(s12);
  ex1Test23.push_back(s12);
  ex1Test23.push_back(s13);
  ex1Test231.push_back(s12);
  ex1Test231.push_back(s13);
  ex1Test231.push_back(s11);

  ex1Witness1.push_back(s11);
  ex1Witness2.push_back(s12);
  ex1Witness3.push_back(s13);
  ex1Witness42.push_back(s12);
  ex1Witness23.push_back(s12);
  ex1Witness231.push_back(s11);

  vector<vector<ShareTuple> > ex1TestRun;
  vector<vector<ShareTuple> > ex1WitnessRun;

  ex1TestRun.push_back(ex1Test1);
  ex1TestRun.push_back(ex1Test2);
  ex1TestRun.push_back(ex1Test3);
  ex1TestRun.push_back(ex1Test4);
  ex1TestRun.push_back(ex1Test5);
  ex1TestRun.push_back(ex1Test42);
  ex1TestRun.push_back(ex1Test23);
  ex1TestRun.push_back(ex1Test231);

  ex1WitnessRun.push_back(ex1Witness1);
  ex1WitnessRun.push_back(ex1Witness2);
  ex1WitnessRun.push_back(ex1Witness3);
  ex1WitnessRun.push_back(ex1Witness4);
  ex1WitnessRun.push_back(ex1Witness5);
  ex1WitnessRun.push_back(ex1Witness42);
  ex1WitnessRun.push_back(ex1Witness23);
  ex1WitnessRun.push_back(ex1Witness231);  

  errors = testVectors(errors, ex1TestRun, ex1WitnessRun, pol1, expr1);

  return errors;
}

int testExpr2(int errors) {

  std::string expr2 = op_OR + "(" + op_AND + "(1,2)," + op_AND + "(3,4))";
  BLAccessPolicy pol2(expr2, 4);

  ShareTuple s11(1,0,"1:1");
  ShareTuple s12(2,0,"1:2");
  ShareTuple s13(3,0,"1:3");
  ShareTuple s22(2,0,"2:2");
  ShareTuple s23(3,0,"2:3");
  ShareTuple s24(4,0,"2:4");
  

  vector<ShareTuple> ex2TestA1;
  vector<ShareTuple> ex2TestA2;
  vector<ShareTuple> ex2TestA1A2;
  vector<ShareTuple> ex2TestB3B4;
  vector<ShareTuple> ex2TestB2B3B4;
  vector<ShareTuple> ex2TestA1B4A2;
  vector<ShareTuple> ex2TestA1B2B3B4;
  vector<ShareTuple> ex2TestA1A3B4B2;

  vector<ShareTuple> ex2WitnessA1;
  vector<ShareTuple> ex2WitnessA2;
  vector<ShareTuple> ex2WitnessA1A2;
  vector<ShareTuple> ex2WitnessB3B4;
  vector<ShareTuple> ex2WitnessB2B3B4;
  vector<ShareTuple> ex2WitnessA1B4A2;
  vector<ShareTuple> ex2WitnessA1B2B3B4;
  vector<ShareTuple> ex2WitnessA1A3B4B2;

  ex2TestA1.push_back(s11);
  ex2TestA2.push_back(s12);
  ex2TestA1A2.push_back(s11);
  ex2TestA1A2.push_back(s12);
  ex2TestB3B4.push_back(s23);
  ex2TestB3B4.push_back(s24);
  ex2TestB2B3B4.push_back(s22);
  ex2TestB2B3B4.push_back(s23);
  ex2TestB2B3B4.push_back(s24);
  ex2TestA1B4A2.push_back(s11);
  ex2TestA1B4A2.push_back(s24);
  ex2TestA1B4A2.push_back(s12);
  ex2TestA1B2B3B4.push_back(s11);
  ex2TestA1B2B3B4.push_back(s22);
  ex2TestA1B2B3B4.push_back(s23);
  ex2TestA1B2B3B4.push_back(s24);
  ex2TestA1A3B4B2.push_back(s11);
  ex2TestA1A3B4B2.push_back(s13);
  ex2TestA1A3B4B2.push_back(s24);
  ex2TestA1A3B4B2.push_back(s22);

  ex2WitnessA1A2.push_back(s11);
  ex2WitnessA1A2.push_back(s12);
  ex2WitnessB3B4.push_back(s23);
  ex2WitnessB3B4.push_back(s24);
  ex2WitnessB2B3B4.push_back(s23);
  ex2WitnessB2B3B4.push_back(s24);
  ex2WitnessA1B4A2.push_back(s11);
  ex2WitnessA1B4A2.push_back(s12);
  ex2WitnessA1B2B3B4.push_back(s23);
  ex2WitnessA1B2B3B4.push_back(s24);


  vector<vector<ShareTuple> > ex2TestRun;
  vector<vector<ShareTuple> > ex2WitnessRun;

  ex2TestRun.push_back(ex2TestA1);
  ex2TestRun.push_back(ex2TestA2);
  ex2TestRun.push_back(ex2TestA1A2);
  ex2TestRun.push_back(ex2TestB3B4);
  ex2TestRun.push_back(ex2TestB2B3B4);
  ex2TestRun.push_back(ex2TestA1B4A2);
  ex2TestRun.push_back(ex2TestA1B2B3B4);
  ex2TestRun.push_back(ex2TestA1A3B4B2);

  ex2WitnessRun.push_back(ex2WitnessA1);
  ex2WitnessRun.push_back(ex2WitnessA2);
  ex2WitnessRun.push_back(ex2WitnessA1A2);
  ex2WitnessRun.push_back(ex2WitnessB3B4);
  ex2WitnessRun.push_back(ex2WitnessB2B3B4);
  ex2WitnessRun.push_back(ex2WitnessA1B4A2);
  ex2WitnessRun.push_back(ex2WitnessA1B2B3B4);
  ex2WitnessRun.push_back(ex2WitnessA1A3B4B2);

  errors = testVectors(errors, ex2TestRun, ex2WitnessRun, pol2, expr2);

  return errors;
}

int testExpr3(int errors) {

  std::string expr3 = op_OR + "(" + op_AND + "(3,4,5)," + op_AND + "(1,2)," + op_AND + "(4,1,6,5))";
  BLAccessPolicy pol3(expr3, 6);

  ShareTuple s11(1,0,"1:1");
  ShareTuple s12(2,0,"1:2");
  ShareTuple s13(3,0,"1:3");
  ShareTuple s14(4,0,"1:4");
  ShareTuple s15(5,0,"1:5");
  ShareTuple s21(1,0,"2:1");
  ShareTuple s22(2,0,"2:2");
  ShareTuple s23(3,0,"2:3");
  ShareTuple s31(1,0,"3:1");
  ShareTuple s32(2,0,"3:2");
  ShareTuple s33(3,0,"3:3");
  ShareTuple s34(4,0,"3:4");
  ShareTuple s35(5,0,"3:5");
  ShareTuple s36(6,0,"3:6");
  

  vector<ShareTuple> ex3TestA3A4A5;
  vector<ShareTuple> ex3TestB1B2;
  vector<ShareTuple> ex3TestC1C2C3C4C5C6;
  vector<ShareTuple> ex3TestA4C6C5B1;
  vector<ShareTuple> ex3TestA1A2B3B1C4B2;

  vector<ShareTuple> ex3WitnessA3A4A5;
  vector<ShareTuple> ex3WitnessB1B2;
  vector<ShareTuple> ex3WitnessC1C2C3C4C5C6;
  vector<ShareTuple> ex3WitnessA4C6C5B1;
  vector<ShareTuple> ex3WitnessA1A2B3B1C4B2;


  ex3TestA3A4A5.push_back(s13);
  ex3TestA3A4A5.push_back(s14);
  ex3TestA3A4A5.push_back(s15);

  ex3TestB1B2.push_back(s21);
  ex3TestB1B2.push_back(s22);

  ex3TestC1C2C3C4C5C6.push_back(s31);
  ex3TestC1C2C3C4C5C6.push_back(s32);
  ex3TestC1C2C3C4C5C6.push_back(s33);
  ex3TestC1C2C3C4C5C6.push_back(s34);
  ex3TestC1C2C3C4C5C6.push_back(s35);
  ex3TestC1C2C3C4C5C6.push_back(s36);

  ex3TestA4C6C5B1.push_back(s14);
  ex3TestA4C6C5B1.push_back(s36);
  ex3TestA4C6C5B1.push_back(s35);
  ex3TestA4C6C5B1.push_back(s21);

  ex3TestA1A2B3B1C4B2.push_back(s11);
  ex3TestA1A2B3B1C4B2.push_back(s12);
  ex3TestA1A2B3B1C4B2.push_back(s23);
  ex3TestA1A2B3B1C4B2.push_back(s21);
  ex3TestA1A2B3B1C4B2.push_back(s34);
  ex3TestA1A2B3B1C4B2.push_back(s22);


  ex3WitnessA3A4A5.push_back(s13);
  ex3WitnessA3A4A5.push_back(s14);
  ex3WitnessA3A4A5.push_back(s15);

  ex3WitnessB1B2.push_back(s21);
  ex3WitnessB1B2.push_back(s22);

  ex3WitnessC1C2C3C4C5C6.push_back(s34);
  ex3WitnessC1C2C3C4C5C6.push_back(s31);
  ex3WitnessC1C2C3C4C5C6.push_back(s36);
  ex3WitnessC1C2C3C4C5C6.push_back(s35);

  ex3WitnessA1A2B3B1C4B2.push_back(s21);
  ex3WitnessA1A2B3B1C4B2.push_back(s22);

  vector<vector<ShareTuple> > ex3TestRun;
  vector<vector<ShareTuple> > ex3WitnessRun;

  ex3TestRun.push_back(ex3TestA3A4A5);
  ex3TestRun.push_back(ex3TestB1B2);
  ex3TestRun.push_back(ex3TestC1C2C3C4C5C6);
  ex3TestRun.push_back(ex3TestA4C6C5B1);
  ex3TestRun.push_back(ex3TestA1A2B3B1C4B2);

  ex3WitnessRun.push_back(ex3WitnessA3A4A5);
  ex3WitnessRun.push_back(ex3WitnessB1B2);
  ex3WitnessRun.push_back(ex3WitnessC1C2C3C4C5C6);
  ex3WitnessRun.push_back(ex3WitnessA4C6C5B1);
  ex3WitnessRun.push_back(ex3WitnessA1A2B3B1C4B2);

  errors = testVectors(errors, ex3TestRun, ex3WitnessRun, pol3, expr3);

  return errors;
}


int testEvaluate() {
  int errors = 0;

  errors += testExpr1(errors);
  errors += testExpr2(errors);
  errors += testExpr3(errors);

  return errors;
}

int testGetNumShares() {
	int errors = 0;

	std::string base = "TestGetNumShares: ";

	std::string expr0 = "1";
	BLAccessPolicy pol0(expr0, 3);
	test_diagnosis(base + expr0, pol0.getNumShares() == 1, errors);


	std::string expr1 = op_OR + "(1,2,3)";
	BLAccessPolicy pol1(expr1, 3);
	test_diagnosis(base + expr1, pol1.getNumShares() == 3, errors);

	std::string expr2 = op_OR + "(" + op_AND + "(1,2)," + op_AND + "(3,4))";
	BLAccessPolicy pol2(expr2, 4);
	test_diagnosis(base + expr2, pol2.getNumShares() == 4, errors);

	std::string expr3 = op_OR + "(" + op_AND + "(3,4,5),7," + op_AND + "(1,2)," + op_AND + "(4,1,6,5))";
	BLAccessPolicy pol3(expr3, 7);
	test_diagnosis(base + expr3, pol3.getNumShares() == 10, errors);


	return errors;
}

int testGetSharesForParticipants(PFC &pfc) {
  int errors = 0;

  ENHDEBUG("Creating instances");
  std::string expr = op_OR + "(1, " + op_AND + "(2,3,4), " + op_AND + "(2,5), " + op_AND + "(4,5))";
  shared_ptr<BLAccessPolicy> policy = make_shared<BLAccessPolicy>(expr, 5);
  BLSS testScheme(policy, pfc.order(), pfc);

  ENHDEBUG("Creating shares");
  ShareTuple s1a(1, 0, "1a");
  ShareTuple s1b(1, 0, "1b");
  ShareTuple s2a(2, 0, "2a");
  ShareTuple s3a(3, 0, "3a");
  ShareTuple s3b(3, 0, "3b");
  ShareTuple s3c(3, 0, "3c");
  ShareTuple s4a(4, 0, "4a");
  ShareTuple s5a(5, 0, "5a");

  vector<ShareTuple> shares;

  shares.push_back(s1a);
  shares.push_back(s1b);
  shares.push_back(s2a);
  shares.push_back(s3a);
  shares.push_back(s3b);
  shares.push_back(s3c);
  shares.push_back(s4a);
  shares.push_back(s5a);

  vector<int> parts313;
  vector<int> parts276;
  vector<int> parts45;

  parts313.push_back(3);
  parts313.push_back(1);
  parts313.push_back(3);

  parts276.push_back(2);
  parts276.push_back(7);
  parts276.push_back(6);

  parts45.push_back(4);
  parts45.push_back(5);

  vector<ShareTuple> verif313;
  vector<ShareTuple> verif276;
  vector<ShareTuple> verif45;

  verif313.push_back(s1a);
  verif313.push_back(s1b);
  verif313.push_back(s3a);
  verif313.push_back(s3b);
  verif313.push_back(s3c);

  verif276.push_back(s2a);
  
  verif45.push_back(s4a);
  verif45.push_back(s5a);
  
  std::string base = "testGetSharesForParticipants - ";

  ENHDEBUG("Calling diagnosis");
  DEBUG("call 1");
  vector<ShareTuple> returnedShares = SecretSharing::getSharesForParticipants(parts313, shares);
  test_diagnosis(base + "[313]", returnedShares == verif313, errors);

  DEBUG("call 2");
  returnedShares = SecretSharing::getSharesForParticipants(parts276, shares);
  test_diagnosis(base + "[276]", returnedShares == verif276, errors);

  DEBUG("call 3");
  returnedShares = SecretSharing::getSharesForParticipants(parts45, shares);
  test_diagnosis(base + "[45]", returnedShares == verif45, errors);

  ENHDEBUG("Finished diagnosis");	
  return errors;
}

int testReconFromShares(vector<int> party, std::string message,
			BLSS scheme, vector<ShareTuple> shares, bool goodParty,
			unsigned int expectedNumberShares, Big orig_secret) {

  int errors = 0;

  ENHDEBUG("testReconFromShares");
  for (unsigned int i = 0; i < party.size(); i++) {
    DEBUG("Party: " << party[i]);
  }

  for (unsigned int i = 0; i < shares.size(); i++) {
    DEBUG("Shares: " << shares[i].to_string());
  }
  

  vector<ShareTuple> partShares;
  partShares = SecretSharing::getSharesForParticipants(party, shares);

  for (unsigned int i = 0; i < partShares.size(); i++) {
    DEBUG("Participant Shares: " << partShares[i].to_string());
  }


  vector<ShareTuple> witnessShares;
  test_diagnosis(message + "evaluation", scheme.evaluate(partShares, witnessShares) == goodParty, errors);

  for (unsigned int i = 0; i < witnessShares.size(); i++) {
    DEBUG("Witness Shares: " << witnessShares[i].to_string());
  }

  test_diagnosis(message + "number of shares", witnessShares.size() == expectedNumberShares, errors);
  DEBUG("number of witness shares: " << witnessShares.size());
  DEBUG("expected number of shares: " << expectedNumberShares);
  Big reced_secret = scheme.reconstruct(witnessShares);
  if (goodParty) {
    test_diagnosis(message + "reconstruction", reced_secret == orig_secret, errors);
  } else {
    test_diagnosis(message + "reconstruction", reced_secret != orig_secret, errors);
  }

  return errors;
}


int testDistributeAndReconstruct(PFC &pfc){
  int errors = 0;
  ENHDEBUG("testDistribute and Reconstruct");
  int niter = 5;
  DEBUG("Initting s");
  Big s;
  DEBUG("Initting old_s");
  Big old_s=0;
  DEBUG("Obtaining order");
  Big order = pfc.order();


  DEBUG("Creating instances");

  std::string expr = op_OR + "(1, " + op_AND + "(2,3,4), " + op_AND + "(2,5), " + op_AND + "(4,5))";
  shared_ptr<BLAccessPolicy> policy = make_shared<BLAccessPolicy>(expr, 5);
  BLSS testScheme(policy, pfc.order(), pfc);
  
  std::string base = "testDistributeAndReconstruct ";
  
  vector<int> party1;
  vector<int> party234;
  vector<int> party25;
  vector<int> party45;
  vector<int> badparty23;

  party1.push_back(1);
  
  party234.push_back(2);
  party234.push_back(3);
  party234.push_back(4);

  party25.push_back(2);
  party25.push_back(5);

  party45.push_back(4);
  party45.push_back(5);
  
  badparty23.push_back(2);
  badparty23.push_back(3);
  
  
  old_s = 0;
  for (int j = 0; j < niter; j++){
    OUT(base + "Iteration: " << j);
    pfc.random(s);
    s = s % order;
    
    DEBUG("s: " << s << "\t old s: " << old_s);
    guard("s should be random, and different from the last value or 0", s != old_s); // the probability that s is 0 or the old value should be negligible
    
    vector<ShareTuple> shares = testScheme.distribute_random(s);
    
    test_diagnosis(base + "number of shares:", shares.size() == policy->getNumShares(), errors);
    
    errors += testReconFromShares(party1, base + "[1]", testScheme, shares, true, 1, s);
    errors += testReconFromShares(party234, base + "[234]", testScheme, shares, true, 3, s);
    errors += testReconFromShares(party25, base + "[25]", testScheme, shares, true, 2, s);
    errors += testReconFromShares(party45, base + "[45]", testScheme, shares, true, 2, s);
    errors += testReconFromShares(badparty23, base + "[23]", testScheme, shares, false, 0, s);
    
    
    old_s = s;
  }
    return errors;
}

int testObtainCoveredFrags() {
  int errors = 0;

  std::string base = "testObtainCoveredFrags: ";
  
  vector<int> atts;
  atts.push_back(7);
  atts.push_back(8);
  atts.push_back(2);
  atts.push_back(4);
  atts.push_back(1);
  atts.push_back(3);
  
  vector<int> attFragIndices;
  vector<int> keyFragIndices;
  vector<std::string> coveredShareIDs;

  std::string expr = op_OR + "(1, " + op_AND + "(2,3,4), " + op_AND + "(2,5), " + op_AND + "(4,5))";
  shared_ptr<BLAccessPolicy> policy = make_shared<BLAccessPolicy>(expr, 5);
    
  vector<std::string> verifShareIDs;
  verifShareIDs.push_back("1:1");
  verifShareIDs.push_back("2:2");
  verifShareIDs.push_back("2:3");
  verifShareIDs.push_back("2:4");
  verifShareIDs.push_back("3:2");
  verifShareIDs.push_back("4:4");

  vector<int> verifKeyFragIDs;
  verifKeyFragIDs.push_back(0);
  verifKeyFragIDs.push_back(1);
  verifKeyFragIDs.push_back(2);
  verifKeyFragIDs.push_back(3);
  verifKeyFragIDs.push_back(4);
  verifKeyFragIDs.push_back(6);

  vector<int> verifAttFragIDs;
  verifAttFragIDs.push_back(4);
  verifAttFragIDs.push_back(2);
  verifAttFragIDs.push_back(5);
  verifAttFragIDs.push_back(3);
  verifAttFragIDs.push_back(2);
  verifAttFragIDs.push_back(3);

  policy->obtainCoveredFrags(atts, attFragIndices, keyFragIndices, coveredShareIDs); 

  test_diagnosis(base + "number of covered IDs", coveredShareIDs.size() == 6, errors);
  for (unsigned int i = 0; i < coveredShareIDs.size(); i++) {
    test_diagnosis(base + "coveredID " + convertIntToStr(i), coveredShareIDs[i] == verifShareIDs[i], errors);
  }

  test_diagnosis(base + "number of keyFragIndices", keyFragIndices.size() == 6, errors);
  for (unsigned int i = 0; i < keyFragIndices.size(); i++) {
    test_diagnosis(base + "keyFragIndex " + convertIntToStr(i), keyFragIndices[i] == verifKeyFragIDs[i], errors);
  }

  test_diagnosis(base + "number of attFragIndices", attFragIndices.size() == 6, errors);
  for (unsigned int i = 0; i < attFragIndices.size(); i++) {
    test_diagnosis(base + "attFragIndex " + convertIntToStr(i), attFragIndices[i] == verifAttFragIDs[i], errors);
  }
  
  return errors;
}

int runTests(PFC &pfc) {
  int errors = 0;

  // Policy tests
  ENHOUT("Secret sharing policy tests");
  errors += testParseExpression();
  errors += testEvaluate();
  errors += testGetNumShares();
  errors += testObtainCoveredFrags();
  
  // Secret Sharing tests
  ENHOUT("Secret sharing scheme tests");
  errors += testGetSharesForParticipants(pfc);
  errors += testDistributeAndReconstruct(pfc);


  return errors;
}


int main() {
  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=10;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  std::string test_name = "Test BLAccessPolicy";
  int result = runTests(pfc);
  print_test_result(result,test_name);

  return 0;
}


