
//#define AttOnG1_KeyOnG2
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

#ifdef AttOnG1_KeyOnG2
  vector<G2> makeKeyFrags(std::vector<SharePair> shares);
  bool encrypt_main_body(const vector<int> &atts, vector<G1>& AttFrags, GT& blinder);
  bool decrypt_main_body(const ShamirAccessPolicy& policy, vector<G2> keyFrags, const vector<int>& atts, vector<G1>& AttFrags, GT& blinder);
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> makeKeyFrags(std::vector<SharePair> shares);
  bool encrypt_main_body(const vector<int> &atts, vector<G2>& AttFrags, GT& blinder);
  bool decrypt_main_body(const ShamirAccessPolicy& policy, vector<G1> keyFrags, const vector<int>& atts, vector<G2>& AttFrags, GT& blinder);
#endif



public:

  KPABE(PFC &pfc, int nAttr);
  void paramsgen(G1& P, G2& Q, Big& order);  
  int numberAttr();
  void setup();
  vector<Big>& getPrivateAttributes();
  Big& getPrivateKeyRand();
  Big& getLastEncryptionRandomness();
  GT& getPublicCTBlinder();

#ifdef AttOnG1_KeyOnG2
  vector<G1>& getPublicAttributes();
  vector<G2> genKey(const ShamirAccessPolicy& policy);
  vector<G2> genKey(const ShamirAccessPolicy& policy, vector<Big> poly);
  bool encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G1>& AttFrags);
  bool encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G1>& AttFrags);
  bool decrypt(const ShamirAccessPolicy& policy, vector<G2> keyFrags, const vector<int>& atts, const GT& CT,  vector<G1>& AttFrags, GT& PT);
  bool decryptS(const ShamirAccessPolicy& policy,  vector<G2> keyFrags, const vector<int>& atts, const Big& CT,  vector<G1>& AttFrags, Big& PT);
#endif

#ifdef AttOnG2_KeyOnG1
  vector<G2>& getPublicAttributes();
  vector<G1> genKey(const ShamirAccessPolicy& policy);
  vector<G1> genKey(const ShamirAccessPolicy& policy, vector<Big> poly);
  bool encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G2>& AttFrags);
  bool encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G2>& AttFrags);
  bool decrypt(const ShamirAccessPolicy& policy, vector<G1> keyFrags, const vector<int>& atts, const GT& CT,  vector<G2>& AttFrags, GT& PT);
  bool decryptS(const ShamirAccessPolicy& policy,  vector<G1> keyFrags, const vector<int>& atts, const Big& CT,  vector<G2>& AttFrags, Big& PT);
#endif
};



