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

  m_privateAttributes(nAttr);
  m_publicAtts(nAttr);

}

void KPABE::paramsgen(G1& P, G2& Q, const Big& order) 
{
  m_pfc.random(P);
  m_pfc.random(Q);

#ifdef AttOnG1_KeyOnG2
  pfc.precomp_for_mult(Q);
#endif
#ifdef AttOnG2_KeyOnG1
  pfc.precomp_for_mult(P);
#endif

  m_P = P;
  m_Q = Q;
  m_publicCTBlinder = m_pfc.pairing(Q,P);
  order = m_order; // sending the value of order to the outside
}

void KPABE::setup(){
  pfc.random(m_privateKeyRand);
  m_publicCTBlinder = pfc.power(pfc.pairing(Q,P),m_privateKeyRand);

  for (int i = 0; i < m_nAttr; i++) {
    pfc.random(m_privateAttributes[i]);                 

#ifdef AttOnG1_KeyOnG2
    m_publicAttsG1[i]=pfc.mult(P,m_privateAttributes[i]);
    pfc.precomp_for_mult(m_publicAttsG1[i],TRUE);
#endif
#ifdef AttOnG2_KeyOnG1
    m_publicAttsG2[i]=pfc.mult(P,m_privateAttributes[i]);
    pfc.precomp_for_mult(m_publicAttsG2[i],TRUE);
#endif
  }
}


#ifdef AttOnG1_KeyOnG2
vector<G2> KPABE::genKey(const ShamirAccessPolicy& policy)
#endif
#ifdef AttOnG2_KeyOnG1
vector<G1> KPABE::genKey(const ShamirAccessPolicy& policy)
#endif
{
  ShamirSS shamir(policy, m_order, m_pfc);
  std::vector<SharePair> shares = shamir.distribute_random(m_privateKeyRand);

  const SharePair& temp;
  
#ifdef AttOnG1_KeyOnG2
  vector<G2> keyFrags(shares.size());
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G1> keyFrags(shares.size());
#endif
    
    
  std::vector<SharePair> shares = shamir.distribute_random(m_privateKeyRand);
  
  for (int i = 0; i < shares.size(); i++){
    temp = shares[i];
    
#ifdef AttOnG1_KeyOnG2
    keyFrags[i] = pfc.mult(Q,moddiv(temp.getShare(),m_privateAttributes[temp.getPartIndex()],m_order));
#endif
#ifdef AttOnG2_KeyOnG1          
    keyFrags[i] = pfc.mult(P,moddiv(temp.getShare(),m_privateAttributes[temp.getPartIndex()],m_order));
#endif
    pfc.precomp_for_pairing(keyFrags[i]);  // precomputes on the private key   
  }
  
  return keyFrags;
}



#ifdef AttOnG1_KeyOnG2
bool KPABE::encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G1>& AttFrags)
#endif
#ifdef AttOnG2_KeyOnG1
bool KPABE::encrypt(const vector<int> &atts, const GT& M, GT& CT, vector<G2>& AttFrags)
#endif
{

#ifdef AttOnG1_KeyOnG2
  vector<G1> attFrags(atts.size());
#endif
#ifdef AttOnG2_KeyOnG1
  vector<G2> attFrags(atts.size());
#endif

  m_pfc.random(m_lastCTRandomness);
  CT = M * pfc.power(m_publicCTBlinder ,m_lastCTRandomness);


  for (int i = 0; i < atts.size(); i++){
    int att_index = atts[i];
    if (att_index >= m_nattr) return false; 
    attFrags[i] = m_pfc.mult(getPublicAttributes()[att_index],m_lastCTRandomness);
  }
  return true;
  
}

  
#ifdef AttOnG1_KeyOnG2
GT KPABE::decrypt(const ShamirAccessPolicy& policy, const vector<G2> keyFrags, const vector<int>& atts, const GT& CT, const vector<G1>& AttFrags)
#endif
#ifdef AttOnG2_KeyOnG1
  GT KPABE::decrypt(const ShamirAccessPolicy& policy, const vector<G1> keyFrags, const vector<int>& atts, const GT& CT, const vector<G2>& AttFrags)
#endif
{
  GT temp;
  return temp;
}

