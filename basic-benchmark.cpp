/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file implements some basic time measurements, covering all the operations needed for the KP implementation

  My tests with measuring time by counting clicks have shown that this
  is not reliable. They usually give me half of the actual time
  passed. Because of this, I will adopt the method of repeating the
  experiment a large number of times and measuring the seconds via the
  time function. This has the added benefit of smoothing out
  variations in individual executions.

  The operations to measure are the following:
  
  - Sampling of a pseudo-random Big 
  - Creation of random elements in G1 and G2 
  - Creation of elements in G1 and G2 giving a generator and an exponent (multiplication) 
  - Exponentiation in GT 
  - Pairing of random elements in G1,G2 
  - Multiplication in GT 
  - Modular Division with Bigs 
  - Hash and XOR of message 

*/

#include <ctime>
#include "utils.h"

void report_time(const string& description, long repeats, time_t initial, time_t final) {
  double elapsed = final - initial;
  cout << "--------------------------------------------------------------" << endl;
  cout << "Task: " << description << endl;
  cout << "Number of repetitions: " << repeats << endl;
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

  cout << "Average execution time: " << average << " " << unit << endl;
  cout << "--------------------------------------------------------------" << endl;
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

bool RndBig = false;
bool RndG1 = false;
bool RndG2 = false;
bool MultG1 = false;
bool MultG2 = false;
bool ExpGT = false;
bool PurePair = false;
bool FullPairWithExp = false;
bool MultGT = false;
bool ArithBig = false;
bool HashAndXor = false;
  
void parseInput(int argc, char* argv[]){
  for (int i = 0; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "rb") RndBig = true;
    if (arg == "rg1") RndG1 = true;
    if (arg == "rg2") RndG2 = true;
    if (arg == "mg1") MultG1 = true;
    if (arg == "mg2") MultG2 = true;
    if (arg == "egt") ExpGT = true;
    if (arg == "pr") PurePair = true;
    if (arg == "prxp") FullPairWithExp = true;
    if (arg == "mgt") MultGT = true;
    if (arg == "arit") ArithBig = true;
    if (arg == "hx") HashAndXor = true;
    if (arg == "all") {
      RndBig = RndG1 = RndG2 = MultG1 = MultG2 = ExpGT = PurePair = FullPairWithExp = MultGT = ArithBig = HashAndXor = true;
    }
  }
}

