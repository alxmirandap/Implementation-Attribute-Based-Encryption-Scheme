/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file holds the declarations for the basic classes implementing the notion of Secret Sharing scheme.
  It declares three classes: 
  - ShareTuple: describes each share individual share of a secret sharing scheme. It has three relevant pieces of information: 
    * the value of the share
    * the participant of the scheme that holds the share
    * a unique identifier for each share within the policy. This identifier must hold all the information necessary to reconstruct the secret from the shares, including all the information associated to the share that must be publicly known.
  - AccessPolicy: it is an abstract class that describes an access policy for a generic secret sharing scheme
  - SecretSharing: also an abstract class, that describes a generic secret sharing scheme. Each such scheme holds exactly one Access Policy that it enforces.
*/


#define DEF_SECRET_SHARING

#ifndef DEF_UTILS
#include "utils.h"
#endif



class ShareTuple{
  int partIndex;
  Big share;
  std::string shareID;

 public:
  ShareTuple();
  ShareTuple(const int pi, const Big s, const std::string si);
  ShareTuple(const ShareTuple& other);
  ShareTuple& operator=(const ShareTuple& other);
  void setValues(const int pi, const Big s, const int si) ;
  bool operator==(const ShareTuple& rhs) const;
  std::string to_string() const;
  int getPartIndex() const;
  std::string getShareID() const;
  Big getShare() const;
};


//=============================================================================

class AccessPolicy{
 protected:
  vector<int> m_participants; // the names of the participants
  // policy parameters depend on the type of policy, and have to be defined in the base classes. This includes, for example, the association between shares and users

 public:
  AccessPolicy();
  AccessPolicy(const unsigned int n); // constructor with participants numbered from 1 to n
  AccessPolicy(const vector<int> &parts); // constructor where participants are explicitly numbered.

  // I removed assignment and copy operators, since these seem better suited for base classes

  unsigned int getNumParticipants() const;
  vector<int> getParticipants() const;
  virtual unsigned int getNumShares() = 0; // returns the number of shares distributed by this policy

  // evaluate: evaluates the received shares according to the policy and returns a set of shares that are enough to reconstruct the secret if
  // the policy is satisfied by the first argument
  virtual vector<Big> findCoefficients(const vector<std::string> shareIDs, const Big& order) const = 0; // every linear secret sharing scheme can produce coefficients for reconstruction
  virtual bool evaluateIDs(const vector<std::string> shareIDs, vector<int> &witnessSharesIndices) const = 0;
  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
  virtual void obtainCoveredFrags(const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const = 0;
  virtual ~AccessPolicy(){};
};

//=======================================================================

class SecretSharing
{
 protected:
  enum RandomnessActions {init, randomize};

  shared_ptr<AccessPolicy> m_policy;
  Big m_order;	// the order of the base group
  PFC &m_pfc;   //not sure if this should go in the AccessPolicy as well.
  		// It definitely makes sense in the secret sharing class, because this needs to make computations.
  		// The Access Policy may need it to compute evaluate a set of shares, but is that parf of the policy or of the secret sharing? Seems to be policy.
  // However, this evaluation may not require actual compuation, just a formal check that the right shares exist.
  // For this reason, I have removed both the order and pfc from the Policy
		// the particular structures implementing this secret sharing scheme can only be defined in the base classes


public:
  SecretSharing(shared_ptr<AccessPolicy> policy, PFC &pfc);
  SecretSharing(shared_ptr<AccessPolicy> policy, const Big &order, PFC &pfc);
  Big getOrder() const;

  inline void setPolicy(shared_ptr<AccessPolicy> policy) {
    m_policy = policy;
  }

  inline shared_ptr<AccessPolicy> getPolicy() {
    return m_policy;
  }

  unsigned int getNumParticipants() const;
  unsigned int getNumShares();
  vector<int> getParticipants() const;
  static vector<ShareTuple> getSharesForParticipants(const vector<int> &parts, const vector<ShareTuple> &shares); // returns the subset of shares that are held by certain participants
  virtual bool evaluate(const vector<ShareTuple> uniqueShares, vector<ShareTuple> &witnessShares) const;
  virtual vector<Big> getDistribRandomness() = 0;
  virtual std::vector<ShareTuple> distribute_random(const Big& s) = 0;
  virtual std::vector<ShareTuple> distribute_determ(const Big& s, const vector<Big>& randomness) = 0;
  virtual Big reconstruct (const vector<ShareTuple> shares) = 0;

  virtual ~SecretSharing();
};

