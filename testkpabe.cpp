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

#include "utils.h"
#include "shamir.h"
#include "kpabe.h"

// In the future, this class must be a template parameterized by the secret sharing in use.
// For now, I will bind it to Shamir plain SSS, and later on will parameterize it.

  // template<class T> KPABETest<T>(PFC &pfc):
  // {
  // }
  
  // ~KPABETest<T>(){
  // }

class KPABETest 
{
  PFC &m_pfc;
  int nattr;     
  G1 temp1;
  G2 temp2;

  G1 P;	
  G2 Q;	
  Big order;

  miracl* mip;
  vector<int> pol_parts;
  int threshold;

  vector<Big> poly;

public:
  
  KPABETest(PFC &pfc, miracl* in_mip):
    m_pfc(pfc), nattr(20), mip(in_mip), threshold(4)
  {}
  ~KPABETest()
  {}

  int testSetup(KPABE& testclass){
    testclass.paramsgen(P, Q, order);
    testclass.setup();
    for (int i = 1; i <= 8; i++){
      pol_parts.push_back(i);
    }
  }
  
  int test1(int errors, KPABE& testclass){
  //------------------ Test 1: scheme params generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 1");
   
    temp1 = m_pfc.mult(P,order+1);
    temp2 = m_pfc.mult(Q,order+1);

    test_diagnosis("Test 1: number of attributes", testclass.numberAttr() == nattr, errors);
    test_diagnosis("Test 1: P^order == 1", temp1 == P, errors);
    test_diagnosis("Test 1: Q^order == 1", temp2 == Q, errors);
    
    return errors;
  }

