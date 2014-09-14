/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file implements the classes defined in secretsharing.h
  Since AccessPolicy and SecretSharing are abstract classes, they can not be directly instantiated. 
  Since furthermore ShareTuple is trivially simple, there are not tests for these classes and so there is not a corresponding testfile.
*/

#ifndef DEF_UTILS
#include "utils.h"
#endif

#include "secretsharing.h"


ShareTuple::ShareTuple():
  partIndex(0), share(0), shareID("")
{}

ShareTuple::ShareTuple(const int pi, const Big s, const std::string si):
  partIndex(pi), share(s), shareID(si)
{}

ShareTuple::ShareTuple(const ShareTuple& other) :
  partIndex(other.partIndex),
  share(other.share),
  shareID(other.shareID)
{}

ShareTuple& ShareTuple::operator=(const ShareTuple& other)
{
  partIndex = other.partIndex;
  share = other.share;
  shareID = other.shareID;
  return *this;
}

void ShareTuple::setValues(const int pi, const Big s, const int si)
{
  partIndex = pi;
  share = s;
  shareID = si;
}


bool ShareTuple::operator==(const ShareTuple& rhs) const {
  if (partIndex != rhs.partIndex) return false;
  if (share != rhs.share) return false;
  if (shareID != rhs.shareID) return false;
  return true;
}

string ShareTuple::to_string() const {
  std::stringstream sstrm;
  sstrm << "[" << partIndex << ";" << share << ";" << shareID << "] ";
  return sstrm.str();
}

int ShareTuple::getPartIndex() const{
  return partIndex;
}

Big ShareTuple::getShare() const {
    return share;
}

std::string ShareTuple::getShareID() const{
  return shareID;
}

std::ostream& operator<<(ostream& out, const ShareTuple &sp) {
  return out << "[" << sp.getShareID() << ";" << sp.getPartIndex() << ";" << sp.getShare() << "] ";
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

bool AccessPolicy::evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const{
  witnessShares.clear();
 
  vector<std::string> shareIDs;
  for (unsigned int i = 0; i < shares.size(); i++) {
    shareIDs.push_back(shares[i].getShareID());
  }

  ENHDEBUG("calling evaluateIDs");
  vector<int> witnessSharesIndices;
  bool success = evaluateIDs(shareIDs, witnessSharesIndices);

  ENHDEBUG("returned from evaluateIDs");
  debugVector("evalute: list of indices found", witnessSharesIndices);

  for (unsigned int i = 0; i < witnessSharesIndices.size(); i++) {
    witnessShares.push_back(shares[witnessSharesIndices[i]]);
  }
  return success;
}


//================================================================

SecretSharing::SecretSharing(shared_ptr<AccessPolicy> policy, PFC &pfc):
  m_policy(policy), m_order(pfc.order()), m_pfc(pfc)
{}
SecretSharing::SecretSharing(shared_ptr<AccessPolicy> policy, const Big &order, PFC &pfc):
  m_policy(policy), m_order(order), m_pfc(pfc)
{}

SecretSharing::~SecretSharing(){
}


Big SecretSharing::getOrder() const{
  return m_order;
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
    if ( n >= 0) {
      outShares.push_back(share);
    }
  }
  return outShares;
}

bool SecretSharing::evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const {
  return m_policy->evaluate(shares, witnessShares);
}


