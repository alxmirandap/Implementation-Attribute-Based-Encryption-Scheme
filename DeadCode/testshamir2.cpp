/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds a testing framework for the ShamirSS class.

  - Compile this file as

  	g++ -O2 -m64 -DZZNS=4 testshamir.cpp shamir.o -lbn -lmiracl -lpairs -o testshamir
*/


//----------------------------------------------------------------
//---------------------- ShamirTest Class ------------------------

#include "utils.h"
#include "shamir2.h"

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

  int runTests(){   
    int errors = 0;

    ShamirSS testClass(m_k, m_nparts, m_order, m_pfc);

    // ------------------- Test 1 ------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 1");

    Big s = 10; // secret that is shared
    Big s2; // reconstructed secret
    vector<ShareTuple> party;
    bool success;
    poly_rand.push_back(2);
    poly_rand.push_back(3);
    std::vector<ShareTuple> shares = testClass.distribute_determ(s, poly_rand);  
    std::vector<ShareTuple> verifs(5);

    verifs[0].setValues(1,7,5);
    verifs[1].setValues(2,0,10);
    verifs[2].setValues(3,0,15);
    verifs[3].setValues(4,7,20);
    verifs[4].setValues(5,10,25);
    test_diagnosis("Test 1 - distribution", (shares == verifs), errors);

    party.clear();
    party.push_back(verifs[0]);
    party.push_back(verifs[1]);
    party.push_back(verifs[2]);
    DEBUG("Calling reconstruct");
    s2 = testClass.reconstruct(party);
    test_diagnosis("Test 1 - reconstruction (1,2,3)", (s2 == s), errors);

    party.clear();
    party.push_back(verifs[2]);
    party.push_back(verifs[0]);
    party.push_back(verifs[3]);
    party.push_back(verifs[1]);
    s2 = testClass.reconstruct(party);
    test_diagnosis("Test 1 - reconstruction (3,1,4,2)", (s2 == s), errors);

    shares.clear();

    // ------------------- Test 2 ------------
    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 2");

    vector<int> parts;
    vector<int> shareParts;

    m_order = 43;

    parts.push_back(2);
    parts.push_back(7);
    parts.push_back(3);
    parts.push_back(5);

    shareParts.push_back(1);
    shareParts.push_back(2);
    shareParts.push_back(2);
    shareParts.push_back(3);



    ShamirSS testClass2(m_k, parts, shareParts, m_order, m_pfc);

    vector<int> returnedParts = testClass2.getParticipants();
    test_diagnosis("List of participants is wrong: ", returnedParts == parts, errors);

    for (unsigned int i = 0; i < returnedParts.size(); i++) {
      DEBUG("Returned participant: " << returnedParts[i]);
    }


    vector<int> returnedShareParts = (dynamic_cast<ShamirAccessPolicy*>(testClass2.getPolicy()))->getSharesByParticipant();
    test_diagnosis("List of participants is wrong: ", returnedShareParts == shareParts, errors);

    for (unsigned int i = 0; i < returnedShareParts.size(); i++) {
      DEBUG("Returned share for participant: " << returnedShareParts[i]);
    }


    test_diagnosis("Number of shares returned by the policy should be 8", testClass2.getNumShares(), errors);


    s = 0 ;
    poly_rand[0] = 7;
    poly_rand[1] = 4;
    shares = testClass2.distribute_determ(s, poly_rand);  

    for (unsigned int i = 0; i < shares.size(); i++) {
      DEBUG("Actually returned these shares: " << shares[i].to_string());
    }


    stringstream ss;
    ss << "Test 2 distribution should have return 8 shares";  
    DEBUG("Size of shares output " << shares.size());
    test_diagnosis(ss.str(), shares.size() == 8, errors);

    verifs = vector<ShareTuple>(8);
    verifs[0].setValues(2,18,16);
    verifs[1].setValues(7,36,56);
    verifs[2].setValues(7,22,57);
    verifs[3].setValues(3,21,24);
    verifs[4].setValues(3,9,25);
    verifs[5].setValues(5,15,40);
    verifs[6].setValues(5,2,41);
    verifs[7].setValues(5,40,42);

    for (unsigned int i = 0; i < verifs.size(); i++) {
      DEBUG("Comparison " << i << verifs[i].to_string() << " : " << shares[i].to_string());
    }

    success = (shares == verifs);
    test_diagnosis("Test 2 distribution", success, errors);

    party.clear();
    party.push_back(verifs[0]);
    party.push_back(verifs[1]);
    party.push_back(verifs[2]);
    party.push_back(verifs[3]);
    s2 = testClass2.reconstruct(party);
    test_diagnosis("Test 2 - reconstruction (1,2,3,4)", (s2 == s), errors);

    party.clear();
    party.push_back(verifs[6]);
    party.push_back(verifs[0]);
    party.push_back(verifs[7]);
    party.push_back(verifs[3]);
    party.push_back(verifs[2]);
    s2 = testClass2.reconstruct(party);
    test_diagnosis("Test 2 - reconstruction (7,1,8,4,3)", (s2 == s), errors);    

    //  ------------------- Test 3 ---------------------------

    OUT("==============================<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>==============================");
    OUT("Beginning of test 3");

    int niter = 5; // number of times this test is run
    int k = 1;
    int kr = 12; // how many participants are passed to the reconstruction algorithm. Arbitrary, but larger than k
    int nparts = 20;
    parts = vector<int>(nparts);
    Big real_order = m_pfc.order();
    Big old_s=0;
    
    for (int i = 0; i < nparts; i++) {
      parts[i]=((7*i) % 43 + 1 ); // just so that participants are not contiguous, because there's nothing special in having participants go from 1 to n
      					// numbers 7 and 43 were chosen arbitrarily, but the modulus has to be prime to guarantee all participants different
    }

    ShamirSS testClassReal(k, parts, real_order, m_pfc);
    //DEBUG("Echo in run tests: reached the end of constructor");

    for (int j = 0; j < niter; j++){
      DEBUG("Test 3 run: " << j);
      m_pfc.random(s);
      s = s % real_order;
    
      DEBUG("s: " << s << "\t old s: " << old_s);
      guard("s should be random, and different from the last value or 0", s != old_s); // the probability that s is 0 or the old value should be negligible
      //DEBUG("random ok");
      shares = testClassReal.distribute_random(s);   
      DEBUG("shares size: " << shares.size() << "\t nparts = " << nparts);
      guard("The right amout of shares was not given: ", shares.size() == nparts);
      DEBUG("Distribution ok");
      party.clear();
      DEBUG("Party cleared...");
      DEBUG("shares size: " << shares.size() << "\t kr = " << kr);
      guard("There are not enough shares in shares: ", shares.size() >= kr);
      for (int i = 0; i < kr; i++){
	party.push_back(shares[i]);
      }
      guard("Party does not have the right number of shares", party.size() == kr);
      DEBUG("new party ok");
      s2 = testClassReal.reconstruct(party);
      DEBUG("reconstruction ok");
      stringstream ss;
      DEBUG("s: " << s << "\t s2: " << s2);
      ss << "Test 3." << j << " - distribution and reconstruction";
      test_diagnosis(ss.str(), (s2 == s), errors);    
      DEBUG("Diagnosis ok");
      old_s = s;
    }

    // ------------------- Test 4 ---------------------------
    // policy where the shares have different participants


    OUT("Beginning of test 4");

    int thrflex = 11;
    int nsharesflex = 20;
    unsigned int npartsflex = 10;

    vector<int> partsflex(npartsflex);
    old_s=0;


    DEBUG("Test 4. Mark Points 1");
    
    for (unsigned int i = 0; i < npartsflex; i++) {
      partsflex[i]=((7*i) % 43 + 1 ); // just so that participants are not contiguous, because there's nothing special in having participants go from 1 to n
      					// numbers 7 and 43 were chosen arbitrarily, but the modulus has to be prime to guarantee all participants different
    }

    shareParts = vector<int>(npartsflex); // another choice of formula that gives an unbalanced choice of indices from 0 to parts2.size()-1. 
    			         // This is to represent a policy
    				// where some participants are more important than others

    
    
    // this generates the following shares per participant's index. Remember threshold=11
    // {2, 4, 4, 2, 1, 2, 2, 2, 1, 0}
    // Attempts to reconstruct by participant's index:
    // {0,1,2,3,4}: success (13 shares) 
    // {3,4,5,6,7,8,9}: failure (10 shares)

    DEBUG("Test 4. Mark Points 2");
    
    int nsharesPPart[] = {2, 4, 4, 2, 1, 2, 2, 2, 1, 0};

    for (unsigned int i = 0; i < npartsflex; i++) {
      shareParts[i] = nsharesPPart[i];
    }

    for (int i = 0; i < shareParts.size(); i++){
      DEBUG("Contents of shareParts: " << shareParts[i]);
    }

    DEBUG("Test 4. Mark Points 3");
    ShamirSS testClassFlex(thrflex, partsflex, shareParts, real_order, m_pfc);

    DEBUG("Test 4. Mark Points 4");
    vector<int> goodSet;
    for (int i = 0; i <= 4; i++){
      goodSet.push_back(partsflex[i]);
    }
    
    DEBUG("Test 4. Mark Points 5");
    vector<int> badSet;
    for (int i = 3; i <= 9; i++){
      badSet.push_back(partsflex[i]);
    }

    DEBUG("GoodSet Size: " << goodSet.size());
    DEBUG("BadSet Size: " << badSet.size());

    for (int i = 0; i < goodSet.size(); i++){
      DEBUG("contents of goodSet: " << goodSet[i]);
    }
    for (int i = 0; i < badSet.size(); i++){
      DEBUG("contents of badSet: " << badSet[i]);
    }

    DEBUG("Test 4. Mark Points 6");

    int goodSharesSize = 13;
    int badSharesSize = 10;

    niter = 1;

    for (int j = 0; j < niter; j++){
      DEBUG("Test 4 run: " << j);
      m_pfc.random(s);
      s = s % real_order;
    
      DEBUG("s: " << s << "\t old s: " << old_s);
      guard("s should be random, and different from the last value or 0", s != old_s); // the probability that s is 0 or the old value should be negligible
      //DEBUG("random ok");
      shares = testClassFlex.distribute_random(s);   
      DEBUG("shares size: " << shares.size() << "\t nparts = " << nsharesflex);
      guard("The right amout of shares was not given: ", shares.size() == nsharesflex);
      DEBUG("Distribution ok");
      party.clear();
      DEBUG("Party cleared...");
      DEBUG("shares size: " << shares.size() << "\t kr = " << npartsflex);
      guard("There are not enough shares in shares: ", shares.size() >= npartsflex);

      DEBUG("Finding GoodShares");
      vector<ShareTuple> goodShares = testClassFlex.getSharesForParticipants(goodSet, shares);

      DEBUG("Finding BadShares");
      vector<ShareTuple> badShares = testClassFlex.getSharesForParticipants(badSet,  shares);

      stringstream ss;

      DEBUG("GoodShares size: " << goodShares.size());
      DEBUG("BadShares size: " << badShares.size());
      ss << "Test 4." << j << " - goodShares does not have the right number of shares";
      test_diagnosis(ss.str(), goodShares.size() == goodSharesSize, errors);    

      ss.str("");
      ss << "Test 4." << j << " - badShares does not have the right number of shares";
      test_diagnosis(ss.str(), badShares.size() == badSharesSize, errors);    

      vector<ShareTuple> goodWitness;
      vector<ShareTuple> badWitness;    

      ss.str("");
      ss << "Test 4." << j << " - goodShares was not correcty Evaluated";
      test_diagnosis(ss.str(), testClassFlex.evaluate(goodShares, goodWitness), errors);


      ss.str("");
      ss << "Test 4." << j << " - badShares was not correcty Evaluated";
      test_diagnosis(ss.str(), !testClassFlex.evaluate(badShares, badWitness), errors);

      ss.str("");
      ss << "Test 4." << j << " - goodWitness should have as many shares as the threshold";
      test_diagnosis(ss.str(), goodWitness.size() == thrflex, errors);

      ss.str("");
      ss << "Test 4." << j << " - badWitness should have no shares";
      test_diagnosis(ss.str(), badWitness.size() == 0, errors);

      DEBUG("new party ok");
      s2 = testClassFlex.reconstruct(goodWitness); // previously it was goodShares but witness seems to make more sense
      DEBUG("reconstruction ok");

      ss.str("");
      DEBUG("s: " << s << "\t s2: " << s2);
      ss.str("");
      ss << "Test 4." << j << " - distribution and reconstruction";
      test_diagnosis(ss.str(), (s2 == s), errors);    
      DEBUG("Diagnosis ok");
      old_s = s;
    }


    return errors;
  }


  const string name() {
    return "[ShamirTest class:]";
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

  mip->IOBASE=10;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  DEBUG("Calling first constructor");
  ShamirTest tests(pfc); 
  int result = tests.runTests();
  print_test_result(result,tests.name());
  return 0;
}
