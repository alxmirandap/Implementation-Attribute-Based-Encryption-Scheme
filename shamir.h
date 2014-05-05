class SharePair{
  int partIndex;
  Big share;

public:
  SharePair();
  SharePair(const int pi, const Big s); 
  SharePair(const SharePair& other);
  void setValues(const int pi, const Big s);
  SharePair& operator=(const SharePair& other);
  bool operator==(const SharePair& rhs) const;
  string to_string() ;
  int getPartIndex() const;
  Big getShare() const;
};

class ShamirSS 
{


private:
  int m_k;	// the threshold degree
  int m_nparts;	// the total number of participants
  Big m_order;	// the order of the base group
  PFC &m_pfc;
  std::vector<int> m_participants; // the name of the participants


public:
  ShamirSS(const int in_k, const int nparts, const Big& in_order, PFC &pfc, const vector<int> parts);  
  Big lagrange(const int i,const vector<int> parts);
  Big reconstruct (const vector<SharePair> shares);
  std::vector<SharePair> distribute_random(const Big& s);
  std::vector<SharePair> distribute_determ(const Big& s, const vector<Big> randomness);
};
