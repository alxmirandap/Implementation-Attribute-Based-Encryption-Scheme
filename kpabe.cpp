/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file is an implementation of the KPABE_{LSSS} scheme of Crampton, Pinto 2014.

  It should be compiled as a library, and is meant to be used by the testbed.
  For compilation details, see the main file. Here, I will list only the needed instructions for this file.

  Using BN curves for a security level of AES-128. k = 12, for Type 3 pairings.

  Compile with modules as specified below:

  - Compile this file as

  	g++ -O2 -m64 -c -DZZNS=4 kpabe.cpp -lbn -lmiracl -lpairs -o kpabe.o

*/

#include "utils.h"
#include "shamir.h"
#include "kpabe.h"


int KPABE::numberAttr()
{
  return m_nAttr;
}

vector<Big>& KPABE::getPrivateAttributes()
{
  return m_privateAttributes;
}

Big& KPABE::getPrivateKeyRand()
{
  return m_privateKeyRand;
}

#ifdef AttOnG1_KeyOnG2
vector<G1>& KPABE::getPublicAttributes()
{
  return m_publicAtts;
}
#endif
#ifdef AttOnG2_KeyOnG1
vector<G2>& KPABE::getPublicAttributes()
{
  return m_publicAtts;
}
#endif


Big& KPABE::getLastEncryptionRandomness()
{
  return m_lastCTRandomness;
}


GT& KPABE::getPublicCTBlinder()
{
  return m_publicCTBlinder;
}

KPABE::KPABE(PFC& pfc, int nAttr): // the last argument specifies which group is used to build attribute fragments.
  m_pfc(pfc), m_nAttr(nAttr), m_privateKeyRand(0), m_lastCTRandomness(0), m_order(m_pfc.order())
{
  m_privateAttributes.reserve(m_nAttr);
  m_publicAtts.reserve(m_nAttr);
}

void KPABE::paramsgen(G1& P, G2& Q, Big& order)  // all arguments have their values changed on the outside of the function
{
  m_pfc.random(P);
  m_pfc.random(Q);

#ifdef AttOnG1_KeyOnG2
  m_pfc.precomp_for_mult(Q);
#endif
#ifdef AttOnG2_KeyOnG1
  m_pfc.precomp_for_mult(P);
#endif

  m_P = P;
  m_Q = Q;
  m_publicCTBlinder = m_pfc.pairing(Q,P);
  order = m_order; // sending the value of order to the outside
}

void KPABE::setup(){
  m_privateAttributes.clear();
  m_publicAtts.clear();
  m_pfc.random(m_privateKeyRand);
  m_privateKeyRand %= m_order;
  guard("Key Rand must be smaller than group order", m_privateKeyRand < m_order);
  m_publicCTBlinder = m_pfc.power(m_pfc.pairing(m_Q,m_P),m_privateKeyRand);

  for (int i = 0; i < m_nAttr; i++) {
    m_privateAttributes.push_back(0);
    m_pfc.random(m_privateAttributes[i]);

#ifdef AttOnG1_KeyOnG2
    m_publicAtts.push_back(m_pfc.mult(m_P,m_privateAttributes[i]));
    m_pfc.precomp_for_mult(m_publicAtts[i],TRUE);
#endif
#ifdef AttOnG2_KeyOnG1   
    m_publicAtts.push_back(m_pfc.mult(m_Q,m_privateAttributes[i]));
    m_pfc.precomp_for_mult(m_publicAtts[i],TRUE);
#endif
  }
  guard("[SETUP:] Attribute's vector size should be m_nAttr", m_privateAttributes.size() == m_nAttr);
}


#ifdef AttOnG1_KeyOnG2
vector<G2> KPABE::genKey(const ShamirAccessPolicy& policy)
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::genKey(const ShamirAccessPolicy& policy)
#endif
{
  ShamirSS shamir(policy, m_pfc);
  std::vector<SharePair> shares = shamir.distribute_random(m_privateKeyRand);
  
  DEBUG("[GENKEY] Distributed secret: " << m_privateKeyRand);

  return makeKeyFrags(shares);
}

