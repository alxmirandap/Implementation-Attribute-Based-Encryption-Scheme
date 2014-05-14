class SharePair{
  int partIndex;
  Big share;

 public:
  SharePair();
  SharePair(const int pi, const Big s); 
  SharePair(const SharePair& other);
  SharePair& operator=(const SharePair& other);
  void setValues(const int pi, const Big s);
  bool operator==(const SharePair& rhs) const;
  string to_string() ;
  int getPartIndex() const;
  Big getShare() const;
};

class ShamirAccessPolicy{
  int m_k;  // threshold
  vector<int> m_participants; // the names of the participants
  
 public:
  ShamirAccessPolicy();
  ShamirAccessPolicy(const int k, const int n); // constructor with participants numbered from 1 to n
  ShamirAccessPolicy(const int k, const vector<int> parts);
  ShamirAccessPolicy(const ShamirAccessPolicy& other);
  ShamirAccessPolicy& operator=(const ShamirAccessPolicy& other);
  int getThreshold();
  int getNumParticipants();
  vector<int> getParticipants();
  int getNumShares(); // returns the number of shares distributed by this policy
};


class ShamirSS 
{
private:
  ShamirAccessPolicy m_policy; // the associated access structure/policy
  Big m_order;	// the order of the base group
  PFC &m_pfc;

public:
  ShamirSS(const int in_k, const vector<int> parts, const Big& in_order, PFC &pfc);  
  ShamirSS(const int in_k, const int nparts, const Big& in_order, PFC &pfc);  
  ShamirSS(const ShamirAccessPolicy policy, const Big& in_order, PFC &pfc);  
  Big lagrange(const int i,const vector<int> parts);
  Big reconstruct (const vector<SharePair> shares);
  int getThreshold();
  int getNumParticipants();
  vector<int> getParticipants();
  std::vector<SharePair> distribute_random(const Big& s);
  std::vector<SharePair> distribute_determ(const Big& s, const vector<Big> randomness);

};

