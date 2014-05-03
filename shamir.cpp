/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file is an implementation of Shamir's Secret Sharing Scheme. 
  It should be compiled as a library, and is meant to be used by the testbed.
  For compilation details, see the main file. Here, I will list only the needed instructions for this file.

  Using BN curves for a security level of AES-128. k = 12, for Type 3 pairings.

  Compile with modules as specified below:

  - Compile this file as

  	g++ -O2 -m64 -c -DZZNS=4 shamir.cpp -lbn -lmiracl -lpairs -o shamir.o

*/

#include "utils.h"
#include "shamir.h"

// Global Declarations



 
Big ShamirSS::lagrange(int i,vector<int> parts)
  {
    if (parts.size() < m_k) return 0; // an error value, since the Lagrange coefficient can never be 0

    int j;
    Big z=1;

    for (int k=0;k<m_k;k++)
      {
	j=parts[k];
	if (j!=i) z=modmult(z,moddiv(-j,(Big)(i-j),m_order),m_order);
      }
    return z;
  }

Big ShamirSS::reconstruct (vector<SharePair> shares) {
    int nparts = shares.size();
    if (nparts < m_k) return -1; // a fail value, since a share is always positive

    vector<int> parts(m_k);
    for (int i=0; i < m_k; i++){
      parts[i] = shares[i].getPartIndex();
    }
 
    Big s = 0;
    Big t;
    Big c;

    //DEBUG("Reconstruction: ");
   
    for (int i = 0; i < m_k; i++){
      c = lagrange(parts[i], parts);
      //OUT("Part: " << i << " coefficient: " << c << " Share: " << shares[i].getShare());
      t = modmult(shares[i].getShare(),c,m_order);
      //OUT("Contribution: " << t);
      s = (s + t) % m_order;
      //OUT("Temporary secret: " << s);
    }
    return s;
  }

std::vector<SharePair> ShamirSS::distribute_random(const Big& s){
    vector<Big> poly(m_k-1);
    for (int i=1;i<m_k;i++){
      m_pfc.random(poly[i]); // random polynomial coefficient
    }
    return distribute_determ(s, poly);
  }

std::vector<SharePair> ShamirSS::distribute_determ(const Big& s, vector<Big> randomness){
  DEBUG("Randomness size: " << randomness.size());
  DEBUG("Degree minus 1: " << m_k - 1);
  //  guard("Distribute algorithm has received randomness of the wrong size", randomness.size() == m_k-1);

  vector<Big> poly(m_k);	// internal representation of the shamir polynomial
  int pi; // participant index
  Big acc; // cummulative sum for computing the polynomial
  Big accX; // cummulative value for the variable power (x^i)
  std::vector<SharePair> shares(m_nparts);  

  poly[0]=s;
  for (int i = 1; i < m_k; i++){
    poly[i] = randomness[i-1];
  }
    	
  for (int j=0;j<m_nparts;j++) {
    pi=m_participants[j];
    acc=poly[0]; accX=pi;
    for (int k=1;k<m_k;k++) { 
      // evaluate polynomial a0+a1*x+a2*x^2... for x=pi;
      acc+=modmult(poly[k],(Big)accX,m_order); 
      accX*=pi;
      acc%=m_order;
    }    
    shares[j].setValues(pi, acc);
  }
  return shares;
}


