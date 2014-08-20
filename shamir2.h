// This file gives the headers for an implementation of a simple shamir policy already framed as a subclass of an abstract AccessPolicy class

#include "utils.h"
#define SHAMIR2

#ifndef DEF_SECRET_SHARING
#include "secret_sharing.h"
#endif




class ShamirAccessPolicy : public AccessPolicy{
  unsigned int m_threshold;  // threshold
  vector<int> m_shareParts; // this vector indicates how many shares each participant receives

 public:
  ShamirAccessPolicy();
  ShamirAccessPolicy(const int k, const unsigned int n); // constructor with participants numbered from 1 to n, each participant holding one share
  ShamirAccessPolicy(const int k, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  ShamirAccessPolicy(const int k, const vector<int> &parts, const vector<int> &shareParts); // constructor with participants specified freely numbered, and being able to hold several shares. The vector shareParts indicates how many shares each participant has. 
  ShamirAccessPolicy(const ShamirAccessPolicy& other);
  ShamirAccessPolicy& operator=(const ShamirAccessPolicy& other);
  unsigned int getThreshold() const;
  unsigned int getNumShares() const;
  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
  vector<int> getSharesByParticipant() const;
};


class ShamirSS : public SecretSharing
{
 private:
  ShamirAccessPolicy i_policy; // this is the actual implementation of the policy object, with the type that is useful for the derived class
  vector<Big> poly;
  
 protected:
  void init();
  void initPolicy();
  void initPoly();
  int getSharePoint(int partIndex, int shareIndex, int totalShares);

 public:
  ShamirSS(ShamirAccessPolicy* policy, const Big &order, PFC &pfc);  
  ShamirSS(const int in_k, const int nparts, const Big& in_order, PFC &pfc);  
  ShamirSS(const int in_k, const vector<int> &parts, const Big& in_order, PFC &pfc);  
  ShamirSS(const int in_k, const vector<int> &parts, const vector<int> &shareParts, const Big& in_order, PFC &pfc);  
  unsigned int getThreshold() const;

  //computes the lagrange coefficient for a given share in a set
  Big computeLagrangeCoefficient(int shareIndex, vector<ShareTuple> shares) const;

  // virtual inherited methods:
  vector<Big> getDistribRandomness();  
  std::vector<ShareTuple> distribute_random(const Big& s);
  std::vector<ShareTuple> distribute_determ(const Big& s, const vector<Big>& randomness);
  Big reconstruct (const vector<ShareTuple> shares);
};

