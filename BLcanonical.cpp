/*
 * BLcanonical.cpp
 *
 *  Created on: 19 Aug 2014
 *      Author: uxah005
 */

#ifndef DEF_BL_CANON
#include "BLcanonical.h"
#endif


void BLAccessPolicy::init(){
  m_minimal_sets = parseFromExpression(0,m_description);
}

BLAccessPolicy::BLAccessPolicy():
  m_description("")
{}

BLAccessPolicy::BLAccessPolicy(const string &description, const int n):
  AccessPolicy(n),
  m_description(description)
{
  init();
}

BLAccessPolicy::BLAccessPolicy(const string &description, const vector<int> &parts):
  AccessPolicy(parts),
  m_description(description)
{
  init();
}

BLAccessPolicy::BLAccessPolicy(const BLAccessPolicy& other):
  AccessPolicy(other.m_participants),
  m_description(other.m_description),
  m_minimal_sets(other.m_minimal_sets)
{}

BLAccessPolicy& BLAccessPolicy::operator=(const BLAccessPolicy& other)
{
  m_description = other.m_description;
  m_participants = other.m_participants;
  m_minimal_sets = other.m_minimal_sets;
  return *this;
}

std::string BLAccessPolicy::getDescription() const
{
  return m_description;
}

vector<vector<int> >& BLAccessPolicy::getMinimalSets() {
  return m_minimal_sets;
}

Big BLAccessPolicy::findCoefficient(std::string id,const vector<std::string> shareIDs) const {
  int n = contains<std::string>(shareIDs, id);
  if (n >= 0) {
    return 1; 
  } else {
    return 0;
  }
}

unsigned int BLAccessPolicy::getNumShares()  {
  unsigned int nshares = 0;
  for (unsigned int i = 0; i < m_minimal_sets.size(); i++)
    {
      nshares += m_minimal_sets[i].size();
    }
  return nshares;
}


bool BLAccessPolicy::satisfyMinimalSet(int setID, vector<int> set, vector<std::string> shareIDs, vector<int> &satisfyingSharesIndices) const{
  //  DEBUG("SatisfyMinimalSet");
  //  DEBUG("shareIDs size " << shareIDs.size());

  //  debugVector(shareIDs, "ShareID");

  bool missedElement = false;
  for (unsigned int i = 0; (!missedElement) && (i < set.size()); i++) {
    int setElem = set[i];
    std::string elemID = convertIntToStr(setID) + ":" + convertIntToStr(setElem);
    bool elemFound = false;

    for (unsigned int j = 0; (!elemFound)  && (j < shareIDs.size()); j++) {
      std::string shareID = shareIDs[j];
      if (elemID == shareID) {
	elemFound = true;
	satisfyingSharesIndices.push_back(j);
	// Found element at position i
      }
    }
    if (!elemFound) {
      missedElement = true;
    }
  }
  if (missedElement) {
    // I have missed some element of the set. Returning false.
    satisfyingSharesIndices.clear();
    return false;
  }
  // I have not missed any element. Returning true."
  return true;
}

std::string BLAccessPolicy::createShareIndex(std::string setID, std::string partID){
  std::string id = setID + ":" + partID;
  return id;
}


bool BLAccessPolicy::evaluateIDs(const vector<std::string> shareIDs, vector<int> &witnessSharesIndices) const{
  witnessSharesIndices.clear();

  vector<int> satisfyingSharesIndices;
  for (unsigned int i = 0; i < m_minimal_sets.size(); i++)
  {
	  vector<int> minimalSet = m_minimal_sets[i]; 
	  if (satisfyMinimalSet(i+1, minimalSet, shareIDs, satisfyingSharesIndices)){
	    addVector(witnessSharesIndices, satisfyingSharesIndices);
	    return true;
	  }
	  satisfyingSharesIndices.clear();
  }
  return false;
}

