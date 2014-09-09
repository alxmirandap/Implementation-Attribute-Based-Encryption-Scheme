/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file implements some basic time measurements, covering all the operations needed for the KP implementation

  The group structure has two groups G1 and G2 and one Pairing Group GT
  There are two distinct ways to use this: either G1 will hold the key fragments or it will hold the key fragments. That is determined by the definition of constants 
  #define AttOnG1_KeyOnG2 or #define AttOnG2_KeyOnG1 in kpabe.h.

  My tests with measuring time by counting clicks have shown that this
  is not reliable. They usually give me half of the actual time
  passed. Because of this, I will adopt the method of repeating the
  experiment a large number of times and measuring the seconds via the
  time function. This has the added benefit of smoothing out
  variations in individual executions.

  The aim of this section is to perform efficiency measurements of ready made encryption schemes.
  
  I should collect measurements for the following variants:
  - encryption based on algebraic multiplication // encryption based on XOR with hash
  - Key on G1 // Key on G2
  
  Then, I must have variants for the number of attributes in the universe, in the ciphertext, and particular policies

*/

#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#include "benchmark_defs.h"

#ifndef DEF_KPABE
#include "kpabe.h"
#endif



void report_time(const string& description, const string& parameter, long repeats, long nparams, time_t initial, time_t final) {
  double elapsed = final - initial;
  cout << "--------------------------------------------------------------" << endl;
  cout << "Task: " << description << endl;
  cout << "Number of repetitions: " << repeats << endl;
  cout << "Number of " << parameter << ": " << nparams << endl;
  cout << "Total time in seconds: " << elapsed << endl;

  double average;
  string unit;
  double magnitude;

  magnitude = 1000000;
  if (repeats >= magnitude){
    unit = "microseconds";
    average = elapsed / (repeats / magnitude);
  } else {
    magnitude = 1000;
    if (repeats >= magnitude){
      unit = "milliseconds";
      average = elapsed / (repeats / magnitude);
    } else {
      unit = "seconds";
      average = elapsed / repeats;
    }
  }
  cout << "Average execution time per repetition: " << average << " " << unit << endl;


  long repcount = repeats * nparams;
  magnitude = 1000000;
  if (repcount >= magnitude){
    unit = "microseconds";
    average = elapsed / (repcount / magnitude);
  } else {
    magnitude = 1000;
    if (repcount >= magnitude){
      unit = "milliseconds";
      average = elapsed / (repcount / magnitude);
    } else {
      unit = "seconds";
      average = elapsed / repcount;
    }
  }
  cout << "Average execution time per " << parameter << ": " << average << " " << unit << endl;
  cout << "--------------------------------------------------------------" << endl;
}

void get_time(time_t *t) {
  time(t);
}

void report_action(const string& action, time_t *t){
  get_time(t);

//   time_t now = time(t);
//   char* dt = ctime(&now);
//   cout << action << dt; 
}

void report_start(time_t *t){
  report_action("Starting measurement at: ", t);
}

void report_finish(time_t *t){
  report_action("Ending measurement at: ", t);
}

bool Setup = false;
bool Encrp = false;
bool KeyUnif = false;
bool KeyLin = false;
bool KeyExp = false;
bool KeyLinInv = false; 
bool KeyExpInv = false; // the Inv versions put the largest sets at the front, instead of at the end
bool DecUnif = false;
bool DecLin = false;
bool DecExp = false;
bool DecLinInv = false; 
bool DecExpInv = false; // the Inv versions put the largest sets at the front, instead of at the end
 
  
void parseInput(int argc, char* argv[]){
  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "st") Setup = true;
    if (arg == "en") Encrp = true;
    if (arg == "ku") KeyUnif = true;
    if (arg == "kl") KeyLin = true;
    if (arg == "kx") KeyExp = true;
    if (arg == "kli") KeyLinInv = true;
    if (arg == "kxi") KeyExpInv = true;
    if (arg == "du") DecUnif = true;
    if (arg == "dl") DecLin = true;
    if (arg == "dx") DecExp = true;
    if (arg == "dli") DecLinInv = true;
    if (arg == "dxi") DecExpInv = true;
    if (arg == "all") {
      Setup = Encrp = KeyUnif = KeyLin = KeyExp = KeyLinInv = KeyExpInv = DecUnif = DecLin = DecExp = DecLinInv = DecExpInv = true;
    }
  }
}



