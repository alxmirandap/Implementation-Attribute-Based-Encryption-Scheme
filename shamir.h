class SharePair{
  int partIndex;
  Big share;

public:
  SharePair();
  SharePair(int pi, Big s); 
  SharePair(const SharePair& other);
  void setValues(int pi, Big s);
  SharePair& operator=(const SharePair& other);
  bool operator==(const SharePair& rhs) const;
  string to_string();
  int getPartIndex();
  Big getShare();
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
  ShamirSS(int in_k, int nparts, const Big& in_order, PFC &pfc, vector<int> parts);  
  Big lagrange(int i,vector<int> parts);
  Big reconstruct (vector<SharePair> shares);
  std::vector<SharePair> distribute_random(const Big& s);
  std::vector<SharePair> distribute_determ(const Big& s, vector<Big> randomness);
};