void BLAccessPolicy::obtainCoveredFrags(const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const {
  int count = 0;
  for (unsigned int i = 0; i < m_minimal_sets.size(); i++) {
    vector<int> minimalSet = m_minimal_sets[i]; 
    for (unsigned int j = 0; j < minimalSet.size(); j++) {
      int att_index = minimalSet[j];
      int n = contains(atts, att_index);
      if (n >= 0) {
	std::string shareID = convertIntToStr(i+1) + ":" + convertIntToStr(att_index);
    	keyFragIndices.push_back(count);
	attFragIndices.push_back(n);
	coveredShareIDs.push_back(shareID);
      }
      count++;
    }
  }
}

std::vector<std::vector<int> > BLAccessPolicy::parseFromExpression(int level, std::string expr) {
// level 0 denotes the top-level of the expression. An OR is expected here
// level 1 denotes the arguments of the OR. These could be either leaves or AND expressions

	vector<vector<int> > minimalSets;
	if (expr == "") {
		return minimalSets;
	}

  try {
    int leafValue = convertStrToInt(expr);
    // If possible, create a leaf node with that value
    // This can happen at any level
    vector<int> minimalSet;
    minimalSet.push_back(leafValue);
    minimalSets.push_back(minimalSet);
    return minimalSets;
  } catch (std::exception &e) {
	  // If not, it must be some operator, depending on the level we are

    // look for string until "("
    size_t start_index = expr.find("(");
    if (start_index == std::string::npos) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse policy: it does not have an operator " << std::endl;
      throw std::runtime_error(ss.str());
    }
    std::string op = expr.substr(0,start_index);

    // check it is the right operator
    bool right_or = ((level == 0) && (op == op_OR));
    bool right_and = ((level == 1) && (op == op_AND));
    if (! (right_or || right_and)) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse BLAccessPolicy: wrong operator for this level: " << op << " level: " << level << std::endl;
      throw std::runtime_error(ss.str());
    }
    // find the whole sub-expression of operator arguments
    size_t end_index = expr.rfind(")");
    if (end_index == std::string::npos) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse policy: it does not have a closing [ ) ]" << std::endl;
      throw std::runtime_error(ss.str());
    }
    if (end_index != expr.length()-1) {
      stringstream ss;
      ss << ": Could not parse policy: there is content beyond last [ ) ]" << std::endl;
      throw std::runtime_error(ss.str());
    }
    //start: 4
    //end: 10
    //string: 5 to 9
    //number chars: 9 - 5 + 1 = 5 = end-1-(start+1)+1 = end-1-start-1+1 = end - start - 1
    std::string sub_expr = expr.substr(start_index+1, end_index - start_index - 1);
    // parse sub-expression into tokens, separated by ","
    vector<std::string> tokens;
    exprTokenize(sub_expr, tokens, ",","(",")");

	// if we are at level 0, each token must construct an AND
	// and so we expect to receive a full minimal set.
	// Because of the signature, this is wrapped
	// inside an external vector, but it is its single element.
    if (level == 0) {
		for (unsigned int i = 0; i < tokens.size(); i++) {
			vector<vector<int> > minimalSet = parseFromExpression(level+1, tokens[i]);
			minimalSets.push_back(minimalSet[0]);
		}
		return minimalSets;
    }
    // if we are at level 1, we are constructing a minimal set for an AND expression
    // each token is a single leaf and will be returned as a single element set
    // inside a vector
    if (level == 1) {
    	vector<int> minimalSet;
    	for (unsigned int i = 0; i < tokens.size(); i++) {
    		vector<vector<int> > literal = parseFromExpression(level+1, tokens[i]);
    		minimalSet.push_back(literal[0][0]);
		}
		minimalSets.push_back(minimalSet);
		return minimalSets;
   	}
    minimalSets.clear();
    return minimalSets;
  }
  minimalSets.clear();
  return minimalSets;
}


//==============================================

void BLSS::initPolicy(){
  i_policy = std::dynamic_pointer_cast<BLAccessPolicy>(m_policy);
  if (!i_policy) {
    stringstream ss(ERR_BAD_POLICY);
    ss << ": BLSS has an AccessPolicy that is not BLAccessPolicy!" << std::endl;
    throw std::runtime_error(ss.str());
  }
}
    
void BLSS::init(){
  initPolicy();
  initRandomness();
}
  
BLSS::BLSS(shared_ptr<BLAccessPolicy>  policy, PFC &pfc):
  SecretSharing(policy, pfc)
{
  init();
}

BLSS::BLSS(shared_ptr<BLAccessPolicy>  policy, const Big &order, PFC &pfc):
  SecretSharing(policy, order, pfc)
{
  init();
}
 
// virtual inherited methods:
vector<Big> BLSS::getDistribRandomness() {  
  return m_randomness;
}

void BLSS::initRandomness() {  
  vector< vector<int> > &minimalSets = i_policy->getMinimalSets();
  for (unsigned int i = 0; i < minimalSets.size(); i++) {
    vector<int> set = minimalSets[i];
    for (unsigned int j = 0; j < set.size()-1; j++) {
      m_randomness.push_back(0);
    }
  }
}


std::vector<ShareTuple> BLSS::distribute_random(const Big& s){
  Big r;

  vector< vector<int> > &minimalSets = i_policy->getMinimalSets();
  
  int count = 0;
  for (unsigned int i = 0; i < minimalSets.size(); i++) {
    vector<int> set = minimalSets[i];
    for (unsigned int j = 0; j < set.size()-1; j++) {
      m_pfc.random(m_randomness[count]);
      count++;
    }
  }
  return distribute_determ(s, m_randomness);
}

std::vector<ShareTuple> BLSS::distribute_determ(const Big& s, const vector<Big>& randomness){
  int count = 0;
  vector<ShareTuple> shares;

  vector< vector<int> > minimalSets = i_policy->getMinimalSets();
  for (unsigned int i = 0; i < minimalSets.size(); i++) {
    vector<int> set = minimalSets[i];
    Big currentSum = 0;
    for (unsigned int j = 0; j < set.size(); j++) {      
      int partIndex = set[j];
      std::string shareID = BLAccessPolicy::createShareIndex(convertIntToStr(i+1), convertIntToStr(partIndex));
      Big value;
      if (j < set.size()-1) {
    	  value = randomness[count];
    	  count++;
    	  currentSum += value % m_order;
      } else {
	value = (s - currentSum + m_order) % m_order;
      }
      ShareTuple share(partIndex, value, shareID);
      shares.push_back(share);

    }    
  }
  return shares;
}


Big BLSS::reconstruct (const vector<ShareTuple> shares){
    vector<ShareTuple> witnessShares;
    if (!i_policy->evaluate(shares, witnessShares)) return -1;

    Big s = 0;
    for (unsigned int i=0; i < witnessShares.size(); i++){
      s = (s + witnessShares[i].getShare()) % m_order ;
    }
    return s;
}