int million = 1000000;
int thousand = 1000;
int hundred = 100;

void  report_title(std::string title, std::string params ) {
  cout << "Task: " << title << std::endl << params << std::endl;
}

void report_setup_data(int nAtts, long repeats, time_t initial, time_t final) {
  double elapsed = final - initial;
  cout << nAtts << "\t" << repeats << "\t" << elapsed << std::endl;
}

void report_encr_data(int nAtts, long repeats, time_t a_initial, time_t a_final, time_t h_initial, time_t h_final) {
  double a_elapsed = a_final - a_initial;
  double h_elapsed = h_final - h_initial;
  cout << nAtts << "\t" << repeats << "\t" << a_elapsed << "\t" << h_elapsed << std::endl;
}

void report_key_data(int max, int k, long repeats, long nLeaves, long nSets, time_t initial, time_t final, std::string policy) {
  double elapsed = final - initial;
  cout << nLeaves << "\t" << nSets << "\t" << max << "\t" << k << "\t" << repeats << "\t" << elapsed << "\t" << policy << std::endl;
}

void report_dec_data(int max, int k, long repeats, long nLeaves, long nSets, long nAtts, long minSetSize, time_t a_initial, time_t a_final, 
		     time_t h_initial, time_t h_final, std::string policy) {
  double a_elapsed = a_final - a_initial;
  double h_elapsed = h_final - h_initial;
  cout << nLeaves << "\t" << nSets << "\t" << nAtts << "\t" << minSetSize << "\t" << max << "\t" << k << "\t" << repeats << "\t" << a_elapsed << "\t" << h_elapsed << "\t" << policy << std::endl;
}

void measureSetup(PFC &pfc) {
  Big order; // can not be initialized at global scope because that will require work from the mip, that is still not initialized
  G1 P;
  G2 Q;

  std::string expr = op_OR + "(1,2,3)";
  shared_ptr<SS_ACC_POL_TYPE> policy = make_shared<SS_ACC_POL_TYPE>(expr, 3);
  shared_ptr<SS_TYPE> testScheme = make_shared<SS_TYPE>(policy, pfc);

  int attrsInUniverse[] = {5, 10, 100, 500, 1000};
  const int attInUnivVars = 5; 
  //  int varRepeats[] = {1000,1000,500,200,100};
  int varRepeats[] = {1,1,1,1,1};

  long repeats;

  stringstream ss;
  ss << "#Atts in Universe" << "\t" << "Repetitions" << "\t" << "Total time" << std::endl;
  report_title("#1 Setup times: ", ss.str());

  time_t t0;
  time_t t1;

  for (int i = 0; i < attInUnivVars; i++){
    int nAtts = attrsInUniverse[i];
    repeats = varRepeats[i];
    KPABE testClass(testScheme, pfc, nAtts);    
    testClass.paramsgen(P, Q, order);

    report_start(&t0);
    for (int j = 0; j < repeats; j++) {
      testClass.setup();
    }
    report_finish(&t1);
    stringstream ss;
    //    ss << "#1 Setup times: " << std::endl << "Number of attributes in universe: " << nAtts;
    //    report_time( ss.str(), "Attribute", repeats, nAtts, t0, t1);
    report_setup_data(nAtts, repeats, t0, t1);
  }
}

