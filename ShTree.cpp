/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file implements a specific Secret Sharing scheme: a tree of Shamir threshold schemes.
  There are two classes implemented here: 
  - ShTreeAccessPolicy is a subclass of the abstract AccessPolicy
  - ShTreeSS is a subclass of the abstract SecretSharing
*/


#include "ShTree.h"

void ShTreeAccessPolicy::init(){
  m_treePolicy = parsePolicy();
} 

ShTreeAccessPolicy::ShTreeAccessPolicy():
  m_description("")
{}

ShTreeAccessPolicy::ShTreeAccessPolicy(const string &description, const int n):
  AccessPolicy(n), 
  m_description(description)
{
  init();
}

ShTreeAccessPolicy::ShTreeAccessPolicy(const string &description, const vector<int> &parts):
  AccessPolicy(parts), 
  m_description(description)
{
  init();
}

ShTreeAccessPolicy::ShTreeAccessPolicy(const ShTreeAccessPolicy& other):
  AccessPolicy(other.m_participants), 
  m_description(other.m_description),
  m_treePolicy(other.m_treePolicy)
{}

ShTreeAccessPolicy& ShTreeAccessPolicy::operator=(const ShTreeAccessPolicy& other)
{
  m_description = other.m_description;
  m_participants = other.m_participants;
  m_treePolicy = other.m_treePolicy;
  return *this;
}

std::string ShTreeAccessPolicy::getDescription() const
{
  return m_description;
}

unsigned int ShTreeAccessPolicy::getNumShares()  {
  return m_treePolicy->getNumLeaves();
}

// this function returns true if the shares received are enough to satisfy the policy. In ShTree, this requires checking the whole tree. All the nodes are Shamir
// gates, and for each, the number of satisfied children must at least equal the threshold


// bool ShTreeAccessPolicy::evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const{
//   witnessShares.clear();
//   vector<ShareTuple> satisfyingShares;
//   bool result = satisfyNode(m_treePolicy, shares, satisfyingShares);
//   if (result) {
//     addVector(witnessShares, satisfyingShares);
//   }
//   return result;
// }

bool ShTreeAccessPolicy::evaluateIDs(const vector<std::string> shareIDs, vector<int> &witnessSharesIndices) const{
  witnessSharesIndices.clear();
  //  ENHDEBUG("Tree: " << m_treePolicy->to_string());
  
  bool success = satisfyNodeID(m_treePolicy, shareIDs, witnessSharesIndices);

  return success;
}

bool ShTreeAccessPolicy::satisfyNode(shared_ptr<TreeNode> treeNode, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares){ 
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



  satisfyingShares.clear();

  DEBUG("Creating ID vector");
  
  vector<std::string> shareIDs;
  for (unsigned int i = 0; i < shares.size(); i++) {
    shareIDs.push_back(shares[i].getShareID());
  }

  DEBUG("Calling evaluateIDs");
  vector<int> satisfyingSharesIndices;
  bool success = satisfyNodeID(treeNode, shareIDs, satisfyingSharesIndices);

  for (unsigned int i = 0; i < satisfyingSharesIndices.size(); i++) {
    satisfyingShares.push_back(shares[satisfyingSharesIndices[i]]);
  }

  return success;
}


