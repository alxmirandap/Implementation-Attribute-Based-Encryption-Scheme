class SharePair{
  int partIndex;
  Big share;

public:
  SharePair(): partIndex(0), share(0){}

  SharePair(int pi, Big s):partIndex(pi), share(s){}
  
  SharePair(const SharePair& other) : 
    partIndex(other.partIndex),
    share(other.share) 
  {}

  void setValues(int pi, Big s){
    partIndex = pi;
    share = s;
  }

  SharePair& operator=(const SharePair& other){
    partIndex = other.partIndex;
    share = other.share;
  }

  bool operator==(const SharePair& rhs) const {
    if (partIndex != rhs.partIndex) return false;
    if (share != rhs.share) return false;
    return true;
  }

  string to_string() {
    std::stringstream sstrm;
    sstrm << "[" << partIndex << ";" << share << "] ";
    return sstrm.str();
  }

  int getPartIndex(){
    return partIndex;
  }

  Big getShare() {
    return share;
  }
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

  ShamirSS(int in_k, int nparts, const Big& in_order, PFC &pfc, vector<int> parts):
    m_k(in_k), m_nparts(nparts), m_order(in_order), m_pfc(pfc)
  {
    //DEBUG("Called the constructor");  
    guard("ShamirSS constructor has received participants vector of the wrong size", parts.size() == nparts);
    for (int i = 0; i < nparts; i++){
      m_participants.push_back(parts[i]);
    }
  }
    
    Big lagrange(int i,vector<int> parts);
    Big reconstruct (vector<SharePair> shares);
    std::vector<SharePair> distribute_random(const Big& s);
    std::vector<SharePair> distribute_determ(const Big& s, vector<Big> randomness);
};
