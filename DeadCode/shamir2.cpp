/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file is an implementation of Shamir's Secret Sharing Scheme, 
  deriving it from an abstract base class.
  It should be compiled as a library, and is meant to be used by the testbed.
  For compilation details, see the make file. 

  Using BN curves for a security level of AES-128. k = 12, for Type 3 pairings.
*/

#include "shamir2.h"


ShamirAccessPolicy::ShamirAccessPolicy():
  m_threshold(0)
{
  // calls default base constructor
  m_shareParts.push_back(1);  
}

ShamirAccessPolicy::ShamirAccessPolicy(const int k, const unsigned int n):
  AccessPolicy(n), 
  m_threshold(k)
{
  // calls specific base constructor
  for (unsigned int i = 0; i < n; i++){
    m_shareParts.push_back(1);
  }
}

ShamirAccessPolicy::ShamirAccessPolicy(const int k, const vector<int> &parts):
  AccessPolicy(parts), 
  m_threshold(k)
{
  for (unsigned int i = 0; i < parts.size(); i++) {
    m_shareParts.push_back(1);
  }
}

ShamirAccessPolicy::ShamirAccessPolicy(const int k, const vector<int> &parts, const vector<int> &shareParts):
  AccessPolicy(parts), 
  m_threshold(k), m_shareParts(shareParts)
{
  guard("shareParts and parts must be the same size", parts.size() == shareParts.size());
}

ShamirAccessPolicy::ShamirAccessPolicy(const ShamirAccessPolicy& other):
  AccessPolicy(other.m_participants), 
  m_threshold(other.m_threshold), m_shareParts(other.m_shareParts)
{}

ShamirAccessPolicy& ShamirAccessPolicy::operator=(const ShamirAccessPolicy& other)
{
  m_threshold = other.m_threshold;
  m_participants = other.m_participants;
  m_shareParts = other.m_shareParts;
  return *this;
}

unsigned int ShamirAccessPolicy::getThreshold() const
{
  return m_threshold;
}

unsigned int ShamirAccessPolicy::getNumShares() const{
  unsigned int nshares = 0;
  for (unsigned int i = 0; i < m_shareParts.size(); i++){
    nshares += m_shareParts[i];
  }
  return nshares;
}

bool ShamirAccessPolicy::evaluate(const vector<ShareTuple> uniqueShares, vector<ShareTuple> &witnessShares) const{
  // Threshold policies are satisfied by any set of at least k shares 
  // of the same polynomial.
  // When receiving random shares, it is not possible to know if they came
  // from the same distribution process, so we can not really guarantee that
  // the shares will actually be valid for a given secret.
  // However, we can evaluate if they are formally enough to find the secret
  // Notice that even in a pure Shamir scheme, one participant may have more
  // than one share. 
  // This method does that check, but it assumes that the shares it receives are unique. 
  // There is another method elsewhere that produces a list of unique shares up to a certain number.

  // In Shamir's scheme, each share is given to a participant (possibly 
  // repeating participants) and includes a public value and a private value.
  // The public is necessary to compute the Lagrange coefficients, and is 
  // unique for each share, even if a share's value and participant are not.
  // This public value is often an index, and I assume that in here.
  witnessShares.clear();

  if (uniqueShares.size() < m_threshold) {
    return false; // witnessShares remains an empty vector
  }
  for (unsigned int i = 0; i < m_threshold; i++){
    witnessShares.push_back(uniqueShares[i]);
  }
  return true;
}

vector<int> ShamirAccessPolicy::getSharesByParticipant() const{
  return m_shareParts;
}


//=========================================================

inline void ShamirSS::initPolicy(){
  ShamirAccessPolicy* tempPtr = dynamic_cast<ShamirAccessPolicy*> (m_policy);
  if (tempPtr) {
    i_policy = *tempPtr;
  } else {
    cerr << "ShamirSecretSharing has an AccessPolicy that is not ShamirAccessPolicy!";
    throw std::runtime_error(ERR_BAD_POLICY);
  }
}

inline void ShamirSS::initPoly(){
    int npoly = getThreshold()-1;
    poly.reserve(npoly);
    for (int i = 0; i < npoly; i++){
      poly.push_back(0);
    }
}

void ShamirSS::init(){
  initPolicy();
  initPoly();
}

ShamirSS::ShamirSS(ShamirAccessPolicy* policy, const Big &order, PFC &pfc):
  SecretSharing(policy, order, pfc)
{
  init();
}

ShamirSS::ShamirSS(const int in_k, const int nparts, const Big& order, PFC &pfc):
  SecretSharing( new ShamirAccessPolicy(in_k, nparts), order, pfc)
{
  init();
}


// for backwards compatibility: I need to reproduce the constructor that exists in shamir.h with four arguments. int in_k, vector<int> parts, Big order, Big pfc)
// the new constructor requires shareparts whenever I specify arbitrary participants
// the previous constructor only assumes that all participants have exactly one share, so in fact shareParts would simply have the value 1 in the ith position
// this will be done by adding a new constructor to the ShamirAccessPolicy

ShamirSS::ShamirSS(const int in_k, const vector<int> &parts, const Big& order, PFC &pfc):
  SecretSharing( new ShamirAccessPolicy(in_k, parts), order, pfc)
{
  init();
}

ShamirSS::ShamirSS(const int in_k, const vector<int> &parts, const vector<int> &shareParts, const Big& order, PFC &pfc):
  SecretSharing(new ShamirAccessPolicy(in_k, parts, shareParts), order, pfc)
{
  init();
}


