
// #define AttOnG1_KeyOnG2
#define AttOnG2_KeyOnG1

class KPABE {
  PFC& m_pfc;
  int m_nAttr;

  Big m_privateKeyRand;
  Big m_lastCTRandomness;
  Big m_order;

  vector<Big> m_privateAttributes;

#ifdef AttOnG1_KeyOnG2
  vector<G1> m_publicAtts;
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> m_publicAtts;
#endif

  G1 m_P;
  G2 m_Q; 
  GT m_publicCTBlinder;


public:

  KPABE(PFC &pfc, int nAttr);
  void paramsgen(const G1& P, const G2& Q, const Big& order);  
  int numberAttr();
  void setup();
  vector<Big>& getPrivateAttributes();
  Big& getPrivateKeyRand();
  Big& getLastEncryptionRandomness();
  GT& getPublicCTBlinder();

#ifdef AttOnG1_KeyOnG2
  vector<G1>& getPublicAttributes();
  vector<G2> genKey(const ShamirAccessPolicy& policy);
  bool encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G1>& AttFrags);
  GT decrypt(const ShamirAccessPolicy& policy, const vector<G2> keyFrags, const vector<int>& atts, const GT& CT, const vector<G1>& AttFrags);
#endif

#ifdef AttOnG2_KeyOnG1
  vector<G2>& getPublicAttributes();
  vector<G1> genKey(const ShamirAccessPolicy& policy);
  bool encrypt(const vector<int> &atts, const GT& M, const GT& CT, const vector<G2>& AttFrags);
  GT decrypt(const ShamirAccessPolicy& policy, const vector<G1> keyFrags, const vector<int>& atts, const GT& CT, const vector<G2>& AttFrags);
#endif
};



