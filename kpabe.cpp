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

int KPABE::numberAttr()
{
  return nAttr;
}

vector<Big>& KPABE::getPrivateAttributes()
{
  return m_privateAttributes;
}

Big& KPABE::getPrivateKeyRand()
{
  return m_privateKeyRand;
}

vector<G2>& KPABE::getPublicAttributes()
{
  switch (m_attributeGroup)
    {
    case AttOnG1: return m_publicAttsG1;
    case AttOnG2: return m_publicAttsG2;
    }
}

Big& KPABE::getLastEncryptionRandomness()
{
  return m_lastCTRandomness;
}


GT& KPABE::getPublicCTBlinder()
{
  return m_publicCTBlinder;
}

KPABE::KPABE(const PFC& pfc, int nAttr, EnAttGroup attGroup) // the last argument specifies which group is used to build attribute fragments.
{
  m_pfc = pfc;
  m_nAttr = nAttr;

  m_privateAttributes(nAttr);
  m_publicAttsG1(nAttr);
  m_publicAttsG2(nAttr);

  m_privateKeyRand = 0;
  m_lastCTRandomness = 0;
  set_AttributeGroup(attGroup);

  m_order = m_pfc.order();
}

void KPABE::set_AttributeGroup(EnAttGroup attGroup) const{
  switch(attGroup){
  case AttOnG1:
    m_attributeGroup = AttOnG1;
    m_KeyGroup = KeyOnG2;
    break;
  case AttOnG2:
    m_attributeGroup = AttOnG2;
    m_KeyGroup = KeyOnG1;
    break;
  }
}

void KPABE::paramsgen(const G1& P, const G2& Q, const Big& order) 
{
  m_pfc.random(P);
  m_pfc.random(Q);

 switch (m_attributeGroup)
    {
    case AttOnG1: 
      pfc.precomp_for_mult(Q);
      break;
    case AttOnG2: 
      pfc.precomp_for_mult(P);
      break;
    }

  m_P = P;
  m_Q = Q;
  m_publicCTBlinder = m_pfc.pairing(Q,P);
  order = m_order; // sending the value of order to the outside
}

void KPABE::setup(){
  pfc.random(m_lastCTRandomness);
  m_publicCTBlinder = pfc.power(pfc.pairing(Q,P),m_lastCTRandomness);

  for (int i = 0; i < m_nAttr; i++) {
    pfc.random(m_privateAttributes[i]);                 
    switch (m_attributeGroup)
      {
      case AttOnG1: 
	m_publicAttsG1[i]=pfc.mult(P,m_privateAttributes[i]);
	pfc.precomp_for_mult(m_publicAttsG1[i],TRUE);
	break;
      case AttOnG2: pfc.precomp_for_mult(P);
	m_publicAttsG2[i]=pfc.mult(P,m_privateAttributes[i]);
	pfc.precomp_for_mult(m_publicAttsG2[i],TRUE);
	break;
      }
  }
}

vector<G1> KPABE::genKey(const ShamirAccessPolicy& policy)
{
  ShamirSS shamir(policy, m_order, m_pfc);
  std::vector<SharePair> shares = shamir.distribute_random(m_privateKeyRand);

  const SharePair& temp;
  
  std::vector<SharePair> shares = shamir.distribute_random(m_privateKeyRand);

  for (int i = 0; i < shares.size(); i++){
    temp = shares[i];
    
  }

  switch (m_KeyGroup)
    {
    case KeyOnG1: 
      pfc.mult(P,moddiv(temp.getShare(),m_privateAttributes[i],m_order));  // exploits precomputation      
      break;
    case KeyOnG2: pfc.precomp_for_mult(P);
      pfc.mult(Q,moddiv(temp.getShare(),m_privateAttributes[i],m_order));  // exploits precomputation      
      break;
    }


pfc.mult(Q,moddiv(qi,t[i],order));  // exploits precomputation

}

  bool KPABE::encrypt(const vector<int> &atts, const GT& M, const GT& CT, const vector<G2>& AttFrags);
  
  GT KPABE::decrypt(const ShamirAccessPolicy& policy, const vector<int>& atts, const GT& CT, const vector<G2>& AttFrags);
