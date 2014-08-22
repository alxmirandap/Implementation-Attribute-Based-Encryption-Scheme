#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#ifndef DEF_BL_CANON
#include "BLcanonical.h"
#endif

#define DEF_KPABE

//#define AttOnG1_KeyOnG2
#define AttOnG2_KeyOnG1


class KPABE {
  shared_ptr<SecretSharing> m_scheme;
  shared_ptr<AccessPolicy> m_policy;
  PFC& m_pfc;
  unsigned int m_nAttr;

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
  vector<G2> makeKeyFrags(std::vector<ShareTuple> shares);
  bool encrypt_main_body(const vector<int> &atts, vector<G1>& attFrags, GT& blinder);
  bool decrypt_main_body(vector<G2> keyFrags, const vector<int>& atts, vector<G1>& attFrags, GT& blinder);
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> makeKeyFrags(std::vector<ShareTuple> shares);
  bool encrypt_main_body(const vector<int> &atts, vector<G2>& attFrags, GT& blinder);
  bool decrypt_main_body(vector<G1> keyFrags, const vector<int>& atts, vector<G2>& attFrags, GT& blinder);
#endif



public:

  KPABE(shared_ptr<SecretSharing> scheme, PFC &pfc, int nAttr);
  void paramsgen(G1& P, G2& Q, Big& order);  
  unsigned int numberAttr() const;
  void setup();
  vector<Big>& getPrivateAttributes() ;
  Big& getPrivateKeyRand() ;
  Big& getLastEncryptionRandomness() ;
  GT& getPublicCTBlinder() ;
  shared_ptr<SecretSharing> getScheme() ;

#ifdef AttOnG1_KeyOnG2
  vector<G1>& getPublicAttributes() ;
  vector<G2> genKey();
  vector<G2> genKey(vector<Big> randomness);
  bool encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G1>& attFrags);
  bool encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G1>& attFrags);
  bool decrypt(vector<G2> keyFrags, const vector<int>& atts, const GT& CT,  vector<G1>& attFrags, GT& PT);
  bool decryptS(vector<G2> keyFrags, const vector<int>& atts, const Big& CT,  vector<G1>& attFrags, Big& PT);
#endif

#ifdef AttOnG2_KeyOnG1
  vector<G2>& getPublicAttributes() ;
  vector<G1> genKey();
  vector<G1> genKey(vector<Big> randomness);
  bool encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G2>& attFrags);
  bool encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G2>& attFrags);
  bool decrypt(vector<G1> keyFrags, const vector<int>& atts, const GT& CT,  vector<G2>& attFrags, GT& PT);
  bool decryptS(vector<G1> keyFrags, const vector<int>& atts, const Big& CT,  vector<G2>& attFrags, Big& PT);
#endif
};



