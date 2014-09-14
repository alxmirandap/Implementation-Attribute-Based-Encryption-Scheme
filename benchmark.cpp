/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file implements a full testbed for the operations of the KPABE scheme. It tests the time taken by each of the basic operations: 
  Setup, Encryption, Key Generation, Decryption. 

  The group structure has two groups G1 and G2 and one Pairing Group GT. One of the groups will serve to build the key fragments, while the other group will build
  the attribute fragments. 
  The function of each group is determined by using one of two constants: #define AttOnG1_KeyOnG2 or #define AttOnG2_KeyOnG1.

  This source code is used to produce 4 different variants: for each of the two variants of attribute use, there are two possible secret sharing schemes to use: 
  a canonical Benaloh-Leichter or a tree of Shamir schemes.

  These variants are obtained by four different compiling instructions: 
  - the definition of attribute use is defined in atts.h, which is a copy of either atts.h_1 or atts.h_2
  - the definition of secret sharing scheme to use is defined in benchmark_defs.h, which is a copy of either benchmark_defs_bl.h or benchmark_defs_sh.h.

  My tests with measuring time by counting clicks have shown that this
  is not reliable. They usually give me half of the actual time
  passed. Because of this, I will adopt the method of repeating the
  experiment a large number of times and measuring the seconds via the
  time function. This has the added benefit of smoothing out
  variations in individual executions.

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

// #define TEST_RUN


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
   time_t now = time(t);
   char* dt = ctime(&now);
   cout << action << dt; 
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



double million = 1000000;
double thousand = 1000;
double hundred = 100;

void adjustScale(double value, std::string &unit, double &adjustedValue) {
  if (value >= 1) {
    unit = "seconds";
    adjustedValue = value;
    return;
  }

  if (value >= 1/thousand) {
    unit = "milliseconds";
    adjustedValue = value * thousand;
    return;
  }

  if (value >= 1/million) {
    unit = "microseconds";
    adjustedValue = value * million;
    return;
  } else {
    unit = "seconds";
    adjustedValue = value;
  }

}

void  report_title(std::string title, std::string varParam, std::string params1, std::string params2="", std::string params3="" ) {
  stringstream ss;
  ss << "Task: " << title << std::endl 
     << params1 << "\t" << "Time per rep" << "\t\t" << "Time per " << varParam << "\t";

  if (params2 != "") {
    ss << params2 << "\t" << "Time per rep" << "\t\t" << "Time per " << varParam << "\t";
  }

  ss << params3 << std::endl;

  cout << ss.str();
}

void report_setup_data(int nAtts, long repeats, time_t initial, time_t final) {
  double elapsed = final - initial;
  double timeRep = elapsed / repeats;
  double timeAtt = timeRep / nAtts;

  std::string unit1;
  double value1;

  std::string unit2;
  double value2;

  adjustScale(timeRep, unit1, value1);
  adjustScale(timeAtt, unit2, value2);
 
  cout << nAtts << "\t" << repeats << "\t" << elapsed << "\t" << value1 << "\t" << unit1 << "\t" << value2 << "\t" << unit2 << std::endl;
}

void report_encr_data(int nAtts, long repeats, time_t a_initial, time_t a_final, time_t h_initial, time_t h_final) {
  double a_elapsed = a_final - a_initial;
  double h_elapsed = h_final - h_initial;
  double a_timeRep = a_elapsed / repeats;
  double h_timeRep = h_elapsed / repeats;
  double a_timeAtt = a_timeRep / nAtts;
  double h_timeAtt = h_timeRep / nAtts;

  std::string a_unit1;
  double a_value1;

  std::string a_unit2;
  double a_value2;

  adjustScale(a_timeRep, a_unit1, a_value1);
  adjustScale(a_timeAtt, a_unit2, a_value2);

  std::string h_unit1;
  double h_value1;

  std::string h_unit2;
  double h_value2;

  adjustScale(h_timeRep, h_unit1, h_value1);
  adjustScale(h_timeAtt, h_unit2, h_value2);
  
  cout << nAtts << "\t" << repeats << "\t" << a_elapsed << "\t" << a_value1 << "\t" << a_unit1 << "\t" << a_value2 << "\t" << a_unit2
       << "\t" << h_elapsed << "\t" << h_value1 << "\t" << h_unit1 << "\t" << h_value2 << "\t" << h_unit2 << std::endl;
}

void report_key_data(int max, int k, long repeats, long nLeaves, long nSets, time_t initial, time_t final, std::string policy) {
  double elapsed = final - initial;
  double timeRep = elapsed / repeats;
  double timeAtt = timeRep / nLeaves;

  std::string unit1;
  double value1;

  std::string unit2;
  double value2;

  adjustScale(timeRep, unit1, value1);
  adjustScale(timeAtt, unit2, value2);
  
  cout << nLeaves << "\t" << nSets << "\t" << max << "\t" << k << "\t" << repeats << "\t" << elapsed 
       << "\t" << value1 << "\t" << unit1 << "\t" << value2 << "\t" << unit2
       << "\t" << policy << std::endl;
}

