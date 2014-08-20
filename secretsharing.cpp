/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  This file holds some implementation for the concept of an abstract secret
  sharing scheme. Since not all of it belongs in an abstract class, there is
  some real data imlementation here.

*/

#ifndef DEF_UTILS
#include "utils.h"
#endif

#include "secretsharing.h"


ShareTuple::ShareTuple():
  partIndex(0), share(0), shareIndex("")
{}

ShareTuple::ShareTuple(const int pi, const Big s, const std::string si):
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
  sstrm << "[" << partIndex << ";" << share << ";" << shareIndex << "] ";
  return sstrm.str();
}

int ShareTuple::getPartIndex() const{
  return partIndex;
}

Big ShareTuple::getShare() const {
    return share;
}

std::string ShareTuple::getShareIndex() const{
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

AccessPolicy::AccessPolicy(const unsigned int n)
{
  for (unsigned int i = 0; i < n; i++) {
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

vector<ShareTuple> getUniqueShares(vector<ShareTuple> &shares, unsigned int k)
// returns the first k distinct shares in the vector <shares>
{
  vector<ShareTuple> unique;
  vector<std::string> indices;
  unique.reserve(shares.size());
  indices.reserve(shares.size());
  int n;
  ShareTuple share;
  std::string shareIndex;

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

vector<ShareTuple> getUniqueShares(vector<ShareTuple> &shares)
{
  return getUniqueShares(shares, shares.size());
}


//================================================================

SecretSharing::SecretSharing(shared_ptr<AccessPolicy> policy, const Big &order, PFC &pfc):
  m_policy(policy), m_order(order), m_pfc(pfc)
{}

SecretSharing::~SecretSharing(){
}


Big SecretSharing::getOrder() const{
  return m_order;
}

shared_ptr<AccessPolicy> SecretSharing::getPolicy() const{
  return m_policy;
}

unsigned int SecretSharing::getNumParticipants() const{
  return m_policy->getNumParticipants();
}

unsigned int SecretSharing::getNumShares() {
  return m_policy->getNumShares();
}

vector<int> SecretSharing::getParticipants() const{
  return m_policy->getParticipants();
}

vector<ShareTuple> SecretSharing::getSharesForParticipants(vector<int> &parts, vector<ShareTuple> &shares){
  vector<ShareTuple> outShares;
  outShares.reserve(shares.size());
  int n; // this must be a signed int, otherwise contains always returns a positive value and we can not detect an error.
  ShareTuple share;
  for (unsigned int i = 0; i < shares.size(); i++) {
    share = shares[i];
    n = contains(parts, share.getPartIndex());
    DEBUG("Share Participant: " << share.getPartIndex() << " --- Containment: " << n);
    if ( n >= 0) {
      DEBUG("Adding share");
      outShares.push_back(share);
    }
  }
  return outShares;
}

bool SecretSharing::evaluate(const vector<ShareTuple> uniqueShares, vector<ShareTuple> &witnessShares) const {
  return m_policy->evaluate(uniqueShares, witnessShares);
}