#ifdef AttOnG1_KeyOnG2
vector<G2> KPABE::genKey(const ShamirAccessPolicy& policy, vector<Big> poly)
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::genKey(const ShamirAccessPolicy& policy, vector<Big> poly)
#endif
{
  ShamirSS shamir(policy, m_pfc);
  std::vector<SharePair> shares = shamir.distribute_determ(m_privateKeyRand, poly);
  DEBUG("[keyGen] private randomness used for the key generation: " << m_privateKeyRand);
  return makeKeyFrags(shares);
}

#ifdef AttOnG1_KeyOnG2
vector<G2> KPABE::makeKeyFrags(std::vector<SharePair> shares)
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::makeKeyFrags(std::vector<SharePair> shares)
#endif
{

  
#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags(shares.size());
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags(shares.size());
#endif
         
  for (int i = 0; i < shares.size(); i++){
    
#ifdef AttOnG1_KeyOnG2
    keyFrags[i] = m_pfc.mult(m_Q,moddiv(shares[i].getShare(),m_privateAttributes[shares[i].getPartIndex()],m_order));
    m_pfc.precomp_for_pairing(keyFrags[i]);  // precomputes on the G2 element
#endif
#ifdef AttOnG2_KeyOnG1          
    keyFrags[i] = m_pfc.mult(m_P,moddiv(shares[i].getShare(),m_privateAttributes[shares[i].getPartIndex()],m_order));
#endif

    // DEBUG("Iter: " << i << " Share: " << shares[i].getShare() 
    // 	<< " Attribute: " << m_privateAttributes[shares[i].getPartIndex()]);
  }
  
  return keyFrags;
}

#ifdef AttOnG1_KeyOnG2
bool KPABE::encrypt_main_body(const vector<int> &atts, vector<G1>& AttFrags, GT& blinder)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::encrypt_main_body(const vector<int> &atts, vector<G2>& AttFrags, GT& blinder)
#endif
{
  AttFrags.clear();
  guard("Attribute fragments should be an empty vector", AttFrags.size() == 0);
  AttFrags.reserve(atts.size());

  m_pfc.random(m_lastCTRandomness);
  GT preblinder = m_pfc.power(m_pfc.pairing(m_Q,m_P),m_privateKeyRand);
  blinder = m_pfc.power(preblinder, m_lastCTRandomness);

  DEBUG("[ENCRYPT] Private Key Randomness: " << m_privateKeyRand);
  DEBUG("[ENCRYPT] Last CT Randomness    : " << m_lastCTRandomness);
  DEBUG("[ENCRYPT] Full blinder          : " << m_pfc.hash_to_aes_key(blinder));

  for (int i = 0; i < atts.size(); i++){
    int att_index = atts[i];
    //    OUT("Attribute " << i << ": " << att_index);
    if (att_index >= m_nAttr) return false; 
    AttFrags.push_back( m_pfc.mult(getPublicAttributes()[att_index],m_lastCTRandomness));
#ifdef AttOnG2_KeyOnG1
    m_pfc.precomp_for_pairing(AttFrags[i]);  // precomputes on the G2 element
#endif

  }
  guard("Attribute fragments must be as many as attributes", atts.size() == AttFrags.size());
  return true;
}

#ifdef AttOnG1_KeyOnG2
bool KPABE::encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G1>& AttFrags)
#endif
#ifdef AttOnG2_KeyOnG1
bool KPABE::encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G2>& AttFrags)
#endif
{
  GT blinder;
  bool success = encrypt_main_body(atts, AttFrags, blinder);
  if (!success) return false;
  CT=lxor(M,m_pfc.hash_to_aes_key(blinder));
  return true; 
}


#ifdef AttOnG1_KeyOnG2
bool KPABE::encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G1>& AttFrags)
#endif
#ifdef AttOnG2_KeyOnG1
bool KPABE::encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G2>& AttFrags)
#endif
{
  GT blinder;
  bool success = encrypt_main_body(atts, AttFrags, blinder);
  if (!success) return false;
  CT = M * blinder;
  return true;  
}