void measureEncrp(PFC &pfc, miracl* mip) {
  Big order; // can not be initialized at global scope because that will require work from the mip, that is still not initialized
  G1 P;
  G2 Q;

  std::string expr = op_OR + "(1,2,3)";
  shared_ptr<SS_ACC_POL_TYPE> policy = make_shared<SS_ACC_POL_TYPE>(expr, 3);
  shared_ptr<SS_TYPE> testScheme = make_shared<SS_TYPE>(policy, pfc);

  int attrsInUniverse = 1000;


  int attrsInCT[] = {5, 10, 100, 500, 1000};
  const int attInCTVars = 5; 
  //  int varRepeats[] = {20 * hundred, 20 * hundred, 10 * hundred, 2 * hundred, 1 * hundred};
  //  int varRepeats[] = {2500,2500,1000,500,250};
  int varRepeats[] = {1,1,1,1,1};
  

  vector<int> CTAtts;
#ifdef AttOnG1_KeyOnG2
  vector<G1> AttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> AttFrags;
#endif

  GT CT;
  Big sCT;

  KPABE testClass(testScheme, pfc, attrsInUniverse);
  testClass.paramsgen(P, Q, order);
  testClass.setup();
  Big rand;
  GT pair = pfc.pairing(Q,P);
  pfc.random(rand); // picking a random message
  const GT M = pfc.power(pair, rand);
  mip->IOBASE=256;
  const Big sM = (char *)"hello world to be encrypted"; 
  mip->IOBASE=16;

  // OUT("Message initialized");

  long repeats;

time_t a_t0;
time_t a_t1;
time_t h_t0;
time_t h_t1;


  stringstream ss;
  ss << "#Atts in Ciphertext" << "\t" << "Repetitions" << "\t" << "Total time (Algebraic)" << "\t" << "Total time (Hash)" << std::endl;
  report_title("#2 Encryption times: ", ss.str());

  for (int i = 0; i < attInCTVars; i++){
    int nAtts = attrsInCT[i];
    repeats = varRepeats[i];

    // OUT("New repetition: setting attributes");

    CTAtts.clear();
    for (int k = 0; k < nAtts; k++) {
      CTAtts.push_back(k);
    }

    // OUT("Calling report");
    report_start(&a_t0);
    for (int j = 0; j < repeats; j++) {
      testClass.encrypt(CTAtts, M, CT, AttFrags);
    }
    report_finish(&a_t1);
    //     stringstream ss;
    //     ss << "#2 Encryption times (Algebraic): " << "Number of attributes in ciphertext: " << nAtts;
    //     report_time( ss.str(), "Attribute", repeats, nAtts, t0, t1);

    report_start(&h_t0);
    for (int j = 0; j < repeats; j++) {
      testClass.encryptS(CTAtts, sM, sCT, AttFrags);
    }
    report_finish(&h_t1);
    //    ss.str("");
    //    ss << "#2 Encryption times (Hash): " << "Number of attributes in ciphertext: " << nAtts;
    //    report_time( ss.str(), "Attribute", repeats, nAtts, t0, t1);
    report_encr_data(nAtts, repeats, a_t0, a_t1, h_t0, h_t1);
  }
}

std::string makeMinimalSet(int first, int codedLength) {
  guard("makeMinimalSet called with coded codedLength equal to 0", codedLength != 0);
  int length = abs(codedLength);
  int sign = codedLength / length;
  std::string expr = op_AND + "(";
  for (int i = 0; i < length; i++) {
    expr = expr + convertIntToStr(first + i * sign);
    if (i < length - 1) {
      expr = expr + ",";
    } else {
      expr = expr + ")";
    }
  }
  return expr;
}

std::string makeKeyUnifPolicy(int nLeaves, int nSets, int &returnLeaves, int &returnSets){
  ENHDEBUG("MakeKeyUnif Policy");
  DEBUG("nLeaves: " << nLeaves << " - nSets: " << nSets << " - returnLeaves: " << returnLeaves << " - returnSets: " << returnSets);
  returnSets = 0;
  std::string expr = op_OR + "(";
  int leavesPerSet = nLeaves / nSets;

  for (int i = 0; i < nSets; i++) {
    DEBUG("loop: " << i);
    int begin = i * leavesPerSet;
    std::string set = makeMinimalSet(begin, leavesPerSet);
    expr = expr + set;
    expr = expr + ",";
    returnSets++;
  }
  DEBUG("wrapping up");
  expr = expr.substr(0, expr.length()-1);
  expr = expr + ")";
  returnLeaves = leavesPerSet * nSets;
  return expr;
}