int main(int argc, char* argv[] ) {
  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=16;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  long million = 1000000;
  long thousand = 1000;
  
  long repeats;
  time_t t0;
  time_t t1;
  double elapsed = 0;

  parseInput(argc, argv);
  cout << endl;
  
  // Sampling Random Bigs
  if (RndBig) {
    Big randomBig;
    repeats = 5 * million;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(randomBig);
    }
    report_finish(&t1);
    report_time("#1 Sampling Random Bigs", repeats, t0, t1);
  }

  // Sampling G1 elements
  if (RndG1) {
    G1 P;
    repeats = 500 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(P);
    }
    report_finish(&t1);
    report_time("#2 Sampling Random G1 elements", repeats, t0, t1);
  }

  // Sampling G2 elements
  if (RndG2) {
    G2 Q;
    repeats = 50 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(Q);
    }
    report_finish(&t1);
    report_time("#3 Sampling Random G2 elements", repeats, t0, t1);
  }

  // Creating G1 elements by multiplication with generator
  if (MultG1) {
    G1 P;
    Big r;

    pfc.random(P);

    repeats = 100 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.mult(P,r);
    }
    report_finish(&t1);
    report_time("#4.1 Creating G1 elements with exponent", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.mult(P,r);
    }
    report_finish(&t1);
    report_time("#4.2 Creating G1 elements using a fixed exponent", repeats, t0, t1);

  }

  // Creating G2 elements by multiplication with generator
  if (MultG2) {
    G2 Q;
    Big r;

    pfc.random(Q);

    repeats = 100 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.mult(Q,r);
    }
    report_finish(&t1);
    report_time("#5.1 Creating G2 elements with exponent", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.mult(Q,r);
    }
    report_finish(&t1);
    report_time("#5.2 Creating G2 elements using a fixed exponent", repeats, t0, t1);
  }

  // Exponentation in GT
  if (ExpGT) {
    Big r;
    G1 P;
    G2 Q;
    GT Y;

    pfc.random(P);
    pfc.random(Q);
    Y = pfc.pairing(Q,P);

    repeats = 50 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.power(Y,r);
    }
    report_finish(&t1);
    report_time("#6.1 Computing exponentiations on GT", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.power(Y,r);
    }
    report_finish(&t1);
    report_time("#6.2 Computing exponentiations on GT with fresh random exponent", repeats, t0, t1);    
  }


  // Pairs of random elements from G1 and G2
  if (PurePair) {
    G1 P;
    G2 Q;

    pfc.random(P);
    pfc.random(Q);

    repeats = 10 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.pairing(Q,P);
    }
    report_finish(&t1);
    report_time("#7.1 Computing pure pairings", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(P);
      pfc.random(Q);
      pfc.pairing(Q,P);
    }
    report_finish(&t1);
    report_time("#7.2 Computing pure pairings with fresh random elements ", repeats, t0, t1);
  }


  // Pairs of random elements with exponentiation
  if (FullPairWithExp) {
    Big r;
    GT Y;
    G1 P;
    G2 Q;
    G1 Pr;
    G2 Qr;

    pfc.random(P);
    pfc.random(Q);

    repeats = 10 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.random(P);
      pfc.random(Q);
      Y = pfc.pairing(Q,P);
      pfc.power(Y,r);
    }
    report_finish(&t1);
    report_time("#8.1 Computing pure pairings and then exponentiating", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.random(P);
      pfc.random(Q);
      Pr = pfc.mult(P,r);
      pfc.pairing(Q,Pr);
    }
    report_finish(&t1);
    report_time("#8.2 Exponentiating in G1 and then pairing with G2", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.random(P);
      pfc.random(Q);
      Qr = pfc.mult(Q,r);
      pfc.pairing(Qr,P);
    }
    report_finish(&t1);
    report_time("#8.3 Exponentiating in G2 and then pairing with G1", repeats, t0, t1);
  }

  // Exponentation in GT
  if (ExpGT) {
    Big r;
    G1 P;
    G2 Q;
    GT Y;

    pfc.random(P);
    pfc.random(Q);
    Y = pfc.pairing(Q,P);

    repeats = 50 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.power(Y,r);
    }
    report_finish(&t1);
    report_time("#9.1 Computing pairing exponentiations", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      pfc.power(Y,r);
    }
    report_finish(&t1);
    report_time("#9.2 Computing pairing exponentiations with fresh random exponent", repeats, t0, t1);    
  }

  // Multiplication in GT
  if (MultGT) {
    Big r;
    G1 P;
    G2 Q;
    GT M;
    GT N;

    pfc.random(P);
    pfc.random(Q);
    
    pfc.random(r);
    M = pfc.pairing(Q,pfc.mult(P,r));
    pfc.random(r);
    N = pfc.pairing(Q,pfc.mult(P,r));

    repeats = 1 * million;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      M = M * N;
    }
    report_finish(&t1);
    report_time("#10.1 Multiplying fixed random elements of GT", repeats, t0, t1);

    repeats = 5 * thousand;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      N = pfc.pairing(Q,pfc.mult(P,r));

      M = M * N;
    }
    report_finish(&t1);
    report_time("#10.2 Multiplying fresh random elements of GT", repeats, t0, t1);    

    repeats = 1 * million;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      M = M / N;
    }
    report_finish(&t1);
    report_time("#11.1 Dividing fixed random elements of GT", repeats, t0, t1);

    repeats = 5 * thousand;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      N = pfc.pairing(Q,pfc.mult(P,r));

      M = M / N;
    }
    report_finish(&t1);
    report_time("#11.2 Dividing fresh random elements of GT", repeats, t0, t1);    

  }

  // Arithmetic with Bigs
  if (ArithBig) {
    Big a;
    Big b;
    Big r;
    Big s;
    Big order;
    repeats = 5 * million;


    order = pfc.order();
    pfc.random(a);
    pfc.random(b);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      a += b;
      a %= order;
    }
    report_finish(&t1);
    report_time("#13.1 Adding Fixed Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(b);      
      a += b;
      a %= order;
    }
    report_finish(&t1);
    report_time("#13.2 Adding distinct Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(a);
      pfc.random(b);      
      a += b;
      a %= order;
    }
    report_finish(&t1);
    report_time("#13.3 Adding both distinct Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      a = modmult(a,b,order);
    }
    report_finish(&t1);
    report_time("#14.1 Multiplying fixed Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(b);
      a = modmult(a,b,order);
    }
    report_finish(&t1);
    report_time("#14.2 Multiplying distinct Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(a);
      pfc.random(b);
      a = modmult(a,b,order);
    }
    report_finish(&t1);
    report_time("#14.3 Multiplying both distinct Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      a = moddiv(a,b,order);
    }
    report_finish(&t1);
    report_time("#15.1 Dividing fixed Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(b);
      a = moddiv(a,b,order);
    }
    report_finish(&t1);
    report_time("#15.2 Dividing distinct Random Bigs", repeats, t0, t1);

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(a);
      pfc.random(b);
      a = moddiv(a,b,order);
    }
    report_finish(&t1);
    report_time("#15.3 Dividing both distinct Random Bigs", repeats, t0, t1);
  }

  // Hash and XOR
  if (HashAndXor) {
    Big r;
    G1 P;
    G2 Q;
    Big M;
    GT N;


    pfc.random(P);
    pfc.random(Q);
    
    pfc.random(M);
    pfc.random(r);
    N = pfc.pairing(Q,pfc.mult(P,r));

    repeats = 1 * million ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      M = lxor(M,pfc.hash_to_aes_key(N));
    }
    report_finish(&t1);
    report_time("#16.1 Hashing and Xoring fixed random elements of GT", repeats, t0, t1);

    repeats = 5 * thousand ;

    report_start(&t0);
    for (int i = 0; i < repeats; i++) {
      pfc.random(r);
      N = pfc.pairing(Q,pfc.mult(P,r));
      M = lxor(M,pfc.hash_to_aes_key(N));
    }
    report_finish(&t1);
    report_time("#16.2 Hashing and Xoring fresh random elements of GT", repeats, t0, t1);    

  }


  cout << "Nothing left to do";
  
  return 0;
}
