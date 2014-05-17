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

public:
  
  KPABETest(PFC &pfc):
    m_pfc(pfc)
  {}
  ~KPABETest()
  {}


  int runTests(){   
    int errors = 0;

    const int nattr = 20;     
    KPABE testclass(m_pfc, nattr); 

  //------------------ Test 1: scheme params generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 1");

    G1 P;	
    G2 Q;	
    Big order;

    testclass.paramsgen(P, Q, order);     				

    G1 temp1 = m_pfc.mult(P,order+1);
    G2 temp2 = m_pfc.mult(Q,order+1);
    test_diagnosis("Test 1: number of attributes", testclass.numberAttr() == nattr, errors);
    test_diagnosis("Test 1: P^order == 1", temp1 == P, errors);
    test_diagnosis("Test 1: Q^order == 1", temp2 == Q, errors);


  //------------------ Test 2: scheme setup ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 2");

    testclass.setup();

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
      temp2 = m_pfc.mult(P, privateKeyAtts[i]);
#endif
#ifdef AttOnG2_KeyOnG1
      temp2 = m_pfc.mult(Q, privateKeyAtts[i]);
#endif

      test_diagnosis(ss.str(), temp2 == publicKeyAtts[i], errors);
      ss.str("");
    }
    GT pair = m_pfc.pairing(Q,P);
    test_diagnosis("Test 2a: blinding factor", m_pfc.power(pair, privateKeyBlinder) == publicCTBlinder, errors);
  
  //------------------ Test 3: Encryption ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 3");

    vector<int> BadCTatts;  // one invalid attribute index
    BadCTatts.push_back(3);
    BadCTatts.push_back(1);
    BadCTatts.push_back(17);
    BadCTatts.push_back(8);
    BadCTatts.push_back(20);
    BadCTatts.push_back(5);

    vector<int> CTatts; // all valid attribute indices
    CTatts.push_back(3);
    CTatts.push_back(1);
    CTatts.push_back(17);
    CTatts.push_back(8);
    CTatts.push_back(12);
    CTatts.push_back(5);

    int CTnatts = CTatts.size();
 
    GT CT;

#ifdef AttOnG1_KeyOnG2
    vector<G1> AttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
    vector<G2> AttFrags;
#endif


    Big rand;
    Big CTrand;
    GT aux;

    m_pfc.random(rand); // picking a random message
    const GT M = m_pfc.power(pair, rand);

    bool success = testclass.encrypt(BadCTatts, M, CT, AttFrags);

    test_diagnosis("Test 3: bad attributes, return fail", !success, errors);
    
    success = testclass.encrypt(CTatts, M, CT, AttFrags);
    test_diagnosis("Test 3: size of attribute frags", AttFrags.size() == CTatts.size(), errors);
    CTrand = testclass.getLastEncryptionRandomness();
    test_diagnosis("Test 3: good attributes, succeed", success, errors);
    aux = m_pfc.power(publicCTBlinder, CTrand);
    test_diagnosis("Test 3: CT well-formedness", (M * aux) == CT, errors);

    for (int i = 0; i < CTatts.size(); i++){
      ss << "Test 3 - " << i << ": attribute fragments well-formedness";
      temp2 = m_pfc.mult(publicKeyAtts[CTatts[i]], CTrand);
      test_diagnosis(ss.str(), temp2 == AttFrags[i], errors);
      ss.str("");
    }
    
    //------------------ Test 4: Key Generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 4");
    
    vector<int> pol_parts;
    for (int i = 1; i <= 8; i++){
      pol_parts.push_back(i);
    }

    int threshold = 4;
    ShamirAccessPolicy policy(threshold, pol_parts);

    ShamirSS shamir(policy, order, m_pfc);
    std::vector<SharePair> shares = shamir.distribute_random(privateKeyBlinder);   
    vector<Big>& poly = shamir.getDistribRandomness();

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
      test_diagnosis(ss.str(), m_pfc.mult(Q, moddiv(shares[i].getShare(),privateAtt,order)) == keyFrags[i], errors);
#endif
#ifdef AttOnG2_KeyOnG1
      test_diagnosis(ss.str(), m_pfc.mult(P, moddiv(shares[i].getShare(),privateAtt,order)) == keyFrags[i], errors);
#endif
    DEBUG("Iter: " << i << " Share: " << shares[i].getShare() 
	<< " Attribute: " << privateAtt);
      
      ss.str("");
    }

    //------------------ Test 5: Decryption ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 5");
    
    vector<int> UnauthCTatts;  
    BadCTatts.push_back(3);
    BadCTatts.push_back(1);
    BadCTatts.push_back(17);
    BadCTatts.push_back(9);
    BadCTatts.push_back(12);
    BadCTatts.push_back(5);
    BadCTatts.push_back(15);
    BadCTatts.push_back(11);

    success = testclass.encrypt(UnauthCTatts, M, CT, AttFrags);
    test_diagnosis("Test 5: encryption with unauthorized attributes", success, errors);
    
    GT PT; // plaintetx
    PT = testclass.decrypt(policy, keyFrags, UnauthCTatts, CT, AttFrags);
    test_diagnosis("Test 5: decryption with unauthorized attributes", PT != M, errors);

    success = testclass.encrypt(CTatts, M, CT, AttFrags);
    test_diagnosis("Test 5: encryption with authorized attributes", success, errors);
    
    PT = testclass.decrypt(policy, keyFrags, CTatts, CT, AttFrags);
    test_diagnosis("Test 5: decryption with authorized attributes", PT == M, errors);

    return errors;
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
  KPABETest tests(pfc); 
  int result = tests.runTests();
  print_test_result(result,tests.name());
  return 0;
}