std::string makeKeyLinPolicy(int nLeaves, int increment, int &returnLeaves, int &returnSets){
  returnSets = 0;
  std::string expr = op_OR + "(";
  int leavesPerSet = 1;
  int totalLeaves = 0;

  for (int i = 0; totalLeaves + leavesPerSet <= nLeaves; i++) {   
    int begin = totalLeaves;
    std::string set = makeMinimalSet(begin, leavesPerSet);
    totalLeaves += leavesPerSet;
    leavesPerSet += increment;
    expr = expr + set;
    expr = expr + ",";
    returnSets++;
  }
  expr = expr.substr(0, expr.length()-1);
  expr = expr + ")";
  returnLeaves = totalLeaves;
  return expr;
}

std::string makeKeyLinPolicyInv(int nLeaves, int increment, int &returnLeaves, int &returnSets){
  returnSets = 0;
  std::string expr = ")";
  int leavesPerSet = 1;
  int remainingLeaves = nLeaves;

  for (int i = 0; remainingLeaves - leavesPerSet >= 0; i++) {   
    int begin = remainingLeaves-1;
    std::string set = makeMinimalSet(begin, -leavesPerSet);
    remainingLeaves -= leavesPerSet;
    leavesPerSet += increment;
    expr = set + expr;
    expr = "," + expr;
    returnSets++;
  }
  expr = expr.substr(1, expr.length()-1);
  expr = op_OR + "(" + expr;
  returnLeaves = nLeaves - remainingLeaves;
  return expr;
}

std::string makeKeyExpPolicy(int nLeaves, int increment, int &returnLeaves, int &returnSets){
  returnSets = 0;
  std::string expr = op_OR + "(";
  int leavesPerSet = 1;
  int totalLeaves = 0;

  for (int i = 0; totalLeaves + leavesPerSet <= nLeaves; i++) {   
    int begin = totalLeaves;
    std::string set = makeMinimalSet(begin, leavesPerSet);
    totalLeaves += leavesPerSet;
    leavesPerSet *= increment;
    expr = expr + set;
    expr = expr + ",";
    returnSets++;
  }
  expr = expr.substr(0, expr.length()-1);
  expr = expr + ")";
  returnLeaves = totalLeaves;
  return expr;
}

std::string makeKeyExpPolicyInv(int nLeaves, int increment, int &returnLeaves, int &returnSets){
  returnSets = 0;
  std::string expr = ")";
  int leavesPerSet = 1;
  int remainingLeaves = nLeaves;

  for (int i = 0; remainingLeaves - leavesPerSet >= 0; i++) {   
    int begin = remainingLeaves-1;
    std::string set = makeMinimalSet(begin, -leavesPerSet);
    remainingLeaves -= leavesPerSet;
    leavesPerSet *= increment;
    expr = set + expr;
    expr = "," + expr;
    returnSets++;
  }
  expr = expr.substr(1, expr.length()-1);
  expr = op_OR + "(" + expr;
  returnLeaves = nLeaves - remainingLeaves;
  return expr;
}

int nextUnifPol(int k) {
  return 8*k;
}

bool stopUnifPol(int k, int nLeaves) {
  return k <= nLeaves;
}

int middleSetUnifPol(int nLeaves, int nSets, int increment, int &begin){
  int middleSet = nSets / 2;
  int setSize = nLeaves / nSets;
  begin = setSize * middleSet;
  return setSize;
}

int nextLinPol(int k) {
  return k+3;
}

bool stopLinPol(int k, int nLeaves) {
  return k < 10;
}

int middleSetLinPol(int nLeaves, int nSets, int increment, int &begin){
  int middleSet = nSets / 2;
  int setSize = 1 + middleSet * increment;
  begin = increment * (middleSet - 1) * middleSet / 2 + middleSet;
  return setSize;
}

