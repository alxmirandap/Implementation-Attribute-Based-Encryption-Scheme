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

std::string BLAccessPolicy::getDescription() const
{
  return m_description;
}

unsigned int BLAccessPolicy::getNumShares()  {
  return m_treePolicy->getNumLeaves();
}

// this function returns true if the shares received are enough to satisfy the policy. In BL, this does not just have to do with the right participant, it must
// also be the right share. For example, consider the policy (A AND B) OR (A AND C). A receives two shares, and exactly one of them will be useful in any of the
// minimal sets. The share A_1 is not the same as share A_2, and so the share_ID has to be equal to the one in the right position of the tree. 
// This work, however, will be done in the satisfyNode function.


bool BLAccessPolicy::evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const{
  witnessShares.clear();
  vector<ShareTuple> satisfyingShares;
  bool result = satisfyNode(m_treePolicy, shares, satisfyingShares);
  if (result) {
    addVector(witnessShares, satisfyingShares);
  }
  return result;
}

bool BLAccessPolicy::satisfyNode(shared_ptr<TreeNode> treeNode, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares){ 
  // the logic gets very complicated if I pass the reference
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

  shared_ptr<NodeContent> node = treeNode->getNode();
  if (node->getType() == NodeContentType::nil) {
    DEBUG("Fail: nilNode");
    return false;
  }

  if (node->getType() == NodeContentType::leaf) {
    DEBUG("Checking leaf");
    std::string shareID = node->getNodeID();
    int partID = node->getLeafValue();
    DEBUG("PartID: " << partID);
    DEBUG("ShareID: " << shareID);
    for (unsigned int i = 0; i < shares.size(); i++) {
      ShareTuple share = shares[i];
      DEBUG("tested part id: " << share.getPartIndex());
      DEBUG("tested share id: " << share.getShareIndex());
      if ((shareID == share.getShareIndex()) && (partID == share.getPartIndex())) {
	satisfyingShares.push_back(share);
	return true;
      }
    }
    DEBUG("Fail: wrong leaf id");
    satisfyingShares.clear();
    return false;
  } else {
    switch(node->getInnerNodeType()){
    case InnerNodeType::AND:     
      for (unsigned int i = 0; i < treeNode->getNumChildren(); i++){
	if (!satisfyNode(treeNode->getChild(i), shares, goodShares)) {
	  DEBUG("Fail: AND Node with a non-satisfied child");
	  satisfyingShares.clear();
	  return false;
	} else {
	  addVector(satisfyingShares, goodShares);
	}
      }      
      return true; 
      break;
    case InnerNodeType::OR:
      DEBUG("Or node verification");
      for (unsigned int i = 0; i < treeNode->getNumChildren(); i++){
	DEBUG("Checking child " << i);
	if (satisfyNode(treeNode->getChild(i), shares, goodShares)) {
	  DEBUG("child " << i << " passed");
	  addVector(satisfyingShares, goodShares);
	  return true;
	}
      }
      DEBUG("Fail: OR: all nodes are unsatisfied");
      satisfyingShares.clear();
      return false;
      break;
    case InnerNodeType::THR:
      int nSat = 0;
      int threshold = node->getThreshold();
      unsigned int i = 0;
      while ((nSat < threshold) && (i < treeNode->getNumChildren())){
	if (satisfyNode(treeNode->getChild(i), shares, goodShares)) {
	  nSat++;
	  addVector(satisfyingShares, goodShares);
	}
	i++;
      }
      if (nSat >= threshold) return true;
      DEBUG("Fail: Threshold - insufficient nodes satisfied");
      satisfyingShares.clear();
      return false; break;
      //default: return false;break;
    }
  }
  DEBUG("Fail: Default case - not leaf, not inner node");
  satisfyingShares.clear();
  return false;
}

// MAYBE I SHOULD CHANGE THIS
// to accept participants specified as strings
// and then to have a list of participants and store in the tree their respective indices 
// this involves changing the first part, and being less specific about int conversions. 
// These might not even be needed. All it requires is searching for an operator, which is identified by a "("

shared_ptr<TreeNode> BLAccessPolicy::parsePolicy() {
  return parseTreeFromExpression(m_description);
}

// TreeNode* BLAccessPolicy::parseTreeFromExpressionRaw(std::string expr) {
//   if (expr == "") return 0;
//     int leafValue = convertStrToInt(expr);
//     NodeContent* newNode = NodeContent::makeLeafNodeRaw(leafValue);
//     TreeNode* pTree = new TreeNode(*newNode);
//     //    shared_ptr<TreeNode> pTree = make_shared<TreeNode>(newNode);
//     DEBUG("value of pTree: " << pTree);
//     ENHDEBUG("pTree-tostring: " << pTree->to_string());
//     return pTree;
// }  

shared_ptr<TreeNode> BLAccessPolicy::parseTreeFromExpression(std::string expr) {
  ENHDEBUG("Expression: " << expr);
  // try to parse string as an int.   
  // this code can be changed in the future to something more general, if I decide to change the representation of participants to something else than integers
  if (expr == "") {
    shared_ptr<TreeNode> pTree = make_shared<TreeNode>();
    return pTree;
  }
  try {
    int leafValue = convertStrToInt(expr);
    // If possible, create a leaf node with that value  
    shared_ptr<NodeContent> newNode = NodeContent::makeLeafNode(leafValue);
    shared_ptr<TreeNode> pTree = TreeNode::makeTree(newNode);
    return pTree;
  } catch (std::exception &e) {
    // if not possible, look for string until "("    
    size_t start_index = expr.find("(");
    if (start_index == std::string::npos) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse policy: it is not a literal but does not have [ op( ] " << std::endl;
      throw std::runtime_error(ss.str());
    }
    std::string op = expr.substr(0,start_index);
    
    // check it is an operator
    if (!( (op == op_OR) || (op == op_AND) )) { // In BL, Threshold gates are not allowed
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse BLAccessPolicy: operator not recognized: " << op << std::endl;
      throw std::runtime_error(ss.str());
    }
    // find the whole sub-expression of operator arguments
    size_t end_index = expr.rfind(")");
    if (end_index == std::string::npos) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse policy: it is not a literal but does not have a closing [ ) ]" << std::endl;
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
    // create a node for the correct operator
    shared_ptr<NodeContent> newNode = NodeContent::makeNILNode(); // default initialization
    int arity = tokens.size();
    if (op == op_OR) {
      newNode = NodeContent::makeOrNode(arity);
    }
    if (op == op_AND) {
      newNode = NodeContent::makeAndNode(arity);
    }
    shared_ptr<TreeNode> pTree = TreeNode::makeTree(newNode);
    // obtain the corresponding tree for each token and append it to the children of the tree
    for (unsigned int i = 0; i < tokens.size(); i++) {
      shared_ptr<TreeNode> child = parseTreeFromExpression(tokens[i]);
      pTree->appendTree(child);
    }
    return pTree;
  }
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


