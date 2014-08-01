
class ShareTuple{
  int partIndex;
  Big share;
  int shareIndex;

 public:
  ShareTuple();
  ShareTuple(const int pi, const Big s, const int si); 
  ShareTuple(const ShareTuple& other);
  ShareTuple& operator=(const ShareTuple& other);
  void setValues(const int pi, const Big s, const int si) ;
  bool operator==(const ShareTuple& rhs) const;
  string to_string() const;
  int getPartIndex() const;
  int getShareIndex() const;
  Big getShare() const;
};

//=============================================================================

class AccessPolicy{
 protected:
  vector<int> m_participants; // the names of the participants
  // policy parameters depend on the type of policy, and have to be defined in the base classes. This includes, for example, the association between shares and users
  
 public:
  AccessPolicy();
  AccessPolicy(const int n); // constructor with participants numbered from 1 to n
  AccessPolicy(const vector<int> &parts); // constructor where participants are explicitly numbered. 

  // I removed assignment and copy operators, since these seem better suited for base classes

  unsigned int getNumParticipants() const;
  vector<int> getParticipants() const;
  virtual unsigned int getNumShares() const = 0; // returns the number of shares distributed by this policy

  // evaluate: evaluates the received shares according to the policy and returns a set of shares that are enough to reconstruct the secret if 
  // the policy is satisfied by the first argument
  virtual bool evaluate(const vector<ShareTuple> uniqueShares, vector<ShareTuple> &witnessShares) const = 0; 

};

vector<ShareTuple> getUniqueShares(const vector<ShareTuple> &shares); //outside the class, since it is more of an utility function
vector<ShareTuple> getUniqueShares(const vector<ShareTuple> &shares, unsigned int k);

//=======================================================================

class SecretSharing
{
 protected:
  AccessPolicy* const m_policy; // the associated access structure/policy. I could not make smart pointers compile, so I resorted to raw pointers
  Big m_order;	// the order of the base group
  PFC &m_pfc;   //not sure if this should go in the AccessPolicy as well. 
  		// It definitely makes sense in the secret sharing class, because this needs to make computations. 
  		// The Access Policy may need it to compute evaluate a set of shares, but is that parf of the policy or of the secret sharing? Seems to be policy.
  // However, this evaluation may not require actual compuation, just a formal check that the right shares exist.
  // For this reason, I have removed both the order and pfc from the Policy
		// the particular structures implementing this secret sharing scheme can only be defined in the base classes


public:
  SecretSharing(AccessPolicy* policy, const Big &order, PFC &pfc);  
  Big getOrder() const;
  unsigned int getNumParticipants() const;
  vector<int> getParticipants() const; 
  virtual vector<Big> getDistribRandomness() = 0;  
  virtual std::vector<ShareTuple> distribute_random(const Big& s) = 0;
  virtual std::vector<ShareTuple> distribute_determ(const Big& s, const vector<Big>& randomness) = 0;
  virtual Big reconstruct (const vector<ShareTuple> shares) = 0;

  virtual ~SecretSharing(){
    delete(m_policy);
  }
};

