/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds some implementation for the concept of an abstract secret
  sharing scheme. Since not all of it belongs in an abstract class, there is
  some real data imlementation here.

*/

#include "utils.h"
#include "secret_sharing.h"


ShareTuple::ShareTuple(): 
  partIndex(0), share(0), shareIndex(0)
{}

ShareTuple::ShareTuple(const int pi, const Big s, const int si):
  partIndex(pi), share(s), shareIndex(si)
{} 

ShareTuple::ShareTuple(const ShareTuple& other) : 
  partIndex(other.partIndex),
  share(other.share),
  shareIndex(other.shareIndex)
{}

ShareTuple& ShareTuple::operator=(const ShareTuple& other)
{  
  partIndex = other.partIndex;
  share = other.share;
  shareIndex = other.shareIndex;
  return *this;
}

void ShareTuple::setValues(const int pi, const Big s, const int si) 
{
  partIndex = pi;
  share = s;
  shareIndex = si;
}


bool ShareTuple::operator==(const ShareTuple& rhs) const {
  if (partIndex != rhs.partIndex) return false;
  if (share != rhs.share) return false;
  if (shareIndex != rhs.shareIndex) return false;
  return true;
}

string ShareTuple::to_string() const {
  std::stringstream sstrm;
  sstrm << "[" << shareIndex << ";" << partIndex << ";" << share << "] ";
  return sstrm.str();
}

int ShareTuple::getPartIndex() const{
  return partIndex;
}

Big ShareTuple::getShare() const {
    return share;
}

int ShareTuple::getShareIndex() const{
  return shareIndex;
}

std::ostream& operator<<(ostream& out, const ShareTuple &sp) {
  return out << "[" << sp.getShareIndex() << ";" << sp.getPartIndex() << ";" << sp.getShare() << "] ";
}

//==================================================================

AccessPolicy::AccessPolicy()
{
  m_participants.push_back(1);
}

AccessPolicy::AccessPolicy(const int n)
{
  for (int i = 0; i < n; i++) {
    m_participants.push_back(i+1);
  }
}

AccessPolicy::AccessPolicy(const vector<int> &parts):
  m_participants(parts)
{}

unsigned int AccessPolicy::getNumParticipants() const
{
  return m_participants.size();
}

vector<int> AccessPolicy::getParticipants() const
{
  return m_participants;
}

vector<ShareTuple> getUniqueShares(vector<ShareTuple> &shares) 
{
  return getUniqueShares(shares, shares.size());
}

vector<ShareTuple> getUniqueShares(vector<ShareTuple> &shares, unsigned int k) 
// returns the first k distinct shares in the vector <shares>
{
  vector<ShareTuple> unique;
  vector<int> indices;
  unique.reserve(shares.size());
  indices.reserve(shares.size());
  int n;
  ShareTuple share;
  int shareIndex;
  
  unsigned int i = 0;
  while ( (i<k) && (i < shares.size())) {
    share = shares[i];
    shareIndex = share.getShareIndex();
    n = contains(indices, shareIndex);
    if (n < 0){
      indices.push_back(shareIndex);
      unique.push_back(share);
      k++;
    }
    i++;
  }
  return unique;
}



//================================================================

SecretSharing::SecretSharing(AccessPolicy* policy, const Big &order, PFC &pfc):
  m_policy(policy), m_order(order), m_pfc(pfc)
{}

Big SecretSharing::getOrder() const{
  return m_order;
}

unsigned int SecretSharing::getNumParticipants() const{
  return m_policy->getNumParticipants();
}

vector<int> SecretSharing::getParticipants() const{
  return m_policy->getParticipants();
}

