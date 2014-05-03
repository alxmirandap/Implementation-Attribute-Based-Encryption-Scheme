/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds a testing framework for the ShamirSS class.

  - Compile this file as

  	g++ -O2 -m64 -DZZNS=4 shamir.o -lbn -lmiracl -lpairs -o testshamir
*/


//----------------------------------------------------------------
//---------------------- ShamirTest Class ------------------------

#include "utils.h"
#include "shamir.h"

class ShamirTest 
{
  int m_nparts;  // number of participants in the scheme
  Big m_order;   // the order of the group where the polynomial is based
  int m_k;	// the threshold of the access policy
  vector<int> m_parts; // the participants names
  vector<Big> poly_rand; // the randomness I want to use for the distribution
  PFC &m_pfc;

public:
  ShamirTest(PFC &pfc):
    m_nparts(5), m_order(11), m_k(3), m_pfc(pfc)
  {
    m_parts.reserve(m_nparts);
    poly_rand.reserve(m_k-1);
  }
  
  ~ShamirTest(){
  }


  void test_diagnosis(string name, bool success, int& errors){
    string message;
    if (!success) {
      errors++;
      message = name + " has " shRED "failed" shWHITE;
    } else {
      message = name + " has " shGREEN "passed" shWHITE;
    }
    OUT(message);
  }

    int runTests(){   
    int errors = 0;
    for (int i = 0; i < m_nparts; i++) {
      m_parts.push_back(i+1);
    }

    ShamirSS testClass(m_k, m_nparts, m_order, m_pfc, m_parts);

    // ------------------- Test 1 ------------
    Big s = 10; // secret that is shared
    Big s2; // reconstructed secret
    vector<SharePair> party;
    bool success;
    poly_rand.push_back(2);
    poly_rand.push_back(3);
    std::vector<SharePair> shares = testClass.distribute_determ(s, poly_rand);  
    std::vector<SharePair> verifs(5);

    verifs[0].setValues(1,4);
    verifs[1].setValues(2,4);
    verifs[2].setValues(3,10);
    verifs[3].setValues(4,0);
    verifs[4].setValues(5,7);
    test_diagnosis("Test 1 - distribution", (shares == verifs), errors);

    party.clear();
    party.push_back(verifs[0]);
    party.push_back(verifs[1]);
    party.push_back(verifs[2]);
    s2 = testClass.reconstruct(party);
    test_diagnosis("Test 1 - reconstruction (1,2,3)", (s2 == s), errors);

    party.clear();
    party.push_back(verifs[3]);
    party.push_back(verifs[1]);
    party.push_back(verifs[4]);
    party.push_back(verifs[2]);
    s2 = testClass.reconstruct(party);
    test_diagnosis("Test 1 - reconstruction (3,1,4,2)", (s2 == s), errors);

    shares.clear();

    // ------------------- Test 2 ------------
    s = 0 ;
    poly_rand[0] = 7;
    poly_rand[1] = 4;
    shares = testClass.distribute_determ(s, poly_rand);  

    verifs[0].setValues(1,0);
    verifs[1].setValues(2,8);
    verifs[2].setValues(3,2);
    verifs[3].setValues(4,4);
    verifs[4].setValues(5,3);

    success = (shares == verifs);
    test_diagnosis("Test 2 distribution", success, errors);

    party.clear();
    party.push_back(verifs[0]);
    party.push_back(verifs[1]);
    party.push_back(verifs[2]);
    s2 = testClass.reconstruct(party);
    test_diagnosis("Test 2 - reconstruction (1,2,3)", (s2 == s), errors);

    party.clear();
    party.push_back(verifs[3]);
    party.push_back(verifs[1]);
    party.push_back(verifs[4]);
    party.push_back(verifs[2]);
    s2 = testClass.reconstruct(party);
    test_diagnosis("Test 2 - reconstruction (3,1,4,2)", (s2 == s), errors);    

    // ------------------- Test 3 ---------------------------

    int niter = 5; // number of times this test is run
    int k = 12;
    int kr = 15; // how many participants are passed to the reconstruction algorithm. Arbitrary, but larger than k
    int nparts = 20;  
    vector<int> parts(nparts);
    Big real_order = m_pfc.order();
    
    for (int i = 0; i < nparts; i++) {
      parts.push_back((7*i) % 43 + 1 ); // just so that participants are not contiguous, because there's nothing special in having participants go from 1 to n
      					// numbers 7 and 43 were chosen arbitrarily, but the modulus has to be prime to guarantee all participants different
    }

    ShamirSS testClassReal(k, nparts, real_order, m_pfc, parts);

    for (int j = 0; j < niter; j++){
      m_pfc.random(s);
      shares = testClass.distribute_random(s);   
      party.clear();
      for (int i = 0; i < kr; i++){
	party.push_back(shares[i]);
      }
      s2 = testClass.reconstruct(party);
      stringstream ss;
      ss << "Test 3." << j << " - distribution and reconstruction";
      test_diagnosis(ss.str(), (s2 == s), errors);    
    }
    return errors;
  }


  const string name() {
    return "[ShamirTest class:]";
  } 

};

//-----------------------------------------------------------
//------------------- Main Level ----------------------------

miracl *mip=get_mip();  // this is necessary to get the MIRACL functioning, which means that then I can call Bigs and so forth.
PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve

void print_test_result(int result, string name){
  if (result == 0) {
    OUT( name << "Tests " shGREEN "successful!" shWHITE);
  } else {
    OUT( name << result << " tests have " shRED "failed..." shWHITE);
  }
}

int main() {
  //DEBUG("Calling first constructor");
  ShamirTest tests(pfc); 
  int result = tests.runTests();
  print_test_result(result,tests.name());
}