/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds a testing framework for the KPABE class.
  Note that the KPABE class has methods that reveal all sorts of private information, like the challenge randomness used or the private keys.
  This is only for testing purposes, and not for production use

  - Compile this file as

  	g++ -O2 -m64 -DZZNS=4 testkpabe.cpp shamir.o utils.o -lbn -lmiracl -lpairs -o testkpabe
*/


//----------------------------------------------------------------
//---------------------- KPABE Class ------------------------

#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#ifndef DEF_BL_CANON
#include "BLcanonical.h"
#endif

#ifndef DEF_SH_TREE
#include "ShTree.h"
#endif

#ifndef DEF_KPABE
#include "kpabe.h"
#endif

unsigned int polNAttr = 5;
unsigned int nattr = 20;
 
vector<int> pol_parts;


//-----------------------------------------------------------
//------------------- Main Level ----------------------------


int test1(int errors, KPABE& testClass, PFC& m_pfc, G1 &P, G2 &Q, Big order){
  //------------------ Test 1: scheme params generation ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 1");
   
  G1 temp1 = m_pfc.mult(P,order+1);
  G2 temp2 = m_pfc.mult(Q,order+1);

  test_diagnosis("Test 1: number of attributes", testClass.numberAttr() == nattr, errors);
  test_diagnosis("Test 1: P^order == 1", temp1 == P, errors);
  test_diagnosis("Test 1: Q^order == 1", temp2 == Q, errors);
    
  return errors;
}