bool ShTreeAccessPolicy::satisfyNodeID(shared_ptr<TreeNode> treeNode, vector<std::string> shareIDs, vector<int> &satisfyingSharesIndices){ 

  ENHDEBUG("Satisfy Node ID");

  debugVector("Received ShareIDs: ", shareIDs);

  vector<int> goodShares;
  satisfyingSharesIndices.clear();

  shared_ptr<NodeContent> node = treeNode->getNode();
  if (node->getType() == NodeContentType::nil) {
    //    DEBUG("Fail: nilNode");
    return false;
  }

  if (node->getType() == NodeContentType::leaf) {
    DEBUG("Checking leaf");
    std::string shareID = treeNode->getNodeID();
    DEBUG("Tree ShareID: " << shareID);
    for (unsigned int i = 0; i < shareIDs.size(); i++) {
      std::string currentID = shareIDs[i];
      if (shareID == currentID)  {
	satisfyingSharesIndices.push_back(i);
	DEBUG("Leaf satisfied");
	return true;
      }
    }
    //    DEBUG("Fail: wrong leaf id");
    DEBUG("Leaf not satisfied");
    satisfyingSharesIndices.clear();
    return false;
  } else {
    switch(node->getInnerNodeType()){
    case InnerNodeType::AND:     
      DEBUG("AND node");
      for (unsigned int i = 0; i < treeNode->getNumChildren(); i++){
	if (!satisfyNodeID(treeNode->getChild(i), shareIDs, goodShares)) {
	  //	  DEBUG("Fail: AND Node with a non-satisfied child");
	  DEBUG("AND not satisfied");
	  satisfyingSharesIndices.clear();
	  return false;
	} else {
	  DEBUG("AND satisfied");
	  addVector(satisfyingSharesIndices, goodShares);
	}
      }      
      return true; 
      break;
    case InnerNodeType::OR:
      DEBUG("OR node");
      //      DEBUG("Or node verification");
      for (unsigned int i = 0; i < treeNode->getNumChildren(); i++){
	//	DEBUG("Checking child " << i);
	if (satisfyNodeID(treeNode->getChild(i), shareIDs, goodShares)) {
	  //	  DEBUG("child " << i << " passed");
	  DEBUG("OR satisfied");
	  addVector(satisfyingSharesIndices, goodShares);
	  return true;
	}
      }
      //      DEBUG("Fail: OR: all nodes are unsatisfied");
      DEBUG("OR not satisfied");
      satisfyingSharesIndices.clear();
      return false;
      break;
    case InnerNodeType::THR:
      DEBUG("THR node");
      int nSat = 0;
      int threshold = node->getThreshold();
      unsigned int i = 0;
      while ((nSat < threshold) && (i < treeNode->getNumChildren())){
	ENHDEBUG("Threshold node. Checking child: " << i);
	if (satisfyNodeID(treeNode->getChild(i), shareIDs, goodShares)) {	  
	  //	  DEBUG("Satisfied");
	  nSat++;
	  addVector(satisfyingSharesIndices, goodShares);
	} else {
	  //	  DEBUG("not Satisfied");
	}
	i++;
      }
      if (nSat >= threshold) {
	DEBUG("Threshold satisfied");
	return true;
      }
      DEBUG("Fail: Threshold - insufficient nodes satisfied");
      satisfyingSharesIndices.clear();
      DEBUG("Threshold not satisfied");
      return false; break;
      //default: return false;break;
    }
  }
  //  DEBUG("Fail: Default case - not leaf, not inner node");
  satisfyingSharesIndices.clear();
  return false;
}


shared_ptr<TreeNode> ShTreeAccessPolicy::parsePolicy() {
  return parseTreeFromExpression(m_description);
}

shared_ptr<TreeNode> ShTreeAccessPolicy::parseTreeFromExpression(std::string expr) {
  //  DEBUG("Parse Tree From Expression");
  //  ENHDEBUG("Expression: " << expr);
  // try to parse string as an int.   
  // this code can be changed in the future to something more general, if I decide to change the representation of participants to something else than integers
  if (expr == "") {
    shared_ptr<TreeNode> pTree = make_shared<TreeNode>();
    return pTree;
  }
  try {
    int leafValue = convertStrToInt(expr);
    //    DEBUG("Create Leaf node: " << leafValue);
    // If possible, create a leaf node with that value  
    shared_ptr<NodeContent> newNode = NodeContent::makeLeafNode(leafValue);
    shared_ptr<TreeNode> pTree = TreeNode::makeTree(newNode);
    //    DEBUG("Leaf ID: " << pTree->getNodeID());
    return pTree;
  } catch (std::exception &e) {
    // if not possible, look for string until "("    
    size_t start_index = expr.find("(");
    if (start_index == std::string::npos) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse policy: it is not a literal but does not have [ op( ] " << "(" << expr << ")" << std::endl;
      throw std::runtime_error(ss.str());
    }
    std::string op = expr.substr(0,start_index);
    
    // check it is an operator
    if (!( (op == op_OR) || (op == op_AND) || (op == op_THR))) { 
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse ShTreeAccessPolicy: operator not recognized: " << op  << "(" << expr << ")" << std::endl;
      throw std::runtime_error(ss.str());
    }
    // find the whole sub-expression of operator arguments
    size_t end_index = expr.rfind(")");
    if (end_index == std::string::npos) {
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse policy: it is not a literal but does not have a closing [ ) ]" << "(" << expr << ")" << std::endl;
      throw std::runtime_error(ss.str());
    }
    if (end_index != expr.length()-1) {
      stringstream ss;
      ss << ": Could not parse policy: there is content beyond last [ ) ]" << "(" << expr << ")" << std::endl;
      throw std::runtime_error(ss.str());
    }
    //start: 4
    //end: 10
    //string: 5 to 9
    //number chars: 9 - 5 + 1 = 5 = end-1-(start+1)+1 = end-1-start-1+1 = end - start - 1
    std::string sub_expr = expr.substr(start_index+1, end_index - start_index - 1);
    //ENHDEBUG("sub-expression to tokenize: " << sub_expr);
    // parse sub-expression into tokens, separated by ","
    vector<std::string> tokens;
    exprTokenize(sub_expr, tokens, ",","(",")");
    // create a node for the correct operator
    shared_ptr<NodeContent> newNode = NodeContent::makeNILNode(); // default initialization
    int arity = tokens.size();
    if (op == op_OR) {
      newNode = NodeContent::makeThreshNode(arity,1);
    }
    if (op == op_AND) {
      newNode = NodeContent::makeThreshNode(arity, arity);
    }
    if (op == op_THR) {
      if (tokens.size() < 2) {
	stringstream ss(ERR_BAD_POLICY);
	ss << ": Threshold gate with missing arguments" << std::endl;
	throw std::runtime_error(ss.str());
      }
      int threshold = convertStrToInt(tokens[0]); // throws exception if token is not a number
      newNode = NodeContent::makeThreshNode(arity - 1, threshold); // first element in the argument list is the threshold
    }    
    shared_ptr<TreeNode> pTree = TreeNode::makeTree(newNode);
    //    DEBUG("create inner node: " << pTree->to_string());
    //    DEBUG("nodeID: " << pTree->getNodeID());
    // obtain the corresponding tree for each token and append it to the children of the tree

    if (op != op_THR) {
      for (unsigned int i = 0; i < tokens.size(); i++) {
	shared_ptr<TreeNode> child = parseTreeFromExpression(tokens[i]);
	//	DEBUG("Adding new child. Current version: " << pTree->to_string() << " -- child: " << child->to_string());
	pTree->appendTree(child);
	//	DEBUG("Added new child. Current version: " << pTree->to_string());
      }
    } else {
      for (unsigned int i = 1; i < tokens.size(); i++) {
	shared_ptr<TreeNode> child = parseTreeFromExpression(tokens[i]);
	//	DEBUG("Adding new child. Current version: " << pTree->to_string() << " -- child: " << child->to_string());
	pTree->appendTree(child);
	//	DEBUG("Added new child. Current version: " << pTree->to_string());
      }
    }
    return pTree;
  }
}