int middleSetLinPolInv(int nLeaves, int nSets, int increment, int &begin){
  int middleSet = nSets / 2;
  int setSize = 1 + (nSets - middleSet) * increment;
  begin = (middleSet - 1) * (1 + increment - increment * middleSet/2);
  return setSize;
}

int nextExpPol(int k) {
  return k+1;
}

bool stopExpPol(int k, int nLeaves) {
  return k < 3;
}

int middleSetExpPol(int nLeaves, int nSets, int increment, int &begin){
  int middleSet = nSets / 2;
  int setSize = std::pow(2, middleSet);
  begin = setSize - 1;
  return setSize;
}

int middleSetExpPolInv(int nLeaves, int nSets, int increment, int &begin){
  int middleSet = nSets / 2;
  int setSize = std::pow(2, nSets - middleSet);
  begin = std::pow(2, nSets) - std::pow(2, nSets - middleSet);
  return setSize;
}


void measureKeyFunc(PFC &pfc, std::string (*makePolicy) (int, int, int&, int&), int leavesInPolicy[], const int nLeavesVars, int varRepeats[], 
		    int (*next) (int), bool (*stop) (int, int), int start_k, std::string policy_type, std::string k_meaning ) {
  Big order; // can not be initialized at global scope because that will require work from the mip, that is still not initialized
  G1 P;
  G2 Q;

  long repeats;
  int realNLeaves;
  int realNSets;

  time_t t0;
  time_t t1;

  stringstream ss;
  ss << "#Leaves in Policy" << "\t" << "#Minimal Sets" << "\t" << "Max Leaves" << "\t" << "Control Param" << "\t" << "Repetitions" << "\t" << "Total time" << "\t" << "Policy" << std::endl;
  report_title("#3 Key Generation with " + policy_type + " policy:", ss.str());


  for (int i = 0; i < nLeavesVars; i++){
    int nLeaves = leavesInPolicy[i];
    ENHDEBUG("For Loop: leaves variants: " << nLeaves);

    for (int k = start_k; stop(k, nLeaves); k = next(k)) { 
      // for the uniform policy, k is the number of minimal sets
      // for a linear policy, it is the increment in the size of successive minimal sets
      // for an exponential policy, there should be only one case anyway, where the size of sets doubles each turn
      //    OUT("Number of Leaves: " << nLeaves << " -- Number of sets: " << k);

      ENHDEBUG("For Loop: policy: " << k);
      std::string expr = makePolicy(nLeaves, k, realNLeaves, realNSets);      
      //      ENHDEBUG("Current policy: " << expr);
      DEBUG("created expression");

      shared_ptr<SS_ACC_POL_TYPE> policy = make_shared<SS_ACC_POL_TYPE>(expr, realNLeaves);
      DEBUG("created policy");
      shared_ptr<SS_TYPE> testScheme = make_shared<SS_TYPE>(policy, pfc);
      DEBUG("created SS scheme");

      KPABE testClass(testScheme, pfc, nLeaves);    
      DEBUG("created ABE scheme");
      testClass.paramsgen(P, Q, order);
      DEBUG("generated params");
      // OUT("Setting up scheme");
      testClass.setup();
      DEBUG("finished setup");
      // OUT("Scheme set up");

      repeats = varRepeats[i];
      
      report_start(&t0);
      DEBUG("Start time: " << t0);
      for (int j = 0; j < repeats; j++) {
	DEBUG("running repeat: " << j);
	testClass.genKey();
      }
      report_finish(&t1);
      //      stringstream ss;
      //      ss << "#3 Key Generation with " << policy_type << " policy:\n" << "Number of leaves in policy: " << realNLeaves << " --- Number of minimal sets: " << realNSets;
      //      if (k_meaning != "") {
      //	ss << " --- Number of " << k_meaning << ": " << k;
      //      }
      //      report_time( ss.str(), "Leaf", repeats, realNLeaves, t0, t1);
      report_key_data(nLeaves, k, repeats, realNLeaves, realNSets, t0, t1, expr);
    }
    std::cout << std::endl;
  }
}
 

