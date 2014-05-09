/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds a testing framework for the KPABE class.
  Note that the KPABE class has methods that reveal all sorts of private information, like the challenge randomness used or the private keys.
  This is only for testing purposes, and not for production use

  - Compile this file as

  	g++ -O2 -m64 -DZZNS=4 testshamir.cpp shamir.o -lbn -lmiracl -lpairs -o testshamir
*/


//----------------------------------------------------------------
//---------------------- ShamirTest Class ------------------------

#include "utils.h"
#include "shamir.h"

class KPABETest 
{
  PFC &m_pfc;

public:
  template class<T> KPABETest<T>(PFC &pfc):
  {
  }
  
  ~KPABETest(){
  }


  int runTests(){   
    int errors = 0;
    for (int i = 0; i < m_nparts; i++) {
      m_parts.push_back(i+1);
    }

    KPABE testclass<ShamirSS>(m_pfc); 

  //------------------ Test 1: scheme params generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 1");

    const G1& P;	// these are declared constants because in normal use they should be constants.
    const G2& Q;	// in the test, when I need to create a new instance of the scheme I will also need new variables
    const Big& order;

    const int nattr = 20;  

    testclass.paramsgen(P, Q, order, nattr, KPABE.AttOnG2);  // the last argument specifies which group is used to build attribute fragments.
    							     // implicitly it defines which group (the other one( is used to build key fragments.

    test_diagnosis("Test 1: number of attributes", testclass.nAttr() == nattr, errors);
    test_diagnosis("Test 1: P^order == 1", pfc.mult(P,order) == 1, errors);
    test_diagnosis("Test 1: Q^order == 1", pfc.mult(Q,order) == 1, errors);


  //------------------ Test 2: scheme setup ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 2");

    testclass.setup();

    vector<Big&> privateKeyAtts = testclass.getPrivateAttributes();
    Big& privateKeyBlinder = testclass.getPrivateKeyRand();
    vector<G2&> publicKeyAtts = testclass.getPublicAttributes();
    G2& publicCTBlinder = testclass.getPublicCTBlinder();

    test_diagnosis("Test 2: attribute data structures", privateKeyAtts.size() == nattr);
    test_diagnosis("Test 2: attribute data structures", publicKeyAtts.size() == nattr);

    stringstream ss;

    for (int i = 0; i < nattr; i++){
      ss = "Test 2 - " << i << ": atts computation";
      test_diagnosis(ss.str(), pfc.modmult(Q, privateKeyAtts[i]) == publicKeyAtts[i]);
    }
    GT& pair = pfc.pairing(Q,P);
    test_diagnosis("Test 2a: blinding factor", pfc.modmult(pair, privateKeyBlinder) == publicKeyBlinder);
  
  //------------------ Test 3: Key Generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 3");


    vector<int> BadCTatts = {3,1,17,8,212,5}; // one invalid attribute index
    vector<int> CTatts = {3,1,17,8,12,5}; // one invalid attribute index
    int CTnatts = CTatts.size();
 
    vector<GT&> CT;
    vector<G2&> AttFrags;
    GT& M;
    Big& CTrand;

    m_pfc.random(M);

    bool success = testclass.encrypt(BadCTatts, M, CT, AttFrags);

    test_diagnosis("Test 3: bad attributes, return fail", !success);
    
    success = testclass.encrypt(CTatts, M, CT, AttFrags, CTrand);
    CTrand = testclass.getLastEncryptionRandomness();
    test_diagnosis("Test 3: good attributes, succeed", success);
    test_diagnosis("Test 3: CT well-formedness", pfc.power(pfc.mult(M, publicCTBlinder), CTrand) == CT);
    for (int i = 0; i < nattr; i++){
      ss = "Test 3 - " << i << ": attribute fragments well-formedness";
      test_diagnosis(ss.str(), pfc.power(publicKeyAtts[i], CTrand) == AttFrags[i]);
    }
    
  //------------------ Test 4: Key Generation ------------------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 4");
    


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