  int test2(int errors, KPABE& testclass){  
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
    
    test_diagnosis("Test 2: attribute data structures", privateKeyAtts.size() == nattr, errors);
    test_diagnosis("Test 2: attribute data structures", publicKeyAtts.size() == nattr, errors);

    stringstream ss;

    for (int i = 0; i < nattr; i++){
      ss << "Test 2 - " << i << ": attributes' computation";

#ifdef AttOnG1_KeyOnG2
      temp1 = m_pfc.mult(P, privateKeyAtts[i]);
      test_diagnosis(ss.str(), temp1 == publicKeyAtts[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
      temp2 = m_pfc.mult(Q, privateKeyAtts[i]);
      test_diagnosis(ss.str(), temp2 == publicKeyAtts[i], errors);
#endif

      ss.str("");
    }
    GT pair = m_pfc.pairing(Q,P);
    test_diagnosis("Test 2a: blinding factor", m_pfc.power(pair, privateKeyBlinder) == publicCTBlinder, errors);
    
    return errors;
  }


  int test3(int errors, KPABE& testclass){
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

    int CTnatts = CTatts.size();
 
    //    GT CT;
    Big CT;

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


    // m_pfc.random(rand); // picking a random message
    //    const GT M = m_pfc.power(pair, rand);

    mip->IOBASE=256;
    const Big M = (char *)"test message"; 
    mip->IOBASE=16;

    bool success = testclass.encryptS(BadCTatts, M, CT, AttFrags);

    test_diagnosis("Test 3: bad attributes, return fail", !success, errors);
    
    success = testclass.encryptS(CTatts, M, CT, AttFrags);
    test_diagnosis("Test 3: size of attribute frags", AttFrags.size() == CTatts.size(), errors);
    CTrand = testclass.getLastEncryptionRandomness();
    test_diagnosis("Test 3: good attributes, succeed", success, errors);
    aux = m_pfc.power(publicCTBlinder, CTrand);
    //    test_diagnosis("Test 3: CT well-formedness", (M * aux) == CT, errors);

    for (int i = 0; i < CTatts.size(); i++){
      ss << "Test 3 - " << i << ": attribute fragments well-formedness";

#ifdef AttOnG1_KeyOnG2
      temp1 = m_pfc.mult(publicKeyAtts[CTatts[i]], CTrand);
      test_diagnosis(ss.str(), temp1 == AttFrags[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
      temp2 = m_pfc.mult(publicKeyAtts[CTatts[i]], CTrand);
      test_diagnosis(ss.str(), temp2 == AttFrags[i], errors);
#endif

      ss.str("");
    }

    return errors;
  }

  int test4(int errors, KPABE& testclass){
    //------------------ Test 4: Key Generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 4");

    stringstream ss;
    bool success;
    
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

    
    ShamirAccessPolicy policy(threshold, pol_parts, order);

    ShamirSS shamir(policy, m_pfc);
    std::vector<SharePair> shares = shamir.distribute_random(privateKeyBlinder);   
    poly = shamir.getDistribRandomness();

    Big privateAtt;

    DEBUG("------------ Start GEN KEY --------------");
#ifdef AttOnG1_KeyOnG2
    vector<G2> keyFrags = testclass.genKey(policy, poly);
#endif
#ifdef AttOnG2_KeyOnG1
    vector<G1> keyFrags = testclass.genKey(policy, poly);
#endif

    DEBUG("------------ Start TEST COMPARISON --------------");

    for (int i = 0; i < policy.getNumShares(); i++){
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


  int test5(int errors, KPABE& testclass){
    //------------------ Test 5: Decryption ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 5");

    ShamirAccessPolicy policy(threshold, pol_parts, order);
    // Big& privateKeyBlinder = testclass.getPrivateKeyRand();
    // ShamirSS shamir(policy, m_pfc);
    // std::vector<SharePair> shares = shamir.distribute_random(privateKeyBlinder);   
    
    // DEBUG("[TEST5] Distributed secret (t): " << privateKeyBlinder);

#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags = testclass.genKey(policy);
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags = testclass.genKey(policy);
#endif
   
    mip->IOBASE=256;
    const Big M = (char *)"hello world to be encrypted"; 
    mip->IOBASE=16;

    // attribute indices for the ciphertext:
    vector<int> CTatts; // all valid attribute indices, authorized set
    CTatts.push_back(4);
    CTatts.push_back(2);
    CTatts.push_back(17);
    CTatts.push_back(6);
    CTatts.push_back(12);
    CTatts.push_back(5);

    vector<int> BadCTatts;  // one invalid attribute index
    BadCTatts.push_back(4);
    BadCTatts.push_back(2);
    BadCTatts.push_back(17);
    BadCTatts.push_back(6);
    BadCTatts.push_back(20);
    BadCTatts.push_back(5);

    vector<int> UnauthCTatts; // all valid attributes, unauthorized set
    UnauthCTatts.push_back(3);
    UnauthCTatts.push_back(1);
    UnauthCTatts.push_back(17);
    UnauthCTatts.push_back(9);
    UnauthCTatts.push_back(12);
    UnauthCTatts.push_back(5);
    UnauthCTatts.push_back(15);
    UnauthCTatts.push_back(11);

#ifdef AttOnG1_KeyOnG2
  vector<G1> AttFrags;
  vector<G1> BadAttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> AttFrags;
  vector<G2> BadAttFrags;
#endif
    
    Big CT;
    Big PT;

    bool success = testclass.encryptS(CTatts, M, CT, AttFrags);
    test_diagnosis("Test 5: string encryption with authorized attributes", success, errors);

    success = testclass.decryptS(policy, keyFrags, CTatts, CT, AttFrags, PT);

    mip->IOBASE=256;
    DEBUG("[TEST5] found plaintext : " << PT);
    mip->IOBASE=16;


    test_diagnosis("Test 5: string decryption with authorized attributes success", success, errors);
    test_diagnosis("Test 5: string decryption with authorized attributes equality", PT == M, errors);
    
    success = testclass.encryptS(UnauthCTatts, M, CT, BadAttFrags);
    test_diagnosis("Test 5: string encryption with unauthorized attributes", success, errors);
    
    success = testclass.decryptS(policy, keyFrags, UnauthCTatts, CT, BadAttFrags, PT);
    test_diagnosis("Test 5: string decryption with unauthorized attributes", !success, errors);


    // -----------------

    Big rand;
    m_pfc.random(rand); // picking a random message
    GT pair = m_pfc.pairing(Q,P);
    const GT GroupM = m_pfc.power(pair, rand);
    GT GroupPT; // plaintetx
    GT GroupCT; // main part of ciphertext

    DEBUG("[TEST5] plaintext to be encrypted : " << m_pfc.hash_to_aes_key(GroupM));

    success = testclass.encrypt(CTatts, GroupM, GroupCT, AttFrags);
    test_diagnosis("Test 5: algebraic encryption with authorized attributes", success, errors);

    success = testclass.decrypt(policy, keyFrags, CTatts, GroupCT, AttFrags, GroupPT);

    DEBUG("[TEST5] found plaintext : " << m_pfc.hash_to_aes_key(GroupPT));


    test_diagnosis("Test 5: algebraic decryption with authorized attributes success", success, errors);
    test_diagnosis("Test 5: algebraic decryption with authorized attributes equality", GroupPT == GroupM, errors);
    
    success = testclass.encrypt(UnauthCTatts, GroupM, GroupCT, BadAttFrags);
    test_diagnosis("Test 5: algebraic encryption with unauthorized attributes", success, errors);
    
    success = testclass.decrypt(policy, keyFrags, UnauthCTatts, GroupCT, BadAttFrags, GroupPT);
    test_diagnosis("Test 5: algebraic decryption with unauthorized attributes", !success, errors);


    return errors;
  }

  int runTests(miracl *mip){   
    int errors = 0;

    KPABE testclass(m_pfc, nattr); 
    testSetup(testclass);

    errors += test1(errors, testclass);
    errors += test2(errors, testclass);
    errors += test3(errors, testclass);
    errors += test4(errors, testclass);

    // GT old_blinder = testclass.getPublicCTBlinder();

    // DEBUG("[TEST SETUP] PrivateKeyRand (t): " << testclass.getPrivateKeyRand());
    // DEBUG("[TEST SETUP] PublicCTBlinder (e(g1,g2)^t): " << m_pfc.hash_to_aes_key(old_blinder));
    // DEBUG("[TEST SETUP] Last Encryption Randomness before encryption (s): " << testclass.getLastEncryptionRandomness());
    
    // vector<Big>& privateAtts = testclass.getPrivateAttributes();
    // vector<G2>& publicAtts = testclass.getPublicAttributes(); 
    // debugVectorBig("[TEST SETUP] Private attributes (t_i): ", privateAtts);
    // debugVectorG2("[TEST SETUP] Public attributes (t_i): ", publicAtts, privateAtts, Q);
    
    errors += test5(errors, testclass);




    // DEBUG("[TEST SETUP] old_blinder (e(g1,g2)^t): " << m_pfc.hash_to_aes_key(old_blinder));
    // DEBUG("[TEST SETUP] current blinder (e(g1,g2)^t): " << m_pfc.hash_to_aes_key(testclass.getPublicCTBlinder()));

    // GT full_blinder = m_pfc.power(testclass.getPublicCTBlinder(), testclass.getLastEncryptionRandomness());
    // DEBUG("[TEST SETUP] Full Blinder (e(g1,g2)^ts): " << m_pfc.hash_to_aes_key(full_blinder));

    return errors;
  }
  
  void debugVectorBig(string s, vector<Big> vec){    
    for (int i = 0; i < vec.size(); i++) {
      DEBUG(s << "(" << i << ") = " << vec[i]);
    }
  }
  
  void debugVectorG2(string s, vector<G2> vec, vector<Big> exps, G2 g){
    G2 temp2;
    for (int i = 0; i < vec.size(); i++) {
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

  

};

//-----------------------------------------------------------
//------------------- Main Level ----------------------------



int main() {
  //  miracl *mip = mirsys(5000,0); // C version: this is necessary to get the MIRACL functioning, which means that then I can call Bigs and so forth.
  // Miracl precision(5,0); // C++ version for the above, together with the next line
  // miracl* mip = &precision;

  // The constructor of PFC (in bn_pair.cpp) already invokes mirsys and initializes the mip pointer.
  // Because of this, I don't do that explicitly here.
  // It also sets the base to 16, but I include that here for clarity. One should not have to read the code of library classes to understand this code

  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=16;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  DEBUG("Calling first constructor");
  KPABETest tests(pfc, mip); 
  int result = tests.runTests(mip);
  print_test_result(result,tests.name());
  return 0;
}
