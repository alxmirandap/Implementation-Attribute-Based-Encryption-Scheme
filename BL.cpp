#include "BL.h"

void BLAccessPolicy::init(){
  m_treePolicy = parsePolicy();
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
  m_treePolicy(other.m_treePolicy)
{}

BLAccessPolicy& BLAccessPolicy::operator=(const BLAccessPolicy& other)
{
  m_description = other.m_description;
  m_participants = other.m_participants;
  m_treePolicy = other.m_treePolicy;
  return *this;
}

unsigned int BLAccessPolicy::getDescription() const
{
  return m_description;
}

unsigned int BLAccessPolicy::getNumShares() const{
  TreeNode parseTree = parsePolicy;
  return parseTree.getNumLeaves();
}


bool BLAccessPolicy::evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const{
  witnessShares.clear();
  vector<ShareTuple> satisfyingShares;
  bool result = satisfyNode(treeNode, shares, satisfyingShares);
  if (result) {
    addVector(witnessShares, satisfyingShares);
  }
  return result;
}

bool satisfyNode(TreeNode node, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares) const { // the logic gets very complicated if I pass the reference
  // to witness shares here. During the analysis of the tree, some nodes may be satisfied at some point and then be insufficient to satisfy an ancestor
  // that means that their shares should not be present in the final witnessShares, and so the decision of including shares in the witnessShares vector
  // should be taken only at the top. Therefore, satisfyNode will return a copy of those shares that are acceptable so far, and these will be copied into
  // witnessShares by evaluate.
  // The logic then, goes like this:
  // If a node is a leaf and it is satisfied, then the corresponding share is returned
  // If a node is an inner node, then whether it accepts depends on its type. One or more than one subtrees may need to be accepted, and so satisfyNode is
  // invoked for each child until such a decision can be made. Each time it is invoked, it gives a new vector to be filled with the satisfying shares for this node.
  // This vector collects the shares of all satisfying subtrees and returns that for the next level if it is satisfied, and returns an empty vector if not.

  
  vector<ShareTuple> goodShares;
  satisfyingShares.clear();

  NodeContent node = treeNode.getNode();
  if (node.getType() == NodeContentType::leaf) {
    int shareID = node.getLeafID();    
    for (unsigned int i = 0; i < shares.size(); i++) {
      ShareTuple share = shares[i];
      if (shareID == share.getShareIndex()) {
	satisfyingShares.push_back(share);
	return true;
      }
    }
    return false;
  } else {
    vector<int> children = node.getChildren();
    switch(node.m_innerNode){
    case InnerNodeType::AND:
      
      for (unsigned int i = 0; i < children.size(); i++){
	if (!satisfyNode(children[i], shares, goodShares)) {
	  satisfyingShares.clear();
	  return false;
	} else {
	  addVector(satisfyingShares, goodShares);
	}
      }      
      return true; 
      break;
    case InnerNodeType::OR:
      for (unsigned int i = 0; i < children.size(); i++){
	if (satisfyNode(children[i], shares, goodShares)) {
	  addVector(satisfyingShares, goodShares);
	  return true;
	}
      }
      return false;
      break;
    case InnerNodeType::THR:
      int nSat = 0;
      int threshold = node.getThreshold();
      unsigned int i = 0;
      while ((nSat < threshold) && (i < children.size())){
	if (satisfyNode(children[i], shares, goodShares)) {
	  nSat++;
	  addVector(satisfyingShares, goodShares);
	}
	i++;
      }
      if (nSat >= threshold) return true;
      satisfyingShares.clear();
      return false;
    default: return false;
    }
  }
}

// MAYBE I SHOULD CHANGE THIS
// to accept participants specified as strings
// and then to have a list of participants and store in the tree their respective indices 
// this involves changing the first part, and being less specific about int conversions. 
// These might not even be needed. All it requires is searching for an operator, which is identified by a "("

TreeNode BLAccessPolicy::parsePolicy() {
  if (expr == "") return TreeNode();
  return parseTreeFromExpression(m_description);
}

TreeNode BLAccessPolicy::parseTreeFromExpression(std::string expr) {
  // try to parse string as an int.   
  // this code can be changed in the future to something more general, if I decide to change the representation of participants to something else than integers
  try {
    int leafValue = convertStrToInt(expr);
    // If possible, create a leaf node with that value  
    NodeContent newNode = NodeContent::makeLeafNode(leafValue);
    return TreeNode(newNode);
  } catch (std::exception &e) {
    // if not possible, look for string until "("    
    size_t start_index = expr.find("(");
    if (start_index == std::string::npos) {
      cerr << "Could not parse policy: it is not a literal but does not have [ op( ] ";
      throw std::runtime_error(ERR_BAD_POLICY);
    }
    std::string op = expr.substr(0,start_index);
    
    // check it is an operator
    if (!( (op == op_OR) || (op == op_AND) )) { // In BL, Threshold gates are not allowed
      cerr << "Could not parse BLAccessPolicy: operator not recognized";
      throw std::runtime_error(ERR_BAD_POLICY);      
    }
    // find the whole sub-expression of operator arguments
    size_t end_index = expr.rfind(")");
    if (end_index == std::string::npos) {
      cerr << "Could not parse policy: it is not a literal but does not have a closing [ ) ]";
      throw std::runtime_error(ERR_BAD_POLICY);
    }
    if (end_index != expr.length()-1) {
      cerr << "Could not parse policy: there is content beyond last [ ) ]";
      throw std::runtime_error(ERR_BAD_POLICY);
    }
    //start: 4
    //end: 10
    //string: 5 to 9
    //number chars: 9 - 5 + 1 = 5 = end-1-start+1+1 = end-start+1
    std::sub_expr = expr.substr(start_index+1, end_index - start_index + 1);
    // parse sub-expression into tokens, separated by ","
    vector<std::string> tokens;
    exprTokenize(sub_expr, tokens, ",","(",")");
    // create a node for the correct operator
    NodeContent newNode;
    TreeNode newTree;
    int arity = tokens.size();
    if (op == op_OR) {
      newNode = makeOrNode(arity);
    }
    if (op == op_AND) {
      newNode = makeAndNode(arity);
    }
    newTree = TreeNode(newNode);
    // obtain the corresponding tree for each token and append it to the children of the tree
    for (unsigned int i = 0; i < tokens.size(); i++) {
      TreeNode child = parseTreeFromExpression(tokens[i]);
      newTree.appendChild(child);
    }
    return newTree;
  }
}

//==============================================

inline void BLSS::initPolicy(){
  BLAccessPolicy* tempPtr = dynamic_cast<BLAccessPolicy*> (m_policy);
  if (tempPtr) {
    i_policy = *tempPtr;
  } else {
    cerr << "BLSecretSharing has an AccessPolicy that is not BLAccessPolicy!";
    exit(ERR_BAD_POLICY);
  }  
}

void BLSS::init(){
  initPolicy();
}

BLSS::BLSS(BLAccessPolicy* policy, const Big &order, PFC &pfc):
  SecretSharing(policy, order, pfc)
{
  init();
}