void measureDecFunc(PFC &pfc, miracl* mip, std::string (*makePolicy) (int, int, int&, int&), 
		    int leavesInPolicy[], const int lvsInPol, int varRepeats[], 
		    int (*next) (int), bool (*stop) (int, int), int (*findMiddleSetSizeAndFirstElement) (int, int, int, int&),
		    int start_k, std::string policy_type, std::string k_meaning ) {

  // we want to measure the efficiency of decryption
  // we can have different policies but also different attributes in the CT
  // there are too many variables here. How to work?

  // the most comprehensive way, in comparison to what has been done, is to create all different keys as before, but also all different attributes in the CT
  // this needs be done only once for each variant. It must be also guaranteed that we have always enough attributes to make the decryption possible

  // I can indeed create the policies as for key generation. Then, for each case, I will create a CT with enough attributes to succeed.
  // I will, however, create different sets of CT attributes. 

  // First I have to compute the size of the middle minimal set, and what are the limiting attributes in there.
  // Then, I have to compute the set of attributes I want to create. This can grow equally from the middle of that minimal set until reaching the sizes I want.

  // For key generation, I simply defined the attributes in the universe to be simply the same as the leaves of the policies.
  // This was ok because I only wanted to test generation of policies and I did not need any more attributes
  // For decryption, I want to have mismatching sets of attributes, and so I want more attributes in the universe than in the policies
  // However, I can possibly limit this at the extreme case where I will have 1024 leaves: this will be a single case, and in all others I can have more attributes than needed.

  // For the policy variants specified, we have the possible values of leaves per set: 1, 2, 4, 8, 16, 32, 64, 128, 512, 1024
  // I can adapt the number of attributes in the CT from before to the following useful numbers: 10, 50, 150, 600, 1024


  Big order; // can not be initialized at global scope because that will require work from the mip, that is still not initialized
  G1 P;
  G2 Q;

  int attrsInUniverse = 1024;

  int attrsInCT[] = {10, 50, 150, 600, 1024};
  const int attInCTVars = 5; 

  vector<int> CTAtts;
#ifdef AttOnG1_KeyOnG2
  vector<G1> AttFrags;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> AttFrags;
#endif

  GT CT;
  Big sCT;


  Big rand;
  GT pair = pfc.pairing(Q,P);
  pfc.random(rand); // picking a random message
  const GT M = pfc.power(pair, rand);
  mip->IOBASE=256;
  const Big sM = (char *)"hello world to be encrypted"; 
  mip->IOBASE=16;

  GT PT;
  Big sPT;

  long repeats;
  int realNLeaves;
  int realNSets;

  time_t a_t0;
  time_t a_t1;
  time_t h_t0;
  time_t h_t1;

  stringstream ss;
  ss << "#Leaves in Policy" << "\t" << "#Minimal Sets" << "\t" << "#Atts in Ciphertext" << "\t" 
     << "minSetSize" << "\t" << "Max Leaves" << "\t" << "Control Params" << "\t" << "Repetitions" << "\t" << "Total time (Alg)" << "\t" << "Total time (Hash)" << "\t" << "Policy" << std::endl;

  report_title("#4 Decryption with " + policy_type + " policy:", ss.str());


  for (int i = 0; i < lvsInPol; i++){
    int nLeaves = leavesInPolicy[i];

    if (varRepeats[i] == 0) {
      OUT("Skipping");
      continue; // for callibrating purposes
    }

    for (int k = start_k; stop(k, nLeaves); k = next(k) ) { 
      std::string expr = makePolicy(nLeaves, k, realNLeaves, realNSets);      
      //      ENHDEBUG("Current policy: " << expr);
      shared_ptr<SS_ACC_POL_TYPE> policy = make_shared<SS_ACC_POL_TYPE>(expr, realNLeaves);
      shared_ptr<SS_TYPE> testScheme = make_shared<SS_TYPE>(policy, pfc);

      KPABE testClass(testScheme, pfc, attrsInUniverse);    
      testClass.paramsgen(P, Q, order);
      // OUT("Setting up scheme");
      testClass.setup();
      // OUT("Scheme set up");

#ifdef AttOnG1_KeyOnG2
      vector<G2> keyFrags = testClass.genKey();
#endif
#ifdef AttOnG2_KeyOnG1
      vector<G1> keyFrags = testClass.genKey();
#endif

      // Uniform policy:
      // except when there is only one minimal set, the number of minimal sets is always even
      // I prefer to have the useful attributes in the middle, rather than in the begining of the tree. 
      // Therefore, I begin by finding the beginning of middle minimal set and start counting attributes from there.

      // Linear policy:
      // I want to find a point at the start of some minimal set. Minimal sets have indices 0 + k*i or nLeaves - 1 - k*i, 
      // where k is the increment and i is a counter
      // the easiest way is to obtain the number of sets and pick the first element of the first in the second half of sets
      // This would also work for the uniform policy, as it will give the middle element

      // Exponential policy:
      // picking the middle point would always require a set that has nLeaves/2 elements (in the direct case) or nLeaves/4 in the inverse case
      // using the strategy above of picking the middle set first will give us smaller sets. 
      // In total, there are log nLeaves sets. In the direct case, the ith set has 2^i elements. The half-set has 2^log nLeaves/2 = nLeaves^1/2 elements
      // In the inverse case, the ith set has 2^(log nLeaves - i) elements, and the half-set again has about 2^(log nLeaves - i) or less. So this always
      // generates smaller minimal sets and so is a better method to use.

      int begin;
      int minSetSize = findMiddleSetSizeAndFirstElement(realNLeaves, realNSets, k, begin);

      repeats = varRepeats[i] / minSetSize;
     
      //      OUT("Number of minimal sets: " << k);
      //      OUT("Leaves per minimal set: " << minSetSize);

      for (int n = 0; n < attInCTVars; n++) {
	int nAtts = attrsInCT[n];

	//	OUT("Number of leaves: " << nLeaves);
	//	OUT("Number of CTAtts: " << nAtts);

	if (nAtts < minSetSize) continue;

	CTAtts.clear();
	for (int k = 0; k < nAtts; k++) {
	  CTAtts.push_back( (begin + k) % attrsInUniverse);
	}

	testClass.encrypt(CTAtts, M, CT, AttFrags);	
	testClass.encryptS(CTAtts, sM, sCT, AttFrags);

	report_start(&a_t0);
	for (int j = 0; j < repeats; j++) {
	  testClass.decrypt( keyFrags, CTAtts, CT, AttFrags, PT);
	}
	report_finish(&a_t1);

	report_start(&h_t0);
// 	for (int j = 0; j < repeats; j++) {
// 	  testClass.decryptS( keyFrags, CTAtts, sCT, AttFrags, sPT);
// 	}
	report_finish(&h_t1);

// 	stringstream ss;
// 	ss << "#4 Decryption with " << policy_type << " policy:\n" << "Number of leaves in policy: " << realNLeaves << 
// 	  "\nNumber of minimal sets: " << realNSets << " --- " << "Number of CT attributes: " << nAtts;
// 	if (k_meaning != "") {
// 	  ss << " --- Number of " << k_meaning << ": " << k;
// 	}
// 	report_time( ss.str(), "Leaves in minimal set", repeats, minSetSize, t0, t1);
	report_dec_data(nLeaves, k, repeats, realNLeaves, realNSets, nAtts, minSetSize, a_t0, a_t1, h_t0, h_t1, expr);
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}


void measureKeyUnif(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {1000,500,100,50,25};
  int varRepeats[] = {1,1,1,1,1};

  DEBUG("Calling measureKeyFunc");
  measureKeyFunc(pfc, makeKeyUnifPolicy, leavesInPolicy, lvsInPol, varRepeats, nextUnifPol, stopUnifPol, 1, "Uniform", "");
}

void measureKeyLin(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {5000, 1000, 200, 50, 25};
  int varRepeats[] = {1,1,1,1,1};

  measureKeyFunc(pfc, makeKeyLinPolicy, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, 2, "Linear", "set size increment");
}
 
void measureKeyExp(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {2500,500,250,50,1};
  int varRepeats[] = {1,1,1,1,1};

  measureKeyFunc(pfc, makeKeyExpPolicy, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, 2, "Exponential", "set size increment factor");
}
 
void measureKeyLinInv(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {2500,1000,200,50,25};
  int varRepeats[] = {1,1,1,1,1};

  measureKeyFunc(pfc, makeKeyLinPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, 2, "Inverse Linear", "set size decrement");
}
 
void measureKeyExpInv(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {2500, 500, 250, 50, 25};
  int varRepeats[] = {1,1,1,1,1};

  measureKeyFunc(pfc, makeKeyExpPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, 2, "Inverse Exponential", "set size decrement factor");
}
 
void measureDecUnif(PFC &pfc, miracl* mip) { 
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {1024 * 10, 1024 * 16, 1024 * 16, 1024 * 16,1024 * 10};
  int varRepeats[] = {1,1,1,1,1};

  measureDecFunc(pfc, mip, makeKeyUnifPolicy, leavesInPolicy, lvsInPol, varRepeats, nextUnifPol, stopUnifPol, middleSetUnifPol, 1, "Uniform", ""); 
}

void measureDecLin(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {1024 * 16,1024 * 16,1024 * 16,1024 * 16,1024 * 16};
  int varRepeats[] = {1,1,1,1,1};

  measureDecFunc(pfc, mip, makeKeyLinPolicy, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, middleSetLinPol, 2, "Linear", "set size increment"); 
}
 
void measureDecExp(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {1024 * 8,1024 * 8,1024 * 8,1024 * 8,1024 * 8};
  int varRepeats[] = {1,1,1,1,1};

  measureDecFunc(pfc, mip, makeKeyExpPolicy, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, middleSetExpPol, 2, "Exponential", "set size increment factor"); 
}
 
void measureDecLinInv(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {1024 * 8,1024 * 8,1024 * 8,1024 * 8,1024 * 8};
  int varRepeats[] = {1,1,1,1,1};

  measureDecFunc(pfc, mip, makeKeyLinPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, middleSetLinPolInv, 2, "Inverse Linear", "set size decrement"); 
}
 
void measureDecExpInv(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

  //  int varRepeats[] = {1 * thousand,5 * hundred, 2*hundred ,1 * hundred,1 * hundred};
  //  int varRepeats[] = {1024 * 8,1024 * 8,1024 * 8,1024 * 8,1024 * 8};
  int varRepeats[] = {1,1,1,1,1};

  measureDecFunc(pfc, mip, makeKeyExpPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, middleSetExpPolInv, 2, "Inverse Exponential", "set size decrement factor"); 
}


//   int leavesInPolicy[] = {8, 32, 128, 512, 1024};
//   const int lvsInPol = 5; 
// 
//   int varRepeats[] = {1 * thousand,4 * hundred,1 * hundred,50,25};

int main(int argc, char* argv[] ) {
  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=16;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  parseInput(argc, argv);
  cout << endl;

  // Setup
  if (Setup) {
    measureSetup(pfc);
  }
  
  // Encryption
  if (Encrp) {
    measureEncrp(pfc, mip);
  }

  // Key Generation: Uniform policies
  if (KeyUnif) {
    measureKeyUnif(pfc);
  }
 
  if (KeyLin) {
    measureKeyLin(pfc);
  }
 
  if (KeyExp) {
    measureKeyExp(pfc);
  }
 
  if (KeyLinInv) {
    measureKeyLinInv(pfc);
  }
 
  if (KeyExpInv) {
    measureKeyExpInv(pfc);
  }
 
  if (DecUnif) {
    measureDecUnif(pfc, mip);
  }
 
  if (DecLin) {
    measureDecLin(pfc, mip);
  }
 
  if (DecExp) {
    measureDecExp(pfc, mip);
  }
 
  if (DecLinInv) {
    measureDecLinInv(pfc, mip);
  }
 
  if (DecExpInv) {
    measureDecExpInv(pfc, mip);
  }

  cout << "Nothing left to do" << endl;
  
  return 0;
}
