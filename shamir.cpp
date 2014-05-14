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


SharePair::SharePair(): 
  partIndex(0), share(0)
{}

SharePair::SharePair(const int pi, const Big s):
  partIndex(pi), share(s)
{} 

SharePair::SharePair(const SharePair& other) : 
  partIndex(other.partIndex),
  share(other.share) 
{}

SharePair& SharePair::operator=(const SharePair& other)
{  
  partIndex = other.partIndex;
  share = other.share;
  return *this;
}

void SharePair::setValues(const int pi, const Big s)
{
  partIndex = pi;
  share = s;
}


bool SharePair::operator==(const SharePair& rhs) const {
  if (partIndex != rhs.partIndex) return false;
  if (share != rhs.share) return false;
  return true;
}

string SharePair::to_string() {
  std::stringstream sstrm;
  sstrm << "[" << partIndex << ";" << share << "] ";
  return sstrm.str();
}

int SharePair::getPartIndex() const{
  return partIndex;
}

Big SharePair::getShare() const {
    return share;
}

std::ostream& operator<<(ostream& out, const SharePair &sp) {
  return out << "[" << sp.getPartIndex() << ";" << sp.getShare() << "] ";
}

//---------------------------------------------

ShamirAccessPolicy::ShamirAccessPolicy():
  m_k(0)
{
  m_participants.push_back(1);
}

ShamirAccessPolicy::ShamirAccessPolicy(const int k, const int n):
  m_k(k)
{
  for (int i = 0; i < n; i++) {
    m_participants.push_back(i+1);
  }
}

ShamirAccessPolicy::ShamirAccessPolicy(const int k, const vector<int> parts):
  m_k(k), m_participants(parts)
{}

ShamirAccessPolicy::ShamirAccessPolicy(const ShamirAccessPolicy& other):
  m_k(other.m_k), m_participants(other.m_participants)
{}

ShamirAccessPolicy& ShamirAccessPolicy::operator=(const ShamirAccessPolicy& other)
{
  m_k = other.m_k;
  m_participants = other.m_participants;
  return *this;
}

int ShamirAccessPolicy::getThreshold()
{
  return m_k;
}

int ShamirAccessPolicy::getNumParticipants()
{
  return m_participants.size();
}

vector<int> ShamirAccessPolicy::getParticipants()
{
  return m_participants;
}

int ShamirAccessPolicy::getNumShares(){
  return getNumParticipants();
}

//---------------------------------------------

ShamirSS::ShamirSS(const ShamirAccessPolicy policy, const Big& in_order, PFC &pfc):
  m_policy(policy), m_order(in_order), m_pfc(pfc)
{}

ShamirSS::ShamirSS(const int in_k, const int nparts, const Big& in_order, PFC &pfc):
  m_policy(ShamirAccessPolicy(in_k, nparts)), m_order(in_order), m_pfc(pfc)
{}

ShamirSS::ShamirSS(const int in_k, const vector<int> parts, const Big& in_order, PFC &pfc):
  m_policy(ShamirAccessPolicy(in_k, parts)), m_order(in_order), m_pfc(pfc)
{}

Big ShamirSS::lagrange(const int i,const vector<int> parts)
  {
    if (parts.size() < getThreshold()) return 0; // an error value, since the Lagrange coefficient can never be 0

    int j;
    Big z=1;

    for (int k=0;k<getThreshold();k++)
      {
	j=parts[k];
	if (j!=i) z=modmult(z,moddiv(-j,(Big)(i-j),m_order),m_order);
      }
    return z;
  }

Big ShamirSS::reconstruct (const vector<SharePair> shares) {
    DEBUG("============================== RECONSTRUCTION ==============================");  
    int nparts = shares.size();
    if (nparts < getThreshold()) return -1; // a fail value, since a share is always positive

    DEBUG("Reconstruction: (k)---" << getThreshold());
    vector<int> parts(getThreshold());
    for (int i=0; i < getThreshold(); i++){
      parts[i] = shares[i].getPartIndex();
    }
 
    Big s = 0;
    Big t;
    Big c;

    //DEBUG("Reconstruction: ");
    DEBUG("MODULO: " << m_order);
    for (int i = 0; i < getThreshold(); i++){
      c = lagrange(parts[i], parts);
      DEBUG("Part: " << parts[i] << " coefficient: " << c << " Share: " << shares[i].getShare());
      t = modmult(shares[i].getShare(),c,m_order);
      DEBUG("Contribution: " << t);
      s = (s + t); 
      DEBUG("Temporary secret: " << s);
    }
    s %= m_order;
    return s;
  }

int ShamirSS::getThreshold(){
  return m_policy.getThreshold();
}

int ShamirSS::getNumParticipants(){
  return m_policy.getNumParticipants();
}

vector<int> ShamirSS::getParticipants(){
  return m_policy.getParticipants();
}

std::vector<SharePair> ShamirSS::distribute_random(const Big& s){
  int npoly = getThreshold()-1;
  vector<Big> poly(npoly);
    DEBUG("============================== DISTRIBUTE RANDOM ==============================");  
    DEBUG("npoly: " << npoly);
  for (int i=0;i<npoly;i++){
    m_pfc.random(poly[i]); // random polynomial coefficient
  }
  DEBUG("Prepared randomness, ready to call deterministic algorithm");
  return distribute_determ(s, poly);
}

std::vector<SharePair> ShamirSS::distribute_determ(const Big& s, vector<Big> &randomness){
  DEBUG("Randomness size: " << randomness.size());
  guard("Secret must be smaller than group order", s < m_order);
  //  DEBUG("Degree minus 1: " << getThreshold() - 1);
  guard("Distribute algorithm has received randomness of the wrong size", randomness.size() == getThreshold()-1);

  vector<Big> poly(getThreshold());	// internal representation of the shamir polynomial
  int pi; // participant index
  Big acc; // cummulative sum for computing the polynomial
  Big accX; // cummulative value for the variable power (x^i)
  std::vector<SharePair> shares(getNumParticipants());  

  poly[0]=s;
  for (int i = 1; i < getThreshold(); i++){
    poly[i] = randomness[i-1];
  }
    	
  for (int j=0;j<getNumParticipants();j++) {
    pi=getParticipants()[j];
    acc=poly[0]; accX=pi;
    for (int k=1;k<getThreshold();k++) { 
      // evaluate polynomial a0+a1*x+a2*x^2... for x=pi;
      acc+=modmult(poly[k],(Big)accX,m_order); 
      accX*=pi;
      acc%=m_order;
    }    
    shares[j].setValues(pi, acc);
    DEBUG("Share " << j << shares[j]);
  }
  return shares;
}


