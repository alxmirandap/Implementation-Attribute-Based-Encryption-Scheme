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

#ifndef DEF_KPABE
#include "kpabe.h"
#endif


unsigned int KPABE::numberAttr() const
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
  m_scheme(nullptr), m_pfc(pfc), m_nAttr(nAttr), m_privateKeyRand(0), m_lastCTRandomness(0), m_order(m_pfc.order())
{
  m_privateAttributes.reserve(m_nAttr);
  m_publicAtts.reserve(m_nAttr);
}

KPABE::KPABE(shared_ptr<SecretSharing> scheme, PFC& pfc, int nAttr): // the last argument specifies which group is used to build attribute fragments.
  m_scheme(scheme), m_pfc(pfc), m_nAttr(nAttr), m_privateKeyRand(0), m_lastCTRandomness(0), m_order(m_pfc.order())
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

  for (unsigned int i = 0; i < m_nAttr; i++) {
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
vector<G2> KPABE::genKey()
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::genKey()
#endif
{
  guard("genKey was called with a null scheme", !(m_scheme==0));
  //  SecretSharing ssscheme(policy, m_pfc);
  ENHDEBUG("Inside genKey()");
  DEBUG("privateKeyrand: " << m_privateKeyRand);
  DEBUG("calling distribute_random");
  std::vector<ShareTuple> shares = m_scheme->distribute_random(m_privateKeyRand);
  DEBUG("distribute_random returned");
  
  DEBUG("[GENKEY] Distributed secret: " << m_privateKeyRand);

  return makeKeyFrags(shares);
}

#ifdef AttOnG1_KeyOnG2
vector<G2> KPABE::genKey(vector<Big> randomness)
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::genKey(vector<Big> randomness)
#endif
{
  guard("genKey(randomness) was called with a null scheme", !(m_scheme==0));
  ENHDEBUG("Inside genKey(randomness)");
  // SecretSharing ssscheme(policy, m_pfc);
  std::vector<ShareTuple> shares = m_scheme->distribute_determ(m_privateKeyRand, randomness);
  DEBUG("[keyGen] private randomness used for the key generation: " << m_privateKeyRand);
  return makeKeyFrags(shares);
}

#ifdef AttOnG1_KeyOnG2
vector<G2> KPABE::makeKeyFrags(std::vector<ShareTuple> shares)
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::makeKeyFrags(std::vector<ShareTuple> shares)
#endif
{

  
#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags(shares.size());
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags(shares.size());
#endif
         
  for (unsigned int i = 0; i < shares.size(); i++){
    
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

// attributes are implicitly numbered from 0 to m_nAttr, as these are the indices of the public and private attribute values.
// encryption takes a series of attributes it wants to encrypt to. The indices of these attributes are stored in the vector atts.
// during encryption, we first get att_index to identify the proper index of each attribute frag we want to create.
// then we access the corresponding public element by using this att_index to pick the element at the right position.
#ifdef AttOnG1_KeyOnG2
bool KPABE::encrypt_main_body(const vector<int> &atts, vector<G1>& attFrags, GT& blinder)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::encrypt_main_body(const vector<int> &atts, vector<G2>& attFrags, GT& blinder)
#endif
{
  attFrags.clear();
  //  guard("Attribute fragments should be an empty vector", attFrags.size() == 0);
  attFrags.reserve(atts.size());

  m_pfc.random(m_lastCTRandomness);
  GT preblinder = m_pfc.power(m_pfc.pairing(m_Q,m_P),m_privateKeyRand);
  blinder = m_pfc.power(preblinder, m_lastCTRandomness);

  DEBUG("[ENCRYPT] Private Key Randomness: " << m_privateKeyRand);
  DEBUG("[ENCRYPT] Last CT Randomness    : " << m_lastCTRandomness);
  DEBUG("[ENCRYPT] Full blinder          : " << m_pfc.hash_to_aes_key(blinder));

  for (unsigned int i = 0; i < atts.size(); i++){
    unsigned int att_index = atts[i];
    //    OUT("Attribute " << i << ": " << att_index);
    if (att_index >= m_nAttr) return false; 
    attFrags.push_back( m_pfc.mult(getPublicAttributes()[att_index],m_lastCTRandomness));
#ifdef AttOnG2_KeyOnG1
    m_pfc.precomp_for_pairing(attFrags[i]);  // precomputes on the G2 element
#endif

  }
  guard("Attribute fragments must be as many as attributes", atts.size() == attFrags.size());
  return true;
}

#ifdef AttOnG1_KeyOnG2
bool KPABE::encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G1>& attFrags)
#endif
#ifdef AttOnG2_KeyOnG1
bool KPABE::encryptS(const vector<int> &atts, const Big& M, Big& CT, vector<G2>& attFrags)
#endif
{
  GT blinder;
  bool success = encrypt_main_body(atts, attFrags, blinder);
  if (!success) return false;
  CT=lxor(M,m_pfc.hash_to_aes_key(blinder));
  return true; 
}


#ifdef AttOnG1_KeyOnG2
bool KPABE::encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G1>& attFrags)
#endif
#ifdef AttOnG2_KeyOnG1
bool KPABE::encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G2>& attFrags)
#endif
{
  GT blinder;
  bool success = encrypt_main_body(atts, attFrags, blinder);
  if (!success) return false;
  CT = M * blinder;
  return true;  
}