vector<Big> ShamirSS::getDistribRandomness() {
  return poly;
}




Big ShamirSS::reconstruct (const vector<ShareTuple> uniqueShares) {
    DEBUG("============================== RECONSTRUCTION ==============================");  
    // in case of insufficient shares, we return -1 as an error value,
    // since a share is always positive

    vector<ShareTuple> witnessShares;

    if (!i_policy.evaluate(uniqueShares, witnessShares)) return -1;

    DEBUG("Reconstruction: (k)---" << getThreshold());

    vector<int> indices(getThreshold());

    for (unsigned int i=0; i < getThreshold(); i++){
      indices[i] = witnessShares[i].getShareIndex();
    }
 
    Big s = 0;
    Big t;
    Big c;

    //DEBUG("Reconstruction: ");
    DEBUG("MODULO: " << m_order);
    for (unsigned int i = 0; i < getThreshold(); i++){
      c = computeLagrangeCoefficient(indices[i], witnessShares);
      DEBUG("Part: " << indices[i] << " coefficient: " << c << " Share: " << witnessShares[i].getShare());
      t = modmult(witnessShares[i].getShare(),c,m_order);
      DEBUG("Contribution: " << t);
      s = (s + t); 
      DEBUG("Temporary secret: " << s);
    }
    s = ((s + m_order) % m_order);
    DEBUG("Final secret: " << s);
    return s;
  }

unsigned int ShamirSS::getThreshold() const{
  return i_policy.getThreshold();
}

std::vector<ShareTuple> ShamirSS::distribute_random(const Big& s){
  int npoly = poly.size();

  // DEBUG("============================== DISTRIBUTE RANDOM ==============================");  
  // DEBUG("npoly: " << npoly);

  for (int i=0;i<npoly;i++){
    m_pfc.random(poly[i]); // random polynomial coefficient
  }
  //  DEBUG("Prepared randomness, ready to call deterministic algorithm");
  return distribute_determ(s, poly);
}

inline int ShamirSS::getSharePoint(int partIndex, int shareIndex, int totalShares) { // shareIndex is numbered from 0 onwards
  return partIndex * totalShares + shareIndex;
}

std::vector<ShareTuple> ShamirSS::distribute_determ(const Big& s, const vector<Big> &randomness){
  DEBUG("Randomness size: " << randomness.size());
  guard("Secret must be smaller than group order", s < m_order);
  DEBUG("Degree minus 1: " << getThreshold() - 1);
  guard("Distribute algorithm has received randomness of the wrong size", randomness.size() == getThreshold()-1);

  vector<Big> poly(getThreshold());	// internal representation of the shamir polynomial
  int pi; // participant index
  int point; // share public information
  Big acc; // cummulative sum for computing the polynomial
  Big accX; // cummulative value for the variable power (x^i)
  Big temp; // individual term a*x^i
  vector<int> shareParts = i_policy.getSharesByParticipant();

  // the number of a share is computed to be participants_index * nshares + share_number_for_this_participant

  int nshares = i_policy.getNumShares();
  ShareTuple new_share;
  std::vector<ShareTuple> shares; 
  shares.reserve(nshares);  
  vector<int> parts = getParticipants();

  poly[0]=s;
  for (unsigned int i = 1; i < getThreshold(); i++){
    poly[i] = randomness[i-1];
  }
    	
  for (unsigned int j=0;j<getNumParticipants();j++) {   
    pi=parts[j];
    DEBUG("Execution " << j << " / Participant " << pi);
    for (int l = 0; l < shareParts[j]; l++) {
      point = getSharePoint(pi, l, nshares);
      DEBUG("----Share " << l << " / Point " << point);
      acc=poly[0]; accX=point;
      for (unsigned int k=1;k<getThreshold();k++) { 
	// evaluate polynomial a0+a1*x+a2*x^2... for x=pi;
	temp = modmult(poly[k],(Big)accX,m_order); 
	DEBUG("a * x^" << k << ": " << temp);
	acc+=temp;
	accX*=point;
	acc = (acc + m_order) % m_order;
      }    
      // the shares indices do not have to be consecutive and sorted, they only have to be unique. 'point' is unique
      new_share = ShareTuple(pi, acc, point);
      shares.push_back(new_share); 
      DEBUG ("Share " << new_share.to_string());
      //    DEBUG("Share " << j << shares[j]);
    }
  }
  DEBUG("Shares to return size: " << shares.size());

    for (unsigned int i = 0; i < shares.size(); i++) {
      DEBUG("Returning these shares: " << shares[i].to_string());
    }


  return shares;
}

// assumes a good set of shares, that should have been given by evaluate. Does a minimal test
Big ShamirSS::computeLagrangeCoefficient(int shareIndex, vector<ShareTuple> witnessShares) const
{
  if (witnessShares.size() != getThreshold()) return 0; // this is an error value, since this coefficient can never be 0. It could be negative, however

  int j;
  Big z=1;
  Big shareCoef;

  for (unsigned int k=0;k<witnessShares.size();k++)
    {      
      j=witnessShares[k].getShareIndex();
      DEBUG("Reconstruction. Share: " << k << " contents: " << witnessShares[k].to_string());
      guard("Participant index must be greater than 0", j > 0);
      shareCoef = moddiv(m_order - j,(Big)(m_order + shareIndex - j),m_order);
      if (j!= shareIndex) z=modmult(z,shareCoef,m_order);
    }
  return z;
}