void report_dec_data(int max, int k, long repeats, long nLeaves, long nSets, long nAtts, long minSetSize, time_t a_initial, time_t a_final, 
		     time_t h_initial, time_t h_final, std::string policy) {
  double a_elapsed = a_final - a_initial;
  double h_elapsed = h_final - h_initial;
  double a_timeRep = a_elapsed / repeats;
  double h_timeRep = h_elapsed / repeats;
  double a_timeAtt = a_timeRep / minSetSize;
  double h_timeAtt = h_timeRep / minSetSize;

  std::string a_unit1;
  double a_value1;

  std::string a_unit2;
  double a_value2;

  adjustScale(a_timeRep, a_unit1, a_value1);
  adjustScale(a_timeAtt, a_unit2, a_value2);

  std::string h_unit1;
  double h_value1;

  std::string h_unit2;
  double h_value2;

  adjustScale(h_timeRep, h_unit1, h_value1);
  adjustScale(h_timeAtt, h_unit2, h_value2);
  
  cout << nLeaves << "\t" << nSets << "\t" << nAtts << "\t" << minSetSize << "\t" << max << "\t" << k << "\t" << repeats 
       << "\t" << a_elapsed << "\t" << a_value1 << "\t" << a_unit1 << "\t" << a_value2 << "\t" << a_unit2
       << "\t" << h_elapsed << "\t" << h_value1 << "\t" << h_unit1 << "\t" << h_value2 << "\t" << h_unit2 
       << "\t" << policy << std::endl;
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

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {1000,1000,500,250,100};
#endif


  long repeats;

  stringstream ss;
  ss << "#Atts in Universe" << "\t" << "Repetitions" << "\t" << "Total time";
  report_title("#1 Setup times: ", "Attribute", ss.str());

  time_t t0;
  time_t t1;

  for (int i = 0; i < attInUnivVars; i++){
    int nAtts = attrsInUniverse[i];
    repeats = varRepeats[i];
    KPABE testClass(testScheme, pfc, nAtts);    
    testClass.paramsgen(P, Q, order);

    get_time(&t0);
    for (int j = 0; j < repeats; j++) {
      testClass.setup();
    }
    get_time(&t1);
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

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {2500,2500,2000,500,500};
#endif
 

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
  ss << "#Atts in Ciphertext" << "\t" << "Repetitions" << "\t" << "Total time (Algebraic)";

  report_title("#2 Encryption times: ", "Attribute", ss.str(), "Total time (Hash)");

  for (int i = 0; i < attInCTVars; i++){
    int nAtts = attrsInCT[i];
    repeats = varRepeats[i];

    // OUT("New repetition: setting attributes");

    CTAtts.clear();
    for (int k = 0; k < nAtts; k++) {
      CTAtts.push_back(k);
    }

    // OUT("Calling report");
    get_time(&a_t0);
    for (int j = 0; j < repeats; j++) {
      testClass.encrypt(CTAtts, M, CT, AttFrags);
    }
    get_time(&a_t1);
    //     stringstream ss;
    //     ss << "#2 Encryption times (Algebraic): " << "Number of attributes in ciphertext: " << nAtts;
    //     report_time( ss.str(), "Attribute", repeats, nAtts, t0, t1);

    get_time(&h_t0);
    for (int j = 0; j < repeats; j++) {
      testClass.encryptS(CTAtts, sM, sCT, AttFrags);
    }
    get_time(&h_t1);
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
  // increment = # of sets, so equal to nSets in this function
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
  int n = nSets - middleSet - 1;
  int setSize = 1 + n * increment;

  begin = (nLeaves - 1) - n * (1 + increment * (nSets - 1) - increment * (nSets - 2 + middleSet)/2);
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
  int setSize = std::pow(increment, middleSet);
  begin = (setSize - 1) / (increment - 1);
  return setSize;
}

int middleSetExpPolInv(int nLeaves, int nSets, int increment, int &begin){
  int middleSet = nSets / 2;
  int n = nSets - middleSet - 1;
  int setSize = std::pow(increment, n);
  int cumSize = (setSize * increment - 1)/(increment - 1);
  begin = nLeaves - cumSize;
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
  ss << "#Leaves in Policy" << "\t" << "#Minimal Sets" << "\t" << "Max Leaves" << "\t" << "Control Param" << "\t" << "Repetitions" 
     << "\t" << "Total time";
  
  report_title("#3 Key Generation with " + policy_type + " policy:", "Share", ss.str(), "Policy");


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
      
      get_time(&t0);
      DEBUG("Start time: " << t0);
      for (int j = 0; j < repeats; j++) {
	DEBUG("running repeat: " << j);
	testClass.genKey();
      }
      get_time(&t1);
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
       << "minSetSize" << "\t" << "Max Leaves" << "\t" << "Control Params" << "\t" << "Repetitions" << "\t" << "Total time (Alg)";

  report_title("#4 Decryption with " + policy_type + " policy:", "Size of Decrypting Set", ss.str(), "Total time (Hash)", "Policy");


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
      int minSetSize = findMiddleSetSizeAndFirstElement(nLeaves, realNSets, k, begin);

      if (varRepeats[i] < minSetSize) {
	repeats = varRepeats[i];
      } else {
	repeats = varRepeats[i] / minSetSize;
      }
     
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

	get_time(&a_t0);
	for (int j = 0; j < repeats; j++) {
	  testClass.decrypt( keyFrags, CTAtts, CT, AttFrags, PT);
	}
	get_time(&a_t1);

	get_time(&h_t0);
 	for (int j = 0; j < repeats; j++) {
 	  testClass.decryptS( keyFrags, CTAtts, sCT, AttFrags, sPT);
 	}
	get_time(&h_t1);

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
#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {2500,1000,500,250,100};
#endif



  DEBUG("Calling measureKeyFunc");
  measureKeyFunc(pfc, makeKeyUnifPolicy, leavesInPolicy, lvsInPol, varRepeats, nextUnifPol, stopUnifPol, 1, "Uniform", "");
}

void measureKeyLin(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {5000, 2500, 500, 250, 100};
#endif

  measureKeyFunc(pfc, makeKeyLinPolicy, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, 2, "Linear", "set size increment");
}
 
void measureKeyExp(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {5000, 2500, 500, 250, 100};
#endif


  measureKeyFunc(pfc, makeKeyExpPolicy, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, 2, "Exponential", "set size increment factor");
}
 
void measureKeyLinInv(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {5000, 2500, 500, 250, 100};
#endif


  measureKeyFunc(pfc, makeKeyLinPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, 2, "Inverse Linear", "set size decrement");
}
 
void measureKeyExpInv(PFC &pfc) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {5000, 2500, 500, 250, 100};
#endif


  measureKeyFunc(pfc, makeKeyExpPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, 2, "Inverse Exponential", "set size decrement factor");
}
 
void measureDecUnif(PFC &pfc, miracl* mip) { 
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {1024 * 16, 1024 * 16, 1024 * 16, 1024 * 32,1024 * 64};
#endif


  measureDecFunc(pfc, mip, makeKeyUnifPolicy, leavesInPolicy, lvsInPol, varRepeats, nextUnifPol, stopUnifPol, middleSetUnifPol, 1, "Uniform", ""); 
}

void measureDecLin(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 

#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {9000, 15000, 20000, 40000,40000};
#endif




  measureDecFunc(pfc, mip, makeKeyLinPolicy, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, middleSetLinPol, 2, "Linear", "set size increment"); 
}
 
void measureDecExp(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 


#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {9000, 15000, 20000, 40000,40000};
#endif


  measureDecFunc(pfc, mip, makeKeyExpPolicy, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, middleSetExpPol, 2, "Exponential", "set size increment factor"); 
}
 
void measureDecLinInv(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 


#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {9000, 15000, 20000, 40000,40000};
#endif

  measureDecFunc(pfc, mip, makeKeyLinPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextLinPol, stopLinPol, middleSetLinPolInv, 2, "Inverse Linear", "set size decrement"); 
}
 
void measureDecExpInv(PFC &pfc, miracl* mip) {
  int leavesInPolicy[] = {8, 32, 128, 512, 1024};
  const int lvsInPol = 5; 


#ifdef TEST_RUN
  int varRepeats[] = {1,1,1,1,1};
#else
  int varRepeats[] = {9000, 15000, 20000, 40000,40000};
#endif

  measureDecFunc(pfc, mip, makeKeyExpPolicyInv, leavesInPolicy, lvsInPol, varRepeats, nextExpPol, stopExpPol, middleSetExpPolInv, 2, "Inverse Exponential", "set size decrement factor"); 
}



int main(int argc, char* argv[] ) {
  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=16;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  parseInput(argc, argv);
  cout << endl;

  time_t t0;
  time_t t1;

  report_start(&t0);

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

  report_finish(&t1);

  int elapsed = t1 - t0;
  int seconds = elapsed % 60;
  int elapsed_m = (elapsed - seconds) / 60;
  int minutes = elapsed_m % 60;
  int hours = (elapsed_m - minutes) / 60;
  
  cout << "Total time (seconds): " << elapsed << std::endl;
  cout << "Total time: " << hours << "h" << minutes << "m" << seconds << std::endl;

  
  return 0;
}
