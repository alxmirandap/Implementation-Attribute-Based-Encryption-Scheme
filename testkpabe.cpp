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

#ifndef DEF_DEF_KPABE
#include "kpabe.h"
#endif

unsigned int polNAttr = 5;
unsigned int nattr = 20;
 
vector<int> pol_parts;


//-----------------------------------------------------------
//------------------- Main Level ----------------------------


int test1(int errors, KPABE& testclass, PFC& m_pfc, G1 &P, G2 &Q, Big order){
  //------------------ Test 1: scheme params generation ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 1");
   
  G1 temp1 = m_pfc.mult(P,order+1);
  G2 temp2 = m_pfc.mult(Q,order+1);

  test_diagnosis("Test 1: number of attributes", testclass.numberAttr() == nattr, errors);
  test_diagnosis("Test 1: P^order == 1", temp1 == P, errors);
  test_diagnosis("Test 1: Q^order == 1", temp2 == Q, errors);
    
  return errors;
}

int test2(int errors, KPABE& testclass, PFC& m_pfc, G1 &P, G2 &Q){  
  //------------------ Test 2: scheme setup ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 2");

  vector<Big> &privateKeyAtts = testclass.getPrivateAttributes();
  Big& privateKeyBlinder = testclass.getPrivateKeyRand();
  GT& publicCTBlinder = testclass.getPublicCTBlinder();

#ifdef AttOnG1_KeyOnG2
  vector<G1> &publicKeyAtts = testclass.getPublicAttributes();
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> &publicKeyAtts = testclass.getPublicAttributes();
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


int test3(int errors, KPABE& testclass, PFC& m_pfc, miracl *mip, G1 &P, G2 &Q){
  //------------------ Test 3: Encryption ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 3");

  GT& publicCTBlinder  = testclass.getPublicCTBlinder();

#ifdef AttOnG1_KeyOnG2
  vector<G1> &publicKeyAtts = testclass.getPublicAttributes();
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> &publicKeyAtts = testclass.getPublicAttributes();
#endif


  vector<int> BadCTatts;  // one invalid attribute index
  BadCTatts.push_back(4);
  BadCTatts.push_back(2);
  BadCTatts.push_back(17);
  BadCTatts.push_back(6);
  BadCTatts.push_back(20);
  BadCTatts.push_back(5);

  vector<int> CTatts; // all valid attribute indices
  CTatts.push_back(4);
  CTatts.push_back(2);
  CTatts.push_back(17);
  CTatts.push_back(6);
  CTatts.push_back(12);
  CTatts.push_back(5);

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

  bool success = testclass.encryptS(BadCTatts, sM, sCT, AttFrags);

  test_diagnosis("Test 3 - hash encrypt: bad attributes, return fail", !success, errors);
    
  success = testclass.encryptS(CTatts, sM, sCT, AttFrags);
  test_diagnosis("Test 3 - hash encrypt: size of attribute frags", AttFrags.size() == CTatts.size(), errors);
  CTrand = testclass.getLastEncryptionRandomness();
  test_diagnosis("Test 3 - hash encrypt: good attributes, succeed", success, errors);
  aux = m_pfc.power(publicCTBlinder, CTrand);
  test_diagnosis("Test 3 - hash encrypt: CT well-formedness", lxor(sM,m_pfc.hash_to_aes_key(aux)) == sCT, errors);


  success = testclass.encrypt(BadCTatts, M, CT, AttFrags);

  test_diagnosis("Test 3 - mult encrypt: bad attributes, return fail", !success, errors);
    
  success = testclass.encrypt(CTatts, M, CT, AttFrags);
  test_diagnosis("Test 3 - mult encrypt: size of attribute frags", AttFrags.size() == CTatts.size(), errors);
  CTrand = testclass.getLastEncryptionRandomness();
  test_diagnosis("Test 3 - mult encrypt: good attributes, succeed", success, errors);
  aux = m_pfc.power(publicCTBlinder, CTrand);
  test_diagnosis("Test 3 - mult encrypt: CT well-formedness", (M * aux) == CT, errors);

  //  test_diagnosis("Test 3: CT well-formedness", (M * aux) == CT, errors);

  //  CT=lxor(M,m_pfc.hash_to_aes_key(blinder));


  for (unsigned int i = 0; i < CTatts.size(); i++){
    ss << "Test 3 - " << i << ": attribute fragments well-formedness";

#ifdef AttOnG1_KeyOnG2
    G1 temp1 = m_pfc.mult(publicKeyAtts[CTatts[i]], CTrand);
    test_diagnosis(ss.str(), temp1 == AttFrags[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
    G2 temp2 = m_pfc.mult(publicKeyAtts[CTatts[i]], CTrand);
    test_diagnosis(ss.str(), temp2 == AttFrags[i], errors);
#endif

    ss.str("");
  }

  return errors;
}

int test4(int errors, KPABE& testclass, PFC& m_pfc, G1 &P, G2 &Q, Big order){
  //------------------ Test 4: Key Generation ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 4");

  stringstream ss;
    
  Big& privateKeyBlinder = testclass.getPrivateKeyRand();
  vector<Big> &privateKeyAtts = testclass.getPrivateAttributes();


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

  shared_ptr<SecretSharing> scheme = testclass.getScheme();
  shared_ptr<AccessPolicy> policy = scheme->getPolicy();
  std::vector<ShareTuple> shares = scheme->distribute_random(privateKeyBlinder);   

  
  vector<Big> randomness = scheme->getDistribRandomness();

  Big privateAtt;

  DEBUG("------------ Start GEN KEY --------------");
#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags = testclass.genKey(randomness);
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags = testclass.genKey(randomness);
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


int test5(int errors, KPABE& testclass, PFC& m_pfc, miracl* mip, G1 &P, G2 &Q, vector<int> authCTatts, vector<int> badCTatts, vector<int> unauthCTatts){
  //------------------ Test 5: Decryption ------------------------
  OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
  OUT("Beginning of test 5");

  shared_ptr<SecretSharing> scheme = testclass.getScheme();
  shared_ptr<AccessPolicy> policy = scheme->getPolicy();

  //  ShamirAccessPolicy policy(threshold, pol_parts, order);
  // Big& privateKeyBlinder = testclass.getPrivateKeyRand();
  // ShamirSS shamir(policy, m_pfc);
  // std::vector<ShareTuple> shares = shamir.distribute_random(privateKeyBlinder);   
    
  // DEBUG("[TEST5] Distributed secret (t): " << privateKeyBlinder);

#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags = testclass.genKey();
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags = testclass.genKey();
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
  success = testclass.encryptS(authCTatts, sM, sCT, AttFrags);
  test_diagnosis("Test 5: string encryption with authorized attributes", success, errors);

  success = testclass.decryptS(keyFrags, authCTatts, sCT, AttFrags, sPT);

  mip->IOBASE=256;
  DEBUG("[TEST5] found plaintext : " << sPT);
  mip->IOBASE=16;


  test_diagnosis("Test 5: string decryption with authorized attributes success", success, errors);
  test_diagnosis("Test 5: string decryption with authorized attributes equality", sPT == sM, errors);
    
  success = testclass.encryptS(unauthCTatts, sM, sCT, BadAttFrags);
  test_diagnosis("Test 5: string encryption with unauthorized attributes", success, errors);
    
  success = testclass.decryptS( keyFrags, unauthCTatts, sCT, BadAttFrags, sPT);
  test_diagnosis("Test 5: string decryption with unauthorized attributes", !success, errors);


  // -----------------

  Big rand;
  m_pfc.random(rand); // picking a random message
  GT pair = m_pfc.pairing(Q,P);
  const GT GroupM = m_pfc.power(pair, rand);
  GT GroupPT; // plaintetx
  GT GroupCT; // main part of ciphertext

  DEBUG("[TEST5] plaintext to be encrypted : " << m_pfc.hash_to_aes_key(GroupM));

  success = testclass.encrypt(authCTatts, GroupM, GroupCT, AttFrags);
  test_diagnosis("Test 5: algebraic encryption with authorized attributes", success, errors);

  success = testclass.decrypt( keyFrags, authCTatts, GroupCT, AttFrags, GroupPT);

  DEBUG("[TEST5] found plaintext : " << m_pfc.hash_to_aes_key(GroupPT));


  test_diagnosis("Test 5: algebraic decryption with authorized attributes success", success, errors);
  test_diagnosis("Test 5: algebraic decryption with authorized attributes equality", GroupPT == GroupM, errors);
    
  success = testclass.encrypt(unauthCTatts, GroupM, GroupCT, BadAttFrags);
  test_diagnosis("Test 5: algebraic encryption with unauthorized attributes", success, errors);
    
  success = testclass.decrypt( keyFrags, unauthCTatts, GroupCT, BadAttFrags, GroupPT);
  test_diagnosis("Test 5: algebraic decryption with unauthorized attributes", !success, errors);


  return errors;
}

  
void debugVectorBig(string s, vector<Big> vec){    
  for (unsigned int i = 0; i < vec.size(); i++) {
    DEBUG(s << "(" << i << ") = " << vec[i]);
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


int runTests(std::string &testName, KPABE testclass, PFC &pfc, miracl *mip, G1 &P, G2 &Q, Big order,
	     vector<int> authCTAtts, vector<int> badCTAtts, vector<int> unauthCTAtts ){
  testName = "Test KPABE: " + testName;
  int errors = 0;

  errors += test1(errors, testclass, pfc, P, Q, order);
  errors += test2(errors, testclass, pfc, P, Q);
  errors += test3(errors, testclass, pfc, mip, P, Q);
  errors += test4(errors, testclass, pfc, P, Q, order);
  errors += test5(errors, testclass, pfc, mip, P, Q, authCTAtts, badCTAtts, unauthCTAtts);

  return errors;
}

void classSetup(KPABE &testclass, G1 &P, G2 &Q, Big &order, int nparts){
  testclass.paramsgen(P, Q, order);
  testclass.setup();
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


  std::string expr = op_OR + "(1, " + op_AND + "(2,3,4), " + op_AND + "(2,5), " + op_AND + "(4,5))";
  shared_ptr<BLAccessPolicy> policy = make_shared<BLAccessPolicy>(expr, polNAttr);
  shared_ptr<BLSS> testScheme = make_shared<BLSS>(policy, pfc);
  
  //  DEBUG("Created BL");

  KPABE testClass(testScheme, pfc, nattr);

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

  vector<int> authCTatts; // all valid attribute indices, authorized set
  authCTatts.push_back(7);
  authCTatts.push_back(8);
  authCTatts.push_back(2);
  authCTatts.push_back(4);
  authCTatts.push_back(19);
  authCTatts.push_back(3);

  vector<int> badCTatts;  // one invalid attribute index
  badCTatts.push_back(7);
  badCTatts.push_back(8);
  badCTatts.push_back(2);
  badCTatts.push_back(4);
  badCTatts.push_back(29);
  badCTatts.push_back(3);

  vector<int> unauthCTatts; // all valid attributes, unauthorized set
  unauthCTatts.push_back(3);
  unauthCTatts.push_back(11);
  unauthCTatts.push_back(17);
  unauthCTatts.push_back(9);
  unauthCTatts.push_back(12);
  unauthCTatts.push_back(5);
  unauthCTatts.push_back(15);
  unauthCTatts.push_back(11);


  int result = runTests(test_name, testClass, pfc, mip, P, Q, order, authCTatts, badCTatts, unauthCTatts);
  print_test_result(result, test_name);
  return 0;
}