#ifdef AttOnG1_KeyOnG2
bool KPABE::decrypt_main_body(const ShamirAccessPolicy& policy, vector<G2> keyFrags, const vector<int>& atts, vector<G1>& AttFrags, GT& blinder)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::decrypt_main_body(const ShamirAccessPolicy& policy, vector<G1> keyFrags, const vector<int>& atts, vector<G2>& AttFrags, GT& blinder)
#endif
{
  vector<int> attFragIndices;
  vector<int> keyFragIndices;

  if (!policy.evaluate(atts, attFragIndices, keyFragIndices)) return false;
  guard("Key and Attribute fragment vectors should be the same size", attFragIndices.size() == keyFragIndices.size());

  int countAtts = attFragIndices.size();  

  vector<int> minimalParts(countAtts);
  for (int i = 0; i < countAtts; i++) {    
    minimalParts[i] = atts[attFragIndices[i]];
    //    DEBUG("Minimal Part[" << i << "]: " << minimalParts[i]);
  }

  G1 *g1[countAtts];
  G2 *g2[countAtts];
  G2 bufferArray[countAtts]; // this is a temporary placeholder so that computed fragments can have an address that can be used by g1 or g2

  // DEBUG("Order      : " << m_order);
  for (int i = 0; i < countAtts; i++) {    
    Big coeff = policy.findCoefficient(minimalParts[i], minimalParts);
    // DEBUG("coefficient: " << coeff);
    //    OUT("Iter " << i << " --- Attribute Index: " << attIndices[i] << " - - Coeff: " << coeff << " --- Set size: " << attIndices.size());

#ifdef AttOnG1_KeyOnG2
    bufferArray[i] = m_pfc.mult(AttFrag[attFragIndices[i]], coeff); // necessary to fix an address that can be passed to g1.
    g1[i] = &bufferArray[i];
    g2[i] = &keyFrags[keyFragIndices[i]];
#endif
#ifdef AttOnG2_KeyOnG1
    bufferArray[i] = m_pfc.mult(AttFrags[attFragIndices[i]], coeff);
    g1[i] = &keyFrags[keyFragIndices[i]]; 
    g2[i] = &bufferArray[i];
#endif
  }

  // for (int i = 0; i < keyFragIndices.size(); i++){
  //   DEBUG("index for key fragments: " << keyFragIndices[i]);
  // }
  // for (int i = 0; i < attFragIndices.size(); i++){
  //   DEBUG("index for attributes fragments: " << attFragIndices[i]);
  // }

  blinder = m_pfc.multi_pairing(countAtts,g2,g1);

  //  DEBUG("[DECRYPT] Full Blinder: " << m_pfc.hash_to_aes_key(combinedPair));

  return true;   

}
  
#ifdef AttOnG1_KeyOnG2
bool KPABE::decryptS(const ShamirAccessPolicy& policy, vector<G2> keyFrags, const vector<int>& atts, const Big& CT,  vector<G1>& AttFrags, Big& PT)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::decryptS(const ShamirAccessPolicy& policy, vector<G1> keyFrags, const vector<int>& atts, const Big& CT,  vector<G2>& AttFrags, Big& PT)
#endif
{
  GT blinder;
  bool success = decrypt_main_body(policy, keyFrags, atts, AttFrags, blinder);
  if (!success) return false;
  PT=lxor(CT,m_pfc.hash_to_aes_key(blinder));
  //  DEBUG("[DECRYPT] found plaintext : " << PT);

  return true;  
}

#ifdef AttOnG1_KeyOnG2
bool KPABE::decrypt(const ShamirAccessPolicy& policy, vector<G2> keyFrags, const vector<int>& atts, const GT& CT,  vector<G1>& AttFrags, GT& PT)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::decrypt(const ShamirAccessPolicy& policy, vector<G1> keyFrags, const vector<int>& atts, const GT& CT,  vector<G2>& AttFrags, GT& PT)  
#endif
{
  GT blinder;
  bool success = decrypt_main_body(policy, keyFrags, atts, AttFrags, blinder);
  if (!success) return false;
  PT = CT / blinder;
  //  DEBUG("[DECRYPT] found plaintext : " << PT);
  return true;  

}
