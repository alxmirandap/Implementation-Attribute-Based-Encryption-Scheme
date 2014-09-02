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
  //  REPORT("Tree: " << m_treePolicy->to_string());
  
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

  /*  
  vector<ShareTuple> goodShares;
  satisfyingShares.clear();

  shared_ptr<NodeContent> node = treeNode->getNode();
  if (node->getType() == NodeContentType::nil) {
    //    DEBUG("Fail: nilNode");
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
    //    DEBUG("Fail: wrong leaf id");
    satisfyingShares.clear();
    return false;
  } else {
    switch(node->getInnerNodeType()){
    case InnerNodeType::AND:     
      for (unsigned int i = 0; i < treeNode->getNumChildren(); i++){
	if (!satisfyNode(treeNode->getChild(i), shares, goodShares)) {
	  //	  DEBUG("Fail: AND Node with a non-satisfied child");
	  satisfyingShares.clear();
	  return false;
	} else {
	  addVector(satisfyingShares, goodShares);
	}
      }      
      return true; 
      break;
    case InnerNodeType::OR:
      //      DEBUG("Or node verification");
      for (unsigned int i = 0; i < treeNode->getNumChildren(); i++){
	//	DEBUG("Checking child " << i);
	if (satisfyNode(treeNode->getChild(i), shares, goodShares)) {
	  //	  DEBUG("child " << i << " passed");
	  addVector(satisfyingShares, goodShares);
	  return true;
	}
      }
      //      DEBUG("Fail: OR: all nodes are unsatisfied");
      satisfyingShares.clear();
      return false;
      break;
    case InnerNodeType::THR:
      int nSat = 0;
      int threshold = node->getThreshold();
      unsigned int i = 0;
      while ((nSat < threshold) && (i < treeNode->getNumChildren())){
	ENHDEBUG("Threshold node. Checking child: " << i);
	if (satisfyNode(treeNode->getChild(i), shares, goodShares)) {	  
	  DEBUG("Satisfied");
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
  //  DEBUG("Fail: Default case - not leaf, not inner node");
  satisfyingShares.clear();
  return false;

*/

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