int test2(int errors, KPABE& testClass, PFC& m_pfc, G1 &P, G2 &Q){  
  //------------------ Test 2: scheme setup ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 2");

  vector<Big> &privateKeyAtts = testClass.getPrivateAttributes();
  Big& privateKeyBlinder = testClass.getPrivateKeyRand();
  GT& publicCTBlinder = testClass.getPublicCTBlinder();

#ifdef AttOnG1_KeyOnG2
  vector<G1> &publicKeyAtts = testClass.getPublicAttributes();
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> &publicKeyAtts = testClass.getPublicAttributes();
#endif
    
  DEBUG("size of private key: " << privateKeyAtts.size() << " -- expected: " << nattr);
  test_diagnosis("Test 2: attribute data structures", privateKeyAtts.size() == nattr, errors);
  test_diagnosis("Test 2: attribute data structures", publicKeyAtts.size() == nattr, errors);

  stringstream ss;

  for (unsigned int i = 0; i < privateKeyAtts.size(); i++){
    ss << "Test 2 - " << i << ": attributes' computation";

#ifdef AttOnG1_KeyOnG2
    G1 temp1 = m_pfc.mult(P, privateKeyAtts[i]);
    test_diagnosis(ss.str(), temp1 == publicKeyAtts[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
    G2 temp2 = m_pfc.mult(Q, privateKeyAtts[i]);
    test_diagnosis(ss.str(), temp2 == publicKeyAtts[i], errors);
#endif

    ss.str("");
  }
  GT pair = m_pfc.pairing(Q,P);
  test_diagnosis("Test 2a: blinding factor", m_pfc.power(pair, privateKeyBlinder) == publicCTBlinder, errors);
    
  return errors;
}


int test3(int errors, KPABE& testClass, PFC& m_pfc, miracl *mip, G1 &P, G2 &Q, vector<int>& CTAtts, vector<int>& badCTAtts){
  //------------------ Test 3: Encryption ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 3");

  GT& publicCTBlinder  = testClass.getPublicCTBlinder();

#ifdef AttOnG1_KeyOnG2
  vector<G1> &publicKeyAtts = testClass.getPublicAttributes();
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> &publicKeyAtts = testClass.getPublicAttributes();
#endif

  GT CT;
  Big sCT;

#ifdef AttOnG1_KeyOnG2
  vector<G1> AttFrags;
  vector<G1> BadAttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> AttFrags;
  vector<G2> BadAttFrags;
#endif


  Big rand;
  Big CTrand;
  GT aux;

  stringstream ss;

  GT pair = m_pfc.pairing(Q,P);
  m_pfc.random(rand); // picking a random message
  const GT M = m_pfc.power(pair, rand);

  mip->IOBASE=256;
  const Big sM = (char *)"test message"; 
  mip->IOBASE=16;

  bool success = testClass.encryptS(badCTAtts, sM, sCT, AttFrags);

  test_diagnosis("Test 3 - hash encrypt: bad attributes, return fail", !success, errors);
    
  success = testClass.encryptS(CTAtts, sM, sCT, AttFrags);
  test_diagnosis("Test 3 - hash encrypt: size of attribute frags", AttFrags.size() == CTAtts.size(), errors);
  CTrand = testClass.getLastEncryptionRandomness();
  test_diagnosis("Test 3 - hash encrypt: good attributes, succeed", success, errors);
  aux = m_pfc.power(publicCTBlinder, CTrand);
  test_diagnosis("Test 3 - hash encrypt: CT well-formedness", lxor(sM,m_pfc.hash_to_aes_key(aux)) == sCT, errors);


  success = testClass.encrypt(badCTAtts, M, CT, AttFrags);

  test_diagnosis("Test 3 - mult encrypt: bad attributes, return fail", !success, errors);
    
  success = testClass.encrypt(CTAtts, M, CT, AttFrags);
  test_diagnosis("Test 3 - mult encrypt: size of attribute frags", AttFrags.size() == CTAtts.size(), errors);
  CTrand = testClass.getLastEncryptionRandomness();
  test_diagnosis("Test 3 - mult encrypt: good attributes, succeed", success, errors);
  aux = m_pfc.power(publicCTBlinder, CTrand);
  test_diagnosis("Test 3 - mult encrypt: CT well-formedness", (M * aux) == CT, errors);

  //  test_diagnosis("Test 3: CT well-formedness", (M * aux) == CT, errors);

  //  CT=lxor(M,m_pfc.hash_to_aes_key(blinder));


  for (unsigned int i = 0; i < CTAtts.size(); i++){
    ss << "Test 3 - " << i << ": attribute fragments well-formedness";

#ifdef AttOnG1_KeyOnG2
    G1 temp1 = m_pfc.mult(publicKeyAtts[CTAtts[i]], CTrand);
    test_diagnosis(ss.str(), temp1 == AttFrags[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
    G2 temp2 = m_pfc.mult(publicKeyAtts[CTAtts[i]], CTrand);
    test_diagnosis(ss.str(), temp2 == AttFrags[i], errors);
#endif

    ss.str("");
  }

  return errors;
}

int test4(int errors, KPABE& testClass, PFC& m_pfc, G1 &P, G2 &Q, Big order){
  //------------------ Test 4: Key Generation ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 4");

  stringstream ss;
    
  Big& privateKeyBlinder = testClass.getPrivateKeyRand();
  vector<Big> &privateKeyAtts = testClass.getPrivateAttributes();


#ifdef AttOnG1_KeyOnG2
  vector<G1> AttFrags;
  vector<G1> BadAttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> AttFrags;
  vector<G2> BadAttFrags;
#endif

    
  //  ShamirAccessPolicy policy(threshold, pol_parts, order);
  //  ShamirSS shamir(policy, m_pfc);

  shared_ptr<SecretSharing> scheme = testClass.getScheme();
  shared_ptr<AccessPolicy> policy = scheme->getPolicy();
  std::vector<ShareTuple> shares = scheme->distribute_random(privateKeyBlinder);   

  
  vector<Big> randomness = scheme->getDistribRandomness();

  Big privateAtt;

  DEBUG("------------ Start GEN KEY --------------");
#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags = testClass.genKey(randomness);
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags = testClass.genKey(randomness);
#endif

  DEBUG("------------ Start TEST COMPARISON --------------");

  for (unsigned int i = 0; i < policy->getNumShares(); i++){
    ss << "Test 4 - " << i << ": key fragments well-formedness";
    privateAtt = privateKeyAtts[shares[i].getPartIndex()];

#ifdef AttOnG1_KeyOnG2
    G2 tmp = m_pfc.mult(Q, moddiv(shares[i].getShare(),privateAtt,order)); // for some reason, if tmp is not defined and this expression is written as is on
    // in the next line, the compiler will interpret the result of m_pfc::mult as G2& and will not compile. 
    // This error does not occur for AttOnG2_KeyOnG1
    test_diagnosis(ss.str(), tmp == keyFrags[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
    test_diagnosis(ss.str(), m_pfc.mult(P, moddiv(shares[i].getShare(),privateAtt,order)) == keyFrags[i], errors);
#endif
    DEBUG("Iter: " << i << " Share: " << shares[i].getShare() 
	  << " Attribute: " << privateAtt);    
    ss.str("");
  }
  return errors;
}


int test5(int errors, KPABE& testClass, PFC& m_pfc, miracl* mip, G1 &P, G2 &Q, vector<int> authCTAtts, vector<int> unauthCTAtts){
  //------------------ Test 5: Decryption ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 5");

  shared_ptr<SecretSharing> scheme = testClass.getScheme();
  shared_ptr<AccessPolicy> policy = scheme->getPolicy();

  //  ShamirAccessPolicy policy(threshold, pol_parts, order);
  // Big& privateKeyBlinder = testClass.getPrivateKeyRand();
  // ShamirSS shamir(policy, m_pfc);
  // std::vector<ShareTuple> shares = shamir.distribute_random(privateKeyBlinder);   
    
  // DEBUG("[TEST5] Distributed secret (t): " << privateKeyBlinder);

#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags = testClass.genKey();
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags = testClass.genKey();
#endif
   
  mip->IOBASE=256;
  const Big sM = (char *)"hello world to be encrypted"; 
  mip->IOBASE=16;


#ifdef AttOnG1_KeyOnG2
  vector<G1> AttFrags;
  vector<G1> BadAttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> AttFrags;
  vector<G2> BadAttFrags;
#endif
    
  Big sCT;
  Big sPT;
  bool success;
  success = testClass.encryptS(authCTAtts, sM, sCT, AttFrags);
  test_diagnosis("Test 5: string encryption with authorized attributes", success, errors);

  success = testClass.decryptS(keyFrags, authCTAtts, sCT, AttFrags, sPT);

  mip->IOBASE=256;
  DEBUG("[TEST5] found plaintext : " << sPT);
  mip->IOBASE=16;


  test_diagnosis("Test 5: string decryption with authorized attributes success", success, errors);
  test_diagnosis("Test 5: string decryption with authorized attributes equality", sPT == sM, errors);
    
  success = testClass.encryptS(unauthCTAtts, sM, sCT, BadAttFrags);
  test_diagnosis("Test 5: string encryption with unauthorized attributes", success, errors);
    
  success = testClass.decryptS( keyFrags, unauthCTAtts, sCT, BadAttFrags, sPT);
  test_diagnosis("Test 5: string decryption with unauthorized attributes", !success, errors);


  // -----------------

  Big rand;
  m_pfc.random(rand); // picking a random message
  GT pair = m_pfc.pairing(Q,P);
  const GT GroupM = m_pfc.power(pair, rand);
  GT GroupPT; // plaintetx
  GT GroupCT; // main part of ciphertext

  DEBUG("[TEST5] plaintext to be encrypted : " << m_pfc.hash_to_aes_key(GroupM));

  success = testClass.encrypt(authCTAtts, GroupM, GroupCT, AttFrags);
  test_diagnosis("Test 5: algebraic encryption with authorized attributes", success, errors);

  success = testClass.decrypt( keyFrags, authCTAtts, GroupCT, AttFrags, GroupPT);

  DEBUG("[TEST5] found plaintext : " << m_pfc.hash_to_aes_key(GroupPT));


  test_diagnosis("Test 5: algebraic decryption with authorized attributes success", success, errors);
  test_diagnosis("Test 5: algebraic decryption with authorized attributes equality", GroupPT == GroupM, errors);
    
  success = testClass.encrypt(unauthCTAtts, GroupM, GroupCT, BadAttFrags);
  test_diagnosis("Test 5: algebraic encryption with unauthorized attributes", success, errors);
    
  success = testClass.decrypt( keyFrags, unauthCTAtts, GroupCT, BadAttFrags, GroupPT);
  test_diagnosis("Test 5: algebraic decryption with unauthorized attributes", !success, errors);


  return errors;
}

  
void debugVectorBig(string s, vector<Big> vec){    
  for (unsigned int i = 0; i < vec.size(); i++) {
    DEBUG( s << "(" << i << ") = " << vec[i]);
  }
}
  
void debugVectorG2(string s, vector<G2> vec, vector<Big> exps, G2 g, PFC& m_pfc){
  G2 temp2;
  for (unsigned int i = 0; i < vec.size(); i++) {
    temp2 = m_pfc.mult(g, exps[i]);
    if (temp2 == vec[i]) {
      DEBUG(s << "(" << i << ") ---> Exponent: " << exps[i]);
    } else {
      DEBUG(s << "(" << i << ") ---> Wrong exponent");
    }
  }
}


const string name() {
  return "[KPABETest class:]";
} 


int runTests(std::string &testName, KPABE testClass, PFC &pfc, miracl *mip, G1 &P, G2 &Q, Big order,
	     vector<int> authCTAtts, vector<int> badCTAtts, vector<int> unauthCTAtts ){
  testName = "Test KPABE: " + testName;
  int errors = 0;

  errors += test1(errors, testClass, pfc, P, Q, order);
  errors += test2(errors, testClass, pfc, P, Q);
  errors += test3(errors, testClass, pfc, mip, P, Q, authCTAtts, badCTAtts);
  errors += test4(errors, testClass, pfc, P, Q, order);
  errors += test5(errors, testClass, pfc, mip, P, Q, authCTAtts, unauthCTAtts);

  return errors;
}

void classSetup(KPABE &testClass, G1 &P, G2 &Q, Big &order, int nparts){
  testClass.paramsgen(P, Q, order);
  testClass.setup();
  for (int i = 0; i < nparts; i++){
    pol_parts.push_back(i);
  }
}


int main() {
  //  miracl *mip = mirsys(5000,0); // C version: this is necessary to get the MIRACL functioning, which means that then I can call Bigs and so forth.
  // Miracl precision(5,0); // C++ version for the above, together with the next line
  // miracl* mip = &precision;

  // The constructor of PFC (in bn_pair.cpp) already invokes mirsys and initializes the mip pointer.
  // Because of this, I don't do that explicitly here.
  // It also sets the base to 16, but I include that here for clarity. One should not have to read the code of library classes to understand this code

  //  DEBUG("Starting Miracl setup");

  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=16;

  //  DEBUG("Finished Miracl setup");

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  //  DEBUG("Finished rand setup");


  REPORT("Tests for KPABE scheme based on Canonical Benaloh-Leichter");
  std::string expr = op_OR + "(1, " + op_AND + "(2,3,4), " + op_AND + "(2,5), " + op_AND + "(4,5))";
  shared_ptr<BLAccessPolicy> policy = make_shared<BLAccessPolicy>(expr, polNAttr);
  shared_ptr<BLSS> testSchemeBL = make_shared<BLSS>(policy, pfc);
  
  //  DEBUG("Created BL");

  KPABE testClass(testSchemeBL, pfc, nattr);

  //  DEBUG("Created KPABE");

  G1 P;
  G2 Q;
  Big order = pfc.order();
  
  classSetup(testClass, P, Q, order, nattr);
  std::string test_name = "BL canonical";

  //  DEBUG("Finished KPABE Setup");

  //  DEBUG("Running tests");

  // attribute indices for the ciphertext:
  // the scheme has 20 attributes
  // the policy has 5 attributes. 

  vector<int> authCTAtts; // all valid attribute indices, authorized set
  authCTAtts.push_back(7);
  authCTAtts.push_back(8);
  authCTAtts.push_back(2);
  authCTAtts.push_back(4);
  authCTAtts.push_back(19);
  authCTAtts.push_back(3);

  vector<int> badCTAtts;  // one invalid attribute index
  badCTAtts.push_back(7);
  badCTAtts.push_back(8);
  badCTAtts.push_back(2);
  badCTAtts.push_back(4);
  badCTAtts.push_back(29);
  badCTAtts.push_back(3);

  vector<int> unauthCTAtts; // all valid attributes, unauthorized set
  unauthCTAtts.push_back(3);
  unauthCTAtts.push_back(11);
  unauthCTAtts.push_back(17);
  unauthCTAtts.push_back(9);
  unauthCTAtts.push_back(12);
  unauthCTAtts.push_back(5);
  unauthCTAtts.push_back(15);
  unauthCTAtts.push_back(11);

  int result = 0;
  result += runTests(test_name, testClass, pfc, mip, P, Q, order, authCTAtts, badCTAtts, unauthCTAtts);
  print_test_result(result, test_name);

  //==============================================================================================================

  REPORT("Tests for KPABE scheme based on Tree of Shamir schemes");

  std::string exprSH = op_THR + "(2, 1, " + op_AND + "(2,3,4), " + op_THR + "(2,2,5, " + op_OR + "(1,4,5)))";
  shared_ptr<ShTreeAccessPolicy> policySH = make_shared<ShTreeAccessPolicy>(exprSH, polNAttr);
  shared_ptr<ShTreeSS> testSchemeShTree = make_shared<ShTreeSS>(policySH, pfc);
  
  //  DEBUG("Created ShTree");

  KPABE testClassSH(testSchemeShTree, pfc, nattr);

  
  classSetup(testClassSH, P, Q, order, nattr);
  std::string test_nameSH = "Shamir Tree";


  vector<int> authCTAttsSH; // all valid attribute indices, authorized set
  authCTAttsSH.push_back(7);
  authCTAttsSH.push_back(8);
  authCTAttsSH.push_back(2);
  authCTAttsSH.push_back(4);
  authCTAttsSH.push_back(19);
  authCTAttsSH.push_back(3);

  vector<int> badCTAttsSH;  // one invalid attribute index
  badCTAttsSH.push_back(7);
  badCTAttsSH.push_back(8);
  badCTAttsSH.push_back(2);
  badCTAttsSH.push_back(4);
  badCTAttsSH.push_back(29);
  badCTAttsSH.push_back(3);

  vector<int> unauthCTAttsSH; // all valid attributes, unauthorized set
  unauthCTAttsSH.push_back(3);
  unauthCTAttsSH.push_back(11);
  unauthCTAttsSH.push_back(17);
  unauthCTAttsSH.push_back(9);
  unauthCTAttsSH.push_back(12);
  unauthCTAttsSH.push_back(5);
  unauthCTAttsSH.push_back(15);
  unauthCTAttsSH.push_back(11);


  result += runTests(test_nameSH, testClassSH, pfc, mip, P, Q, order, authCTAttsSH, badCTAttsSH, unauthCTAttsSH);
  print_test_result(result, test_nameSH);


  return 0;
}