void ShTreeAccessPolicy::obtainCoveredFrags(const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const {
  int count = 0; // needed, because count is passed as reference to the next function
  obtainCoveredFragsRec(count, m_treePolicy, atts, attFragIndices, keyFragIndices, coveredShareIDs);
}

void ShTreeAccessPolicy::obtainCoveredFragsRec(int &count, shared_ptr<TreeNode> tree, const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const {
//  attFragIndices.clear();
//  keyFragIndices.clear();
//  coveredShareIDs.clear();

  shared_ptr<NodeContent> node = tree->getNode();
  if (node->getType() == NodeContentType::nil) {  
    return;
  }

  if (node->getType() == NodeContentType::leaf) {
    int att_index = node->getLeafValue();
    std::string shareID = tree->getNodeID();
    int n = contains(atts, att_index);
    if (n >= 0) {
      keyFragIndices.push_back(count);
      attFragIndices.push_back(n);
      coveredShareIDs.push_back(shareID);
    }
    count++;
  } else {
    for (unsigned int i = 0; i < tree->getNumChildren(); i++){
	obtainCoveredFragsRec(count, tree->getChild(i), atts, attFragIndices, keyFragIndices, coveredShareIDs);
    }      
  }
}

shared_ptr<TreeNode>& ShTreeAccessPolicy::getPolicy(){
  return m_treePolicy;
}

bool ShTreeAccessPolicy::extractPrefixAndNoFromID(std::string& shareID, std::string& prefix, int& childNo) {
  int n = shareID.find(":");
  if (n == -1) return false;
  int m = shareID.find(":", n+1);
  if (m == -1) return false;
  int p = shareID.find(":=");
  if (p < m) return false;
  prefix = shareID.substr(0, n+1);
  childNo = convertStrToInt(shareID.substr(n+1, m-n-1));
  shareID = shareID.substr(n+1);
  return true;
}

void ShTreeAccessPolicy::storeSharePrefixes(std::map<std::string, vector<int> >& setChildNos, std::string& shareID) {
  std::string ID = shareID;
  std::string prefix;
  std::string oldprefix = "";
  int childNo;
  std::map<std::string, vector<int> >::iterator it;
  while (extractPrefixAndNoFromID(ID, prefix, childNo)) {
    prefix = oldprefix + prefix;
    it = setChildNos.find(prefix);
    if (it == setChildNos.end()) { // prefix does not exist in map
      vector<int> vec;
      vec.push_back(childNo);     
      setChildNos[prefix] = vec;
    } else { // see if childNo is already part of the prefix's vector. If not, add it
      vector<int> vec = it->second;
      ENHDEBUG("current vector in " + prefix);
      debugVector("vec", vec);
      DEBUG("ChildNo: " << childNo);
      int n = contains(vec, childNo);
      if (n == -1) {
	DEBUG("Adding a new element: " << childNo);
	vec.push_back(childNo);
      } else {
	DEBUG("No change in vector: ");
      }
      setChildNos[prefix] = vec;
      ENHDEBUG("current vector in " + prefix);
      debugVector("vec", vec);
    }      
    oldprefix = prefix;
  }  
}

Big ShTreeAccessPolicy::findFinalCoefficient(const std::string& shareID,  std::map<std::string,  vector<int> >& setChildNos, 
				   std::map<std::string, vector<Big> >& setCoeffs, const Big& order) {
  Big acc = 1;
  std::string tempID = shareID;
  std::string prefix;
  int childNo;
  std::string oldprefix = "";
  ENHDEBUG("findFinalCoefficient. shareID: " << shareID);
  while (extractPrefixAndNoFromID(tempID, prefix, childNo)) {
    prefix = oldprefix + prefix;
    ENHDEBUG("findFinalCoefficient. Prefix: " << prefix);
    ENHDEBUG("findFinalCoefficient. tempID: " << tempID);
    ENHDEBUG("findFinalCoefficient. childNo: " << childNo);
    vector<int> childNos = setChildNos[prefix];
    vector<Big> coeffs = setCoeffs[prefix];
    debugVector("childNos", childNos);
    debugVector("coeffs", coeffs);

    int n = contains(childNos, childNo);
    DEBUG("n: " << n);
    guard("findFinalCoefficient: all prefixes found should be in map. " + prefix + "[" + prefix + "]", n >= 0);
    Big coeff = coeffs[n];
    DEBUG("coeff to multiply: " << coeff);
    acc = modmult(acc, coeff, order);
    oldprefix = prefix;
  }
  return acc;
}

/*
Simple approach:
- place all shares in the map, organized by prefix. Instead of storing the share, store its child number. 
- everytime a new prefix is stored, also create an entry for its predecessors that are not yet in the map, with the position it occupies in it.

Example shares:

0:1:0:=2
0:1:1:=3
0:1:2:=4

0:2:1:=2

0:2:2:1:=4

This leads to the following map:

0:1:0:=2:
	"0:1" ---> {0}
	"0" ---> {1}
0:1:1:=3
	"0:1" ---> {0,1}
	"0" ---> {1} (no change)
0:1:2:=4
	"0:1" ---> {0,1,2}
	"0" ---> {1} (no change)
0:2:1:=2
	"0:1" ---> {0,1,2} (no change)
	"0" ---> {1,2} 
	"0:2" ---> {1}
0:2:2:1:=4
	"0:2:2" ---> {1}
	"0:1" ---> {0,1,2} (no change)
	"0" ---> {1,2} (no change)
	"0:2" ---> {1,2}

Now, we can traverse the map and compute the coefficients as we go.
For future expansion, it is better to create a different map that has prefixes as keys and vectors of coefficients as values. For each prefix, we compute the corresponding vector of coefficients and store it in the new map.
Finally, we look at each share and proceed from the beginning, each time extracting a prefix and its position in it to obtain a coefficient, then multiplying that with an accumulator.

Example:

map of coefficients:

"0" ---> [c1_1, c1_2]
"0:1" ---> [c2_1, c2_2, c2_3]
"0:2" ---> [c3_1, c3_2]
"0:2:2" ---> [c4_1]

0:2:2:1:=4

Prefix "0" ---> {1,2} / [c1_1, c1_2] 
Position 2 ---> find index (1) in set, retrieve coefficient (1): c1_2

Prefix "0:2" ---> {1,2} / [c3_1, c3_2]
Position 2 ---> find index (1) in set, retrieve coefficient (1): c3_2

Prefix "0:2:2" ---> {1} / [c4_1]
Position 1 ---> find index (0) in set, retrieve coefficient (1): c4_1

*/
vector<Big> ShTreeAccessPolicy::findCoefficients(const vector<std::string> shareIDs, const Big& order) const {
  vector<Big> coeffs;

  std::map<std::string, vector<int> > setChildNos;
  std::map<std::string, vector<Big> > setCoeffs;

  for (unsigned int i = 0; i < shareIDs.size(); i++) {
    std::string shareID = shareIDs[i];
    ShTreeAccessPolicy::storeSharePrefixes(setChildNos, shareID);
  }

  Big coeff;
  vector<Big> vec_coeffs;
  for (std::map<std::string, vector<int> >::iterator it=setChildNos.begin(); !(it==setChildNos.end()); ++it) {
    vector<int> childNos = it->second;
    DEBUG("computing lagrange coefficients for childNos");
    debugVector("childNos", childNos);
    vec_coeffs.clear();
    for (unsigned int i = 0; i < childNos.size(); i++) {
      coeff = ShTreeAccessPolicy::computeLagrangeCoefficientChildNos(i, childNos, order);
      DEBUG("lagrange coeff for " << i << ": " << coeff);
      vec_coeffs.push_back(coeff);
    }
    guard("coefficients vector should have the same size as childNos", vec_coeffs.size() == childNos.size());
    setCoeffs[it->first] = vec_coeffs;
  }
  ENHDEBUG("===================================");

  ENHDEBUG("findCoefficients");
  debugVector("shareIDs", shareIDs);
  DEBUG("...");


  ENHDEBUG("findCoefficients: contents of maps");
  for (std::map<std::string, vector<int> >::iterator it=setChildNos.begin(); !(it==setChildNos.end()); ++it) {
    DEBUG("childNos: " << it->first);
  }
  for (std::map<std::string, vector<Big> >::iterator it=setCoeffs.begin(); !(it==setCoeffs.end()); ++it) {
    DEBUG("coeffs: " << it->first);
  }
  for (unsigned int i = 0; i < shareIDs.size(); i++) {
    std::string shareID = shareIDs[i];
    Big coeff = ShTreeAccessPolicy::findFinalCoefficient(shareID, setChildNos, setCoeffs, order);
    coeffs.push_back(coeff);
  }

  ENHDEBUG("Returning coeffs");
  debugVector("coeffs to return", coeffs);
  ENHDEBUG("Printed coeffs to return");
  return coeffs;
}

int ShTreeAccessPolicy::extractChildNoFromID(std::string& shareID) {
  int n0 = shareID.rfind(":=");
  int n1 = shareID.rfind(":", n0-1);
  if (n1 == -1) {
    stringstream ss(ERR_BAD_SHARE);
    ss << "Bad share received. It is a root share: " + shareID;
    throw std::runtime_error(ss.str());	  
  }
  int index = convertStrToInt(shareID.substr(n1+1, n0-n1-1));
  return index;
}

Big ShTreeAccessPolicy::computeLagrangeCoefficient(unsigned int shareIndex, vector<ShareTuple>& witnessShares, const Big& order) {
  vector<int> childNos;
  for (unsigned int i = 0; i < witnessShares.size(); i++) {
    std::string shareID = witnessShares[i].getShareID();
    int index = extractChildNoFromID(shareID);
    childNos.push_back(index);
  }
  return computeLagrangeCoefficientChildNos(shareIndex, childNos, order);
}

Big ShTreeAccessPolicy::computeLagrangeCoefficientChildNos(unsigned int shareIndex, vector<int>& witnessChildNos, const Big& order) {
  Big z=1;
  Big shareCoef;

  ENHDEBUG("Lagrange coefficient");
  debugVector("childNos", witnessChildNos);
  int childNo = witnessChildNos[shareIndex];
  int index = ShTreeAccessPolicy::extractPublicInfoFromChildNo(childNo);
  DEBUG("anchor share: " << index);
  guard("Participant index must be positive", index > 0);

  for (unsigned int k=0;k<witnessChildNos.size();k++) {      
    if (k == shareIndex) continue;
    int tempChildNo =  witnessChildNos[k];
    int tempIndex = ShTreeAccessPolicy::extractPublicInfoFromChildNo(tempChildNo);
    
    DEBUG("varying share: " << tempIndex);
    
    shareCoef = moddiv(order - tempIndex,(Big)(order + index - tempIndex),order);
    DEBUG("Multiplying term: " << shareCoef);
    z=modmult(z,shareCoef,order);
  }
  return z;
}

//==============================================

void ShTreeSS::initPolicy(){
  i_policy = std::dynamic_pointer_cast<ShTreeAccessPolicy>(m_policy);
  if (!i_policy) {
    stringstream ss(ERR_BAD_POLICY);
    ss << ": ShTreeSS has an AccessPolicy that is not ShTreeAccessPolicy!" << std::endl;
    throw std::runtime_error(ss.str());
  }
}
    
void ShTreeSS::init(){
  initPolicy();
  manageRandomness(RandomnessActions::init);
}
  
ShTreeSS::ShTreeSS(shared_ptr<ShTreeAccessPolicy>  policy, PFC &pfc):
  SecretSharing(policy, pfc)
{
  init();
}

ShTreeSS::ShTreeSS(shared_ptr<ShTreeAccessPolicy>  policy, const Big &order, PFC &pfc):
  SecretSharing(policy, order, pfc)
{
  init();
}

void ShTreeSS::manageRandomness(RandomnessActions action, shared_ptr<TreeNode> root, int &count) {  
  //  ENHDEBUG("inside manageRandomness.");

//   if (action == RandomnessActions::init) {
//     DEBUG("Operation: init");
//   }
// 
//   if (action == RandomnessActions::randomize) {
//     DEBUG("Operation: randomize");
//     ENHDEBUG("randomness size: " << m_randomness.size());
//   }

  shared_ptr<NodeContent> node = root->getNode();
  
  if (root->isNil() || root->isLeaf()) {
    return;
  }

  if (node->getInnerNodeType() != InnerNodeType::THR) {
    stringstream ss(ERR_BAD_TREE);  
    ss << ": ShTreeSS: ManageRandomness algorithm received a gate different from Threshold: " << node->to_string();
    throw std::runtime_error(ss.str());
  }

  unsigned int arity = node->getArity();
  unsigned int threshold = node->getThreshold();

  for (unsigned int i = 0; i < threshold-1; i++){
    if (action == RandomnessActions::init) {
      m_randomness.push_back(0);
    } else if (action == RandomnessActions::randomize){
      m_pfc.random(m_randomness[count]);
      count++;
    }
  }

  for (unsigned int j=0;j<arity;j++) {     
    shared_ptr<TreeNode> child = root->getChild(j);
    if (child->isLeaf() || child->isNil()) {
      continue;
    }
    
    if (child->isInner()) {
      DEBUG("calling manageRandomness for child");
      manageRandomness(action, child, count);
      continue;
    }
  }
//   DEBUG("manageRandomness over");
//   ENHDEBUG("m_randomness count: " << count);
//   ENHDEBUG("m_randomness size: " << m_randomness.size());
}

void ShTreeSS::manageRandomness(RandomnessActions action) {  
  int count = 0;
  manageRandomness(action, i_policy->getPolicy(), count);
}

  // Need to review this after I start implementing distrib algorithms. Those will dictate the best way to use randomness
  /*
  int count = 0;
  vector< vector<int> > &minimalSets = i_policy->getMinimalSets();
  for (unsigned int i = 0; i < minimalSets.size(); i++) {
    vector<int> set = minimalSets[i];
    for (unsigned int j = 0; j < set.size()-1; j++) {
      if (action == RandomnessActions::init) {
	m_randomness.push_back(0);
      } else if (action == RandomnessActions::randomize){
	m_pfc.random(m_randomness[count]);
	count++;
      }
    }
  }
  */


// virtual inherited methods:
vector<Big> ShTreeSS::getDistribRandomness() {  
  return m_randomness;
}

std::vector<ShareTuple> ShTreeSS::distribute_random(const Big& s){
  ENHDEBUG("Inside distribute_random");
  DEBUG("Calling manageRandomness to randomize");
  manageRandomness(RandomnessActions::randomize);
  DEBUG("Calling distribute determ");
  return distribute_determ(s, m_randomness);
}


std::vector<ShareTuple> ShTreeSS::distribute_determ(shared_ptr<TreeNode> root, const Big& s, const vector<Big>& randomness, int &count){
  ENHDEBUG("Inside distribute_determ");
  vector<ShareTuple> shares;

  shared_ptr<NodeContent> node = root->getNode();

  DEBUG("got node");

  if (root->isNil()) {
    return shares;
  }

  if (root->isLeaf()) {
    ShareTuple newShare(node->getLeafValue(), s, root->getNodeID());
    shares.push_back(newShare);
    return shares;
  }

  if (node->getInnerNodeType() != InnerNodeType::THR) {
    stringstream ss(ERR_BAD_TREE);  
    ss << ": ShTreeSS: Distribution algorithm received a gate different from Threshold: " << node->to_string();
    throw std::runtime_error(ss.str());
  }

  DEBUG("getting arity and threshold");
  int arity = node->getArity();
  unsigned int threshold = node->getThreshold();

  Big acc; // cummulative sum for computing the polynomial
  Big accX; // cummulative value for the variable power (x^i)
  Big temp; // individual term a*x^i
  int point;

  vector<Big> poly(threshold);
  poly[0]=s;
  unsigned int base = count;
  for (unsigned int i = 1; i < threshold; i++){
    unsigned int index = base + i - 1;
    guard("tried to access poly vector out of bounds", i < poly.size());
    guard("tried to access randomness vector out of bounds", index < randomness.size());
    poly[i] = randomness[index];
    count++;
  }

//   debugVector("Poly", poly);
//   debugVector("Randomness vector", randomness);
//   DEBUG("threshold: " << threshold);
//   DEBUG("arity: " << arity);
//   DEBUG("order: " << m_order);

  for (int j=0;j<arity;j++) {   
    //    DEBUG("child: " << j);
    point = j+1; // take notice: we should not have a point 0, because that would reveal the secret immediately as the share. 
    // Reconstruction has to remember to include this +1 in its calculations
    acc=poly[0]; accX=point;
    DEBUG("computing powers for point: " << point);
    for (unsigned int k=1;k<threshold;k++) { 
      // evaluate polynomial a0+a1*x+a2*x^2... for x=point;
      //      DEBUG("making mult");
      temp = modmult(poly[k],(Big)accX,m_order); 
      DEBUG("a" << k << ": " << poly[k]);
      DEBUG("x^" << k << ": " << accX);
      DEBUG("a" << k << " * x^" << k << ": " << temp);
      //      DEBUG("adding term");
      acc = (acc + temp + m_order) % m_order;
      accX = modmult((Big) accX, point, m_order);
    }
    DEBUG("final acc: " << acc);
    

    shared_ptr<TreeNode> child = root->getChild(j);
    if (child->isNil()) {
      continue;
    }

    if (child->isLeaf()) {
      ShareTuple newShare(child->getNode()->getLeafValue(), acc, child->getNodeID());
      shares.push_back(newShare);
      continue;
    }
    
    if (child->isInner()) {
      vector<ShareTuple> childShares = distribute_determ(child, acc, randomness, count);
      addVector(shares, childShares);
      continue;
    }
  }
  ENHDEBUG("returning shares");
  return shares;
}
  

std::vector<ShareTuple> ShTreeSS::distribute_determ(const Big& s, const vector<Big>& randomness){

  // each node in the policy tree is a threshold node. distribution works by computing a share of the secret for each child of that node
  // then, if the child is not a leaf, take its share as the new secret and repeat the process
  // each distribution requires some share public information, that is simply going to be the index of the respective child for that tree
  // this is therefore independent of the participant's value
  // this applies normally even to the case where a participant receives several different shares
  int count = 0;
  vector<ShareTuple> shares = distribute_determ(i_policy->getPolicy(), s, randomness, count);
  return shares;
}

         
void ShTreeSS::updateSet(std::map<std::string, vector<ShareTuple> >& setShares, const std::string& prefix, ShareTuple& share) { 
  vector<ShareTuple> set = setShares.find(prefix)->second;
  set.push_back(share);
  setShares[prefix] = set; 
  // I have to include the previous line because the map has copies of vectors, not references for them. 
  // So, after I add a new element to the vector, this is a different copy from the one in the map, which was not updated.
  // I have to copy the new vector in the map
  // The reason I don't have reference to vectors in the map is because such references would be created in addNewSet, by a temporary variable, and then point at
  // deleted memory. Since I don't want static functions to do memory management, I preferred to take the performance hit with copying
}

      
void ShTreeSS::addNewSet(std::map<std::string, vector<ShareTuple> >& setShares, const std::string& prefix, ShareTuple& share) { 
  vector<ShareTuple> set;
  set.push_back(share);
  setShares[prefix] = set;
}

void ShTreeSS::putShareInSet(std::map<std::string, vector<ShareTuple> >& setShares, const std::string& prefix, ShareTuple& share) { 
  std::map<std::string, vector<ShareTuple> >::iterator it;
  it = setShares.find(prefix);
  if (it == setShares.end()) {
    addNewSet(setShares, prefix, share);
  } else {
    it->second.push_back(share);
  }
}

std::string ShTreeSS::getSetPrefix(std::string& shareID) {
  int n1 = shareID.rfind(":=");
  int n2 = shareID.rfind(":", n1-1);
  std::string result;  
  if (n2 == -1) {
    return "";
  } else {
    return shareID.substr(0, n2);
  }
}

// int ShTreeSS::extractPublicInfoFromID(std::string& shareID) {
//   return extractChildNoFromID(shareID) + 1;
// }


ShareTuple ShTreeSS::detailedReconstruction(vector<ShareTuple>& minimalShares, std::string& prefix, const Big& order){
  Big sum = 0;
  for (unsigned int i = 0; i < minimalShares.size(); i++) {
    Big coeff = ShTreeAccessPolicy::computeLagrangeCoefficient(i, minimalShares, order);
    Big term = modmult(minimalShares[i].getShare(),coeff,order);
    sum = (sum + term); 
  }
  sum = ((sum + order) % order);

  std::string newShareID = TreeNode::findIDForNode(getSetPrefix(prefix), ShTreeAccessPolicy::extractChildNoFromID(prefix), NodeContentType::leaf, "?");
  ShareTuple newShare(0, sum, newShareID);
  return newShare;
}

  ShareTuple ShTreeSS::solveSet(std::map<std::string, vector<ShareTuple> >& setShares, std::string& prefix, const Big& order) { 
  std::map<std::string, vector<ShareTuple> >::iterator it;
  it = setShares.find(prefix);
  vector<ShareTuple> minimalShares = it->second;      
  setShares.erase(it);
  ShareTuple share = detailedReconstruction(minimalShares, prefix, order);
  return share;
}

ShareTuple ShTreeSS::reduceMapShares(std::string& prefix, const std::string& target, 
			   std::map<std::string, vector<ShareTuple> >& setShares, const Big& order){
  ShareTuple computedShare;
  DEBUG("Prefix: " << prefix);
  DEBUG("Target: " << target);
  DEBUG("setShares size: " << setShares.size());
  
  while ( (prefix != target) && (setShares.size() > 0)) {
    DEBUG("Inside While. Current Prefix: " << prefix << " -- target: " << target);
    computedShare = ShTreeSS::solveSet(setShares, prefix, order);
    DEBUG("computedShare: " << computedShare.to_string());
    prefix = ShTreeSS::getSetPrefix(prefix);
    if (prefix != "0") {
    // still more work to do, so keep this share and loop again
      ShTreeSS::putShareInSet(setShares, prefix, computedShare);
    } else {
      // we can't go above "0". We must get out.
      break;
    }
  }

  if (prefix != target) {
    stringstream ss(ERR_BAD_TREE);
    DEBUG("prefix: " << prefix);
    DEBUG("target: " << target);
    ss << ": Could not finish reconstruction. Reached the end of shares but did not finish computation." << std::endl;
    throw std::runtime_error(ss.str());	      
  }

  if (setShares.size() > 0) {
    stringstream ss(ERR_BAD_TREE);
    DEBUG("prefix: " << prefix);
    DEBUG("target: " << target);
    ss << ": Could not finish reconstruction. Reached the top level, but still had more shares." << std::endl;
    throw std::runtime_error(ss.str());	      
  }

  return computedShare;
}


  // a vector of shares will have shares at many different levels.
  // deepest shares have longer prefixes, but all of them can be traced to root
  // the plan is to find a set of shares at the lowest level, reduce them to find a share at the level above and replace those shares by the new share
  // repeat until there is only one share left

  // since we obtain our witnessShares vector from a controlled source (the evaluate function) we know the structure of the vector and we can use that to make
  // shortcuts in the computation

  // Example:
  // 0:0:=
  // 0:1:0:=
  // 0:1:1:0:=
  // 0:1:1:1:0:=
  // 0:1:1:1:1:=
  // 0:1:1:1:2:=
  // 0:1:1:2:...

  // when the new prefix is not a suffix of the current one, a part of the tree is solvable.
  // then, we can gather the vector for that prefix, solve it and insert the new resulting share in the map
  // if the new resulting share's prefix is also not a prefix of the current one, we can solve that prefix and so on, until we reach a point where these prefixes will be equal
  // then we proceed until we have no more shares

ShareTuple ShTreeSS::reduceLowestShares(const vector<ShareTuple>& shares, Big order) {
  DEBUG("Inside Reduce");
  debugVectorObj("shares in reduce", shares);
  bool finished = false;
  ShareTuple computedShare;
  std::map<std::string, vector<ShareTuple> > setShares;
  
  std::string currentSetPrefix = "";
  for (unsigned int i = 0; i < shares.size(); i++) {
    DEBUG("Iteration: " << i);
    ShareTuple share = shares[i];
    std::string shareID = share.getShareID();
    std::string setPrefix = ShTreeSS::getSetPrefix(shareID);

    DEBUG("Before main IF: " << currentSetPrefix << " -- setPrefix: " << setPrefix);
    if (currentSetPrefix == setPrefix) {  
      DEBUG("calling updateSet");
      // a new share in a set that has been previously found
      ShTreeSS::updateSet(setShares, setPrefix, share);
    } else if (isSuffix(currentSetPrefix, setPrefix)) { // if SetPrefix is a suffix of currentSetPrefix
      DEBUG("calling addNewSet");
      // a new set, that will contribute to solve the previous one
      ShTreeSS::addNewSet(setShares, setPrefix, share);
      currentSetPrefix = setPrefix;            
    } else {
      // a new set in a different subtree. Sets that come before can be solved until their prefix matches the new one
      DEBUG("default: entering while");
      computedShare = reduceMapShares(currentSetPrefix, setPrefix, setShares, order);
      if (currentSetPrefix == "0") {
	// should have found the share. Check other conditions that should be true
	if ( (setShares.size() != 1) || (i < shares.size()-1) ) {
	  stringstream ss(ERR_BAD_TREE);
	  ss << ": Could not finish reconstruction. Reached root share, but there are still unprocessed shares." << std::endl;
	  throw std::runtime_error(ss.str());	  
	}
	finished = true;
	break;
      }
    }
  }
  if (!finished) {
    // there's no more shares, setShares is complete and all nodes should be solvable
    computedShare = reduceMapShares(currentSetPrefix, "0", setShares, order);
  } 
  return computedShare;
}


Big ShTreeSS::reconstruct_old(const vector<ShareTuple> shares){
    vector<ShareTuple> witnessShares;
    DEBUG("CALLING EVALUATE");
    if (!i_policy->evaluate(shares, witnessShares)) return -1;
    ENHDEBUG("FINISHED EVALUATE");

    if (witnessShares.size() == 1) {
      ENHDEBUG("Shortcut returning");
      return witnessShares[0].getShare();
    }

    ShareTuple share = reduceLowestShares(shares, m_order);
    ENHDEBUG("FINISHED REDUCE");
    return share.getShare();
    
}


Big ShTreeSS::reconstruct(const vector<ShareTuple> shares){
    vector<ShareTuple> witnessShares;
    DEBUG("CALLING EVALUATE");
    if (!i_policy->evaluate(shares, witnessShares)) return -1;
    ENHDEBUG("FINISHED EVALUATE");

    if (witnessShares.size() == 1) {
      ENHDEBUG("Shortcut returning");
      return witnessShares[0].getShare();
    }
//     DEBUG("CALLING REDUCE");
//     debugVectorObj("shares", shares);
//     debugVectorObj("witnessShares", witnessShares);
//     DEBUG("====");
    vector<std::string> shareIDs;
    for (unsigned int i = 0; i < shares.size(); i++) {
      //      DEBUG("adding shareID: " << shares[i].getShareID());
      shareIDs.push_back(shares[i].getShareID());
    }
    //    debugVector("shareIDs", shareIDs);

    vector<Big> coeffs = i_policy->findCoefficients(shareIDs, getOrder());
    
    ENHDEBUG("Reconstruct:");
    debugVector("coeffs", coeffs);
    ENHDEBUG("Coeffs printed");
    Big sum = 0;
    for (unsigned int i = 0; i < shares.size(); i++) {
      ENHDEBUG("i " << i);
      Big term = modmult(coeffs[i], shares[i].getShare(), m_order);
      sum = (sum + term) % m_order;
    }
    return sum;
    
}