// MAYBE I SHOULD CHANGE THIS
// to accept participants specified as strings
// and then to have a list of participants and store in the tree their respective indices 
// this involves changing the first part, and being less specific about int conversions. 
// These might not even be needed. All it requires is searching for an operator, which is identified by a "("

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
      ss << ": Could not parse policy: it is not a literal but does not have [ op( ] " << std::endl;
      throw std::runtime_error(ss.str());
    }
    std::string op = expr.substr(0,start_index);
    
    // check it is an operator
    if (!( (op == op_OR) || (op == op_AND) || (op == op_THR))) { 
      stringstream ss(ERR_BAD_POLICY);
      ss << ": Could not parse ShTreeAccessPolicy: operator not recognized: " << op << std::endl;
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

Big ShTreeAccessPolicy::findCoefficient(std::string id,const vector<std::string> shareIDs) const {
//   int n = contains<std::string>(shareIDs, id);
//   if (n >= 0) {
//     return 1; 
//   } else {
//     return 0;
//   }
  return 0;
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

void ShTreeSS::manageRandomness(RandomnessActions action, shared_ptr<TreeNode> root, int count) {  
  shared_ptr<NodeContent> node = root->getNode();

  if (root->isNil() || root->isLeaf()) {
    return;
  }

  if (node->getInnerNodeType() != InnerNodeType::THR) {
    stringstream ss(ERR_BAD_TREE);  
    ss << ": ShTreeSS: ManageRandomness algorithm received a gate different from Threshold: " << node->to_string();
    throw std::runtime_error(ss.str());
  }

  if (node->getInnerNodeType() != InnerNodeType::THR) {
    stringstream ss(ERR_BAD_TREE);  
    ss << ": ShTreeSS: Distribution algorithm received a gate different from Threshold: " << node->to_string();
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
      manageRandomness(action, child, count);
      continue;
    }
  }
}

void ShTreeSS::manageRandomness(RandomnessActions action) {  
  manageRandomness(action, i_policy->getPolicy(), 0);
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
  manageRandomness(RandomnessActions::randomize);
  return distribute_determ(s, m_randomness);
}


std::vector<ShareTuple> ShTreeSS::distribute_determ(shared_ptr<TreeNode> root, const Big& s, const vector<Big>& randomness, int count){
  
  vector<ShareTuple> shares;

  shared_ptr<NodeContent> node = root->getNode();

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

  int arity = node->getArity();
  unsigned int threshold = node->getThreshold();

  Big acc; // cummulative sum for computing the polynomial
  Big accX; // cummulative value for the variable power (x^i)
  Big temp; // individual term a*x^i
  int point;
  vector<Big> poly(threshold);
  poly[0]=s;
  for (unsigned int i = 1; i < threshold; i++){
    poly[i] = randomness[count+i-1];
    count++;
  }

  for (int j=0;j<arity;j++) {   
    point = j+1; // take notice: we should not have a point 0, because that would reveal the secret immediately as the share. 
    // Reconstruction has to remember to include this +1 in its calculations
    acc=poly[0]; accX=point;
    for (unsigned int k=1;k<threshold;k++) { 
      // evaluate polynomial a0+a1*x+a2*x^2... for x=point;
      temp = modmult(poly[k],(Big)accX,m_order); 
      //      DEBUG("a * x^" << k << ": " << temp);
      acc+=temp;
      accX*=point;
      acc = (acc + m_order) % m_order;
    }
    
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
  return shares;
}
  

std::vector<ShareTuple> ShTreeSS::distribute_determ(const Big& s, const vector<Big>& randomness){

  // each node in the policy tree is a threshold node. distribution works by computing a share of the secret for each child of that node
  // then, if the child is not a leaf, take its share as the new secret and repeat the process
  // each distribution requires some share public information, that is simply going to be the index of the respective child for that tree
  // this is therefore independent of the participant's value
  // this applies normally even to the case where a participant receives several different shares

  vector<ShareTuple> shares = distribute_determ(i_policy->getPolicy(), s, randomness, 0);
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

int ShTreeSS::extractPublicInfoFromID(std::string& shareID) {
  return extractChildNoFromID(shareID) + 1;
}

int ShTreeSS::extractChildNoFromID(std::string& shareID) {
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

Big ShTreeSS::computeLagrangeCoefficient(unsigned int shareIndex, vector<ShareTuple>& witnessShares, const Big& order) 
{
  Big z=1;
  Big shareCoef;

  std::string shareID = witnessShares[shareIndex].getShareID();
  int index = extractPublicInfoFromID(shareID);
  DEBUG("anchor share: " << index);
  guard("Participant index must be positive", index > 0);

  for (unsigned int k=0;k<witnessShares.size();k++) {      
    if (k == shareIndex) continue;
    std::string tempshareID = witnessShares[k].getShareID();
    int tempIndex = extractPublicInfoFromID(tempshareID);
    DEBUG("varying share: " << tempIndex);
    
    shareCoef = moddiv(order - tempIndex,(Big)(order + index - tempIndex),order);
    z=modmult(z,shareCoef,order);
  }
  return z;
}

ShareTuple ShTreeSS::detailedReconstruction(vector<ShareTuple>& minimalShares, std::string& prefix, const Big& order){
  Big sum = 0;
  for (unsigned int i = 0; i < minimalShares.size(); i++) {
    Big coeff = computeLagrangeCoefficient(i, minimalShares, order);
    Big term = modmult(minimalShares[i].getShare(),coeff,order);
    sum = (sum + term); 
  }
  sum = ((sum + order) % order);

  std::string newShareID = TreeNode::findIDForNode(getSetPrefix(prefix), extractChildNoFromID(prefix), NodeContentType::leaf, "?");
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

Big ShTreeSS::reconstruct(const vector<ShareTuple> shares){
    vector<ShareTuple> witnessShares;
    DEBUG("CALLING EVALUATE");
    if (!i_policy->evaluate(shares, witnessShares)) return -1;
    ENHDEBUG("FINISHED EVALUATE");

    if (witnessShares.size() == 1) {
      ENHDEBUG("Shortcut returning");
      return witnessShares[0].getShare();
    }
    DEBUG("CALLING REDUCE");
    ShareTuple share = reduceLowestShares(shares, m_order);
    ENHDEBUG("FINISHED REDUCE");

    return share.getShare();
}
