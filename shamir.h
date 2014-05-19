
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
  int m_threshold;  // threshold
  vector<int> m_participants; // the names of the participants
  Big m_order;	// the order of the base group
  
 public:
  ShamirAccessPolicy();
  ShamirAccessPolicy(const int k, const int n, Big order); // constructor with participants numbered from 1 to n
  ShamirAccessPolicy(const int k, const vector<int> parts, Big order);
  ShamirAccessPolicy(const ShamirAccessPolicy& other);
  ShamirAccessPolicy& operator=(const ShamirAccessPolicy& other);
  Big getOrder() const;
  int getThreshold() const;
  int getNumParticipants();
  vector<int> getParticipants();
  int getNumShares(); // returns the number of shares distributed by this policy
  bool evaluate(vector<int> atts, vector<int>& authInts) const;
  Big findCoefficient(int attr, vector<int> attIndices) const;  
};


class ShamirSS 
{
private:
  ShamirAccessPolicy m_policy; // the associated access structure/policy
  PFC &m_pfc;
  vector<Big> poly;
  void initPoly();
  Big m_order;	// the order of the base group

public:
  ShamirSS(const ShamirAccessPolicy policy, PFC &pfc);  
  ShamirSS(const int in_k, const int nparts, const Big& in_order, PFC &pfc);  
  ShamirSS(const int in_k, const vector<int> parts, const Big& in_order, PFC &pfc);  
  Big reconstruct (const vector<SharePair> shares);
  Big getOrder();
  int getThreshold();
  int getNumParticipants();
  vector<int> getParticipants();
  std::vector<SharePair> distribute_random(const Big& s);
  std::vector<SharePair> distribute_determ(const Big& s, const vector<Big>& randomness);
  vector<Big>& getDistribRandomness();  
};

