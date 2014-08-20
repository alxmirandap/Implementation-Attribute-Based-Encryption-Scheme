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

unsigned int BLAccessPolicy::getNumShares()  {
	unsigned int nshares = 0;
	for (unsigned int i = 0; i < m_minimal_sets.size(); i++)
	{
		nshares += m_minimal_sets[i].size();
	}
	return nshares;
}

// this function returns true if the shares received are enough to satisfy the policy. In BL, this does not just have to do with the right participant, it must
// also be the right share. For example, consider the policy (A AND B) OR (A AND C). A receives two shares, and exactly one of them will be useful in any of the
// minimal sets. The share A_1 is not the same as share A_2, and so the share_ID has to be equal to the one in the right position of the tree.
// This work, however, will be done in the satisfyNode function.

bool BLAccessPolicy::satisfyMinimalSet(int setID, vector<int> set, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares) const{
  ENHDEBUG("SatisfyMinimalSet");

  for(unsigned int i = 0; i < set.size(); i++) {
    DEBUG("set[" << i << "]: " << set[i]);
  }

  bool missedElement = false;
  for (unsigned int i = 0; (!missedElement) && (i < set.size()); i++) {
    int setElem = set[i];
    std::string elemID = convertIntToStr(setID) + ":" + convertIntToStr(setElem);
    bool elemFound = false;
    DEBUG("ElemID: " << elemID);

    for (unsigned int j = 0; (!elemFound)  && (j < shares.size()); j++) {
      ShareTuple share = shares[j];
      DEBUG("ShareID[" << j << "]: " << share.getShareIndex());
      if (elemID == share.getShareIndex()) {
	elemFound = true;
	satisfyingShares.push_back(share);
	DEBUG("Found element number " << i);
      }
    }
    if (!elemFound) {
      missedElement = true;
    }
    DEBUG("Checked element at position " << i << ". Found: " << elemFound);
  }
  if (missedElement) {
    ENHDEBUG("I have missed some element of the set. Returning false.");
    satisfyingShares.clear();
    return false;
  }
  DEBUG("I have not missed any element. Returning true.");
  return true;
}

bool BLAccessPolicy::evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const{
  witnessShares.clear();

  ENHDEBUG("Evaluating shares");

  vector<ShareTuple> satisfyingShares;
  for (unsigned int i = 0; i < m_minimal_sets.size(); i++)
  {
	  vector<int> minimalSet = m_minimal_sets[i]; 

	  ENHDEBUG("Checking minimal set");
	  for(unsigned int j = 0; j < minimalSet.size(); j++) {
	    DEBUG("Elem: " << j << ": " << minimalSet[j]);
	  }

	  if (satisfyMinimalSet(i+1, minimalSet, shares, satisfyingShares)){
	    DEBUG("Satisfied!");
	    addVector(witnessShares, satisfyingShares);
	    return true;
	  }
	  ENHDEBUG("Not Satisfied!");
	  satisfyingShares.clear();
  }
  DEBUG("End of evaluate. Returning false.");
  return false;
}


std::vector<std::vector<int>> BLAccessPolicy::parseFromExpression(int level, std::string expr) {
// level 0 denotes the top-level of the expression. An OR is expected here
// level 1 denotes the arguments of the OR. These could be either leaves or AND expressions

	vector<vector<int>> minimalSets;
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
	ENHDEBUG("CATCH in parse");
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
    ENHDEBUG("sub-expression to tokenize: " << sub_expr);
    // parse sub-expression into tokens, separated by ","
    vector<std::string> tokens;
    exprTokenize(sub_expr, tokens, ",","(",")");

	// if we are at level 0, each token must construct an AND
	// and so we expect to receive a full minimal set.
	// Because of the signature, this is wrapped
	// inside an external vector, but it is its single element.
    if (level == 0) {
		for (unsigned int i = 0; i < tokens.size(); i++) {
			vector<vector<int>> minimalSet = parseFromExpression(level+1, tokens[i]);
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
    		vector<vector<int>> literal = parseFromExpression(level+1, tokens[i]);
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

// inline void BLSS::initPolicy(){
//   BLAccessPolicy* tempPtr = dynamic_cast<BLAccessPolicy*> (m_policy);
//   if (tempPtr) {
//     i_policy = *tempPtr;
//   } else {
//     cerr << "BLSecretSharing has an AccessPolicy that is not BLAccessPolicy!";
//     exit(ERR_BAD_POLICY);
//   }
// }

// void BLSS::init(){
//   initPolicy();
// }

// BLSS::BLSS(BLAccessPolicy* policy, const Big &order, PFC &pfc):
//   SecretSharing(policy, order, pfc)
// {
//   init();
// }