#ifdef AttOnG1_KeyOnG2
bool KPABE::decrypt_main_body(vector<G2> keyFrags, const vector<int>& atts, vector<G1>& attFrags, GT& blinder)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::decrypt_main_body(vector<G1> keyFrags, const vector<int>& atts, vector<G2>& attFrags, GT& blinder)
#endif
{
  // the first step in decryption is finding which key fragments (keyFrags) are covered by the attribute fragments (attFrags).
  // these are the fragments that have matching participants index. these indices for the attFrags are contained in the atts vector.
  // for the key frags, they are part of the policy which the decryptor must have together with the key. We assume whoever asked for the key and
  // received the frags had to submit a policy that is still around

  // the covered fragments are held in a pair of arrays, attFragIndices and keyFragIndices
  // these vectors hold the indices to the vectors keyFrags and attFrags that correspond to matching fragments
  // each covered fragment in keyFrags will have a corresponding frag in attFrags. 
  // attFragIndices and keyFragIndices hold these indices to these pairs of frags, and so they have to be the same size

  // once we have these pairs in place, we need to find which ones are minimally necessary to recover the secret. That can be done with evaluateIDs
  // to do that, we need to obtain the IDs of the shares in keyFrags. that can only be done by the policy itself. The policy knows the order in which its shares
  // are given away by the distribute algorithm. the keyFragIndices holds points to exactly this order, so the policy can use them to compute the share of IDs of
  // the covered shares. That, therefore, could be done by the same algorithm that analyses covered shares. 

  // after we have those IDs, we run evaluate to recover the indices to the pairs that are relevant. these will be indices into attFragIndices and keyFragIndices
  // finally, we can use these indices to invoke findCoefficient for each relevant share and then prepare the frags for the final multiplication that optimizes 
  // the pairing operations.

  // findCoefficient, once again, does not need to know the shares value. All it requires is the public information of the share, the shareIndex. 
  // but these are now split among two structures, the coveredShareIDs and the witnessSharesIndices vectors. These are then first brought together
  // before being passed to findCoefficient

  vector<int> attFragIndices;
  vector<int> keyFragIndices;
  vector<std::string> coveredShareIDs;

  debugVector("Ciphertext attributes", atts);

  getPolicy()->obtainCoveredFrags(atts, attFragIndices, keyFragIndices, coveredShareIDs); // this computation is independent of the values of fragments themselves

  debugVector("IDs of covered shares", coveredShareIDs);
  debugVector("Indices for att fragments", attFragIndices);
  debugVector("Indices for key fragments", keyFragIndices);


  vector<int> witnessSharesIndices;
  if (!getPolicy()->evaluateIDs(coveredShareIDs, witnessSharesIndices)) return false;

  debugVector("witnessSharesIndices", witnessSharesIndices);

  vector<std::string> minimalShareIDs;
  for (unsigned int i = 0; i < witnessSharesIndices.size(); i++) {
    minimalShareIDs.push_back(coveredShareIDs[witnessSharesIndices[i]]);
  }
  
  //  int countAtts = attFragIndices.size();  

  //  vector<int> minimalParts(countAtts);
//  for (int i = 0; i < countAtts; i++) {    
//    minimalParts[i] = atts[attFragIndices[i]];
//    //    DEBUG("Minimal Part[" << i << "]: " << minimalParts[i]);
//  }
//

  int countAtts = witnessSharesIndices.size();  
  G1 *g1[countAtts];
  G2 *g2[countAtts];

#ifdef AttOnG1_KeyOnG2
  G1 bufferArray[countAtts]; // this is a temporary placeholder so that computed fragments can have an address that can be used by g1 or g2
#endif
#ifdef AttOnG2_KeyOnG1
  G2 bufferArray[countAtts]; // this is a temporary placeholder so that computed fragments can have an address that can be used by g1 or g2
#endif

  vector<Big> coeffs = getPolicy()->findCoefficients(minimalShareIDs, m_order);
  for (int i = 0; i < countAtts; i++) {    
    Big coeff = coeffs[i];
    int keyFragIndex = keyFragIndices[witnessSharesIndices[i]];
    int attFragIndex = attFragIndices[witnessSharesIndices[i]];

#ifdef AttOnG1_KeyOnG2
    bufferArray[i] = m_pfc.mult(attFrags[attFragIndex], coeff); // necessary to fix an address that can be passed to g1.
    g1[i] = &bufferArray[i];
    g2[i] = &keyFrags[keyFragIndex];
#endif
#ifdef AttOnG2_KeyOnG1
    bufferArray[i] = m_pfc.mult(attFrags[attFragIndex], coeff);
    g1[i] = &keyFrags[keyFragIndex]; 
    g2[i] = &bufferArray[i];
#endif
  }

  blinder = m_pfc.multi_pairing(countAtts,g2,g1);

  return true;   

}
  
#ifdef AttOnG1_KeyOnG2
bool KPABE::decryptS(vector<G2> keyFrags, const vector<int>& atts, const Big& CT,  vector<G1>& attFrags, Big& PT)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::decryptS(vector<G1> keyFrags, const vector<int>& atts, const Big& CT,  vector<G2>& attFrags, Big& PT)
#endif
{
  GT blinder;
  bool success = decrypt_main_body(keyFrags, atts, attFrags, blinder);
  if (!success) return false;
  PT=lxor(CT,m_pfc.hash_to_aes_key(blinder));
  //  DEBUG("[DECRYPT] found plaintext : " << PT);

  return true;  
}

#ifdef AttOnG1_KeyOnG2
bool KPABE::decrypt(vector<G2> keyFrags, const vector<int>& atts, const GT& CT,  vector<G1>& attFrags, GT& PT)
#endif
#ifdef AttOnG2_KeyOnG1
  bool KPABE::decrypt(vector<G1> keyFrags, const vector<int>& atts, const GT& CT,  vector<G2>& attFrags, GT& PT)  
#endif
{
  GT blinder;
  bool success = decrypt_main_body(keyFrags, atts, attFrags, blinder);
  if (!success) return false;
  PT = CT / blinder;
  //  DEBUG("[DECRYPT] found plaintext : " << PT);
  return true;  

}
