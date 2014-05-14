
// #define AttOnG1_KeyOnG2
#define AttOnG2_KeyOnG1

class KPABE {
  PFC &m_pfc;
  int m_nAttr;
  vector<Big> m_privateAttributes;
  vector<G1> m_publicAttsG1;
  vector<G2> m_publicAttsG2;
  Big m_privateKeyRand;
  Big m_lastCTRandomness;
  Big m_order;
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
  bool encrypt(const vector<int> &atts, const GT& M, const GT& CT, const vector<G1>& AttFrags);
  GT decrypt(const Vector<G2>& KeyFrags, const vector<int>& atts, const GT& CT, const vector<G1>& AttFrags);
#endif

#ifdef AttOnG2_KeyOnG1
  vector<G2>& getPublicAttributes();
  vector<G1> genKey(const ShamirAccessPolicy& policy);
  bool encrypt(const vector<int> &atts, const GT& M, const GT& CT, const vector<G2>& AttFrags);
  GT decrypt(const Vector<G1>& KeyFrags, const vector<int>& atts, const GT& CT, const vector<G2>& AttFrags);
#endif
};



