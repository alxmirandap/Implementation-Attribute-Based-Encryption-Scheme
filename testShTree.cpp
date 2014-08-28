#include "utils.h"
#include "ShTree.h"


// Shamir Tree can implement OR and AND nodes. Therefore, it should understand
// all the expressions that BL canonical does, and translate those gates to
// appropriate threshold versions. Therefore, most of the tests of BL canonical
// should be inherited here, together with new tests specific for threshold
// gates and their problems

int testParseTreeFromExpression() {
  int errors = 0;

  ShTreeAccessPolicy testPolicy;
  std::string base = "testParseExpression - [";

  std::string expr = "";
  shared_ptr<TreeNode> tree = testPolicy.parseTreeFromExpression(expr);
  shared_ptr<TreeNode> emptyTree = TreeNode::makeTree(NodeContent::makeNILNode());
  test_diagnosis(base + expr + "]", *tree == *emptyTree, errors);

  expr = "a1"; // literals must be integers, not strings
  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr =  op_OR + "(1,2,"; // missing closing )
  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr = op_AND + "(1,2,3),4"; // extra content beyond )
  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr = op_THR + "(4,1,2,3)"; // threshold is greater than arity )
  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  expr = op_THR + "(1)"; // threshold requires nodes beyond arity )
  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", true, errors);
  }

  // valid expressions:

  expr = "1"; 
  shared_ptr<NodeContent> leafNode = NodeContent::makeLeafNode(1);
  shared_ptr<TreeNode> verifTree = TreeNode::makeTree(leafNode);

  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", *tree == *verifTree, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }

  expr = op_OR + "(1)";
  shared_ptr<NodeContent> orNode = NodeContent::makeThreshNode(1,1);
  leafNode = NodeContent::makeLeafNode(1);
  verifTree = TreeNode::makeTree(orNode);
  verifTree->appendChild(leafNode);

  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", *tree == *verifTree, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }


  expr = op_OR + "(1,2,3)"; 
  orNode = NodeContent::makeThreshNode(3,1);
  shared_ptr<NodeContent> leaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> leaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> leaf3 = NodeContent::makeLeafNode(3);

  verifTree = TreeNode::makeTree(orNode);
  verifTree->appendChild(leaf1);
  verifTree->appendChild(leaf2);
  verifTree->appendChild(leaf3);

  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", (*verifTree == *tree), errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }


  expr = op_OR + "(1," + op_AND + "(2,3))";
  orNode = NodeContent::makeThreshNode(2,1);
  shared_ptr<NodeContent> andNode = NodeContent::makeThreshNode(2,2);
  leaf1 = NodeContent::makeLeafNode(1);
  leaf2 = NodeContent::makeLeafNode(2);
  leaf3 = NodeContent::makeLeafNode(3);
  
  verifTree = TreeNode::makeTree(orNode);
  verifTree->appendChild(leaf1);
  verifTree->appendChild(andNode);
  shared_ptr<TreeNode> subTree = verifTree->getChild(1);
  subTree->appendChild(leaf2);
  subTree->appendChild(leaf3);

  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", (*verifTree == *tree), errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }


  expr = op_OR + "(1, " + op_OR + "(2, " + op_THR + "(2,1,2,3)), 4, " + op_AND + "(1,2))"; 
  shared_ptr<NodeContent> rootNode = NodeContent::makeThreshNode(4,1);
  orNode = NodeContent::makeThreshNode(2,1);
  shared_ptr<NodeContent> thrNode = NodeContent::makeThreshNode(3,2);
  andNode = NodeContent::makeThreshNode(2,2);

  shared_ptr<NodeContent> baseleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> baseleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> baseleaf4 = NodeContent::makeLeafNode(4);

  shared_ptr<NodeContent> thrleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> thrleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> thrleaf3 = NodeContent::makeLeafNode(3);

  shared_ptr<NodeContent> andleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> andleaf2 = NodeContent::makeLeafNode(2);

  verifTree = TreeNode::makeTree(rootNode);
  shared_ptr<TreeNode> pTree2 = TreeNode::makeTree(orNode);
  shared_ptr<TreeNode> pTree4 = TreeNode::makeTree(andNode);
  shared_ptr<TreeNode> pTree22 = TreeNode::makeTree(thrNode);
  
  pTree22->appendChild(thrleaf1);
  pTree22->appendChild(thrleaf2);
  pTree22->appendChild(thrleaf3);

  pTree2->appendChild(baseleaf2);
  pTree2->appendTree(pTree22);

  pTree4->appendChild(andleaf1);
  pTree4->appendChild(andleaf2);

  verifTree->appendChild(baseleaf1);
  verifTree->appendTree(pTree2);
  verifTree->appendChild(baseleaf4);
  verifTree->appendTree(pTree4);

  try {
    tree = testPolicy.parseTreeFromExpression(expr);
    test_diagnosis(base + expr + "]", *verifTree == *tree, errors);
  } catch (std::exception &e) {
    test_diagnosis(base + expr + "]", false, errors);
  }

  DEBUG("verification tree: " << verifTree->to_string());
  DEBUG("real tree: " << tree->to_string());

  return errors;
}


// this test tries several simple trees with different sets of shares. The argument of a share is its participant, identified by a simple integer.
// but a share has an implicit ID, defined by the policy that supports its issue. SatisfyNode verifies that the right shares are present, according to a policy.
// testEvaluate then has more complex policies

int testSatisfyNodeDetail(shared_ptr<TreeNode> policy, vector<ShareTuple> shares, vector<ShareTuple> verifWitnesses, std::string text, int i) {
  ENHDEBUG("Satisfy Node Detail");
  debugVectorObj("Shares: ", shares);
  debugVectorObj("Verification: " , verifWitnesses);

  ENHDEBUG("Policy Tree: " << policy->to_string());

  bool b;
  bool verif_b;
  int errors = 0;
  vector<ShareTuple> witnessVector;
  b = ShTreeAccessPolicy::satisfyNode(policy, shares, witnessVector);
  ENHDEBUG("SatisfyNode returned: " << b);
  stringstream ss("");
  ss << "testSatisfyNode - [" << text << "]: " << i;
  if (verifWitnesses.empty()) {
    DEBUG("verif_b is false");
    verif_b = false;
  } else {
    DEBUG("verif_b is true");
    verif_b = true;
  }

  debugVectorObj("real Witness vector", witnessVector);
  debugVectorObj("verification  Witness vector", verifWitnesses);
  test_diagnosis(ss.str(), b == verif_b, errors);
  test_diagnosis(ss.str() + " - Witness vector", witnessVector == verifWitnesses, errors);    

  return errors;
}


int testSatisfyNode() {
  int errors = 0;


  shared_ptr<NodeContent> nilNode = NodeContent::makeNILNode();
  shared_ptr<NodeContent> orNode = NodeContent::makeThreshNode(4,1);
  shared_ptr<NodeContent> andNode = NodeContent::makeThreshNode(4,4);
  shared_ptr<NodeContent> thrNode = NodeContent::makeThreshNode(4,3);

  shared_ptr<NodeContent> orleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> orleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> orleaf3 = NodeContent::makeLeafNode(3);
  shared_ptr<NodeContent> orleaf4 = NodeContent::makeLeafNode(4);

  shared_ptr<NodeContent> andleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> andleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> andleaf3 = NodeContent::makeLeafNode(3);
  shared_ptr<NodeContent> andleaf4 = NodeContent::makeLeafNode(4);

  shared_ptr<NodeContent> thrleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> thrleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> thrleaf3 = NodeContent::makeLeafNode(3);
  shared_ptr<NodeContent> thrleaf4 = NodeContent::makeLeafNode(4);


  std::shared_ptr<TreeNode> nilTree = TreeNode::makeTree(nilNode);
  std::shared_ptr<TreeNode> orTree = TreeNode::makeTree(orNode);
  std::shared_ptr<TreeNode> andTree = TreeNode::makeTree(andNode);
  std::shared_ptr<TreeNode> thrTree = TreeNode::makeTree(thrNode);

  orTree->appendChild(orleaf1);
  orTree->appendChild(orleaf2);
  orTree->appendChild(orleaf3);
  orTree->appendChild(orleaf4);
 
  andTree->appendChild(andleaf3);
  andTree->appendChild(andleaf4);
  andTree->appendChild(andleaf1);
  andTree->appendChild(andleaf2);

  thrTree->appendChild(thrleaf2);
  thrTree->appendChild(thrleaf4);
  thrTree->appendChild(thrleaf1);
  thrTree->appendChild(thrleaf3);

  // share sets to use: {1}, {1,2}, {1,2,3}, {1,2,3,4}
  // respective witnesses: 
  // OR: {1}, {1}, {1}, {1}
  // AND: {}, {}, {}, {3,4,1,2}
  // THR: {}, {}, {4,1,3}, {2,4,1}

  ShareTuple o1(1, 0, "0:0");
  ShareTuple o2(2, 0, "0:1");
  ShareTuple o3(3, 0, "0:2");
  ShareTuple o4(4, 0, "0:3");

  ShareTuple a3(3,0,"0:0");
  ShareTuple a4(4,0,"0:1");
  ShareTuple a1(1,0,"0:2");
  ShareTuple a2(2,0,"0:3");

  ShareTuple t2(2,0,"0:0");
  ShareTuple t4(4,0,"0:1");
  ShareTuple t1(1,0,"0:2");
  ShareTuple t3(3,0,"0:3");

  vector<ShareTuple> orSet;
  vector<ShareTuple> andSet;
  vector<ShareTuple> thrSet;

  orSet.push_back(o1);
  orSet.push_back(o2);
  orSet.push_back(o3);
  orSet.push_back(o4);

  andSet.push_back(a1);
  andSet.push_back(a2);
  andSet.push_back(a3);
  andSet.push_back(a4);

  thrSet.push_back(t1);
  thrSet.push_back(t2);
  thrSet.push_back(t3);
  thrSet.push_back(t4);
  
  vector<ShareTuple> witnessVector; 
  vector<ShareTuple> verifSet;
  vector<ShareTuple> shareList;

  // NIL TREE
  bool b;
  b = ShTreeAccessPolicy::satisfyNode(nilTree, orSet, witnessVector);
  test_diagnosis("testSatisfyNode - [nilTree] [orSet]", !b, errors);  
  test_diagnosis("testSatisfyNode - [nilTree] [orSet] - witness vector", witnessVector.empty(), errors);
  witnessVector.clear();

  b = ShTreeAccessPolicy::satisfyNode(nilTree, andSet, witnessVector);
  test_diagnosis("testSatisfyNode - [nilTree] [andSet]", !b, errors);  
  test_diagnosis("testSatisfyNode - [nilTree] [andSet] - witness vector", witnessVector.empty(), errors);
  witnessVector.clear();

  b = ShTreeAccessPolicy::satisfyNode(nilTree, thrSet, witnessVector);
  test_diagnosis("testSatisfyNode - [nilTree] [thrSet]", !b, errors);  
  test_diagnosis("testSatisfyNode - [nilTree] [thrSet] - witness vector", witnessVector.empty(), errors);
  witnessVector.clear();

   
  // OR TREE

  verifSet.clear();
  verifSet.push_back(orSet[0]);

  for (unsigned int i = 0; i < orSet.size(); i++) {   
    shareList.clear();
    for (unsigned int j = 0; j <= i; j++) {
      shareList.push_back(orSet[j]);
    }

    ENHDEBUG("Calling detail");
    debugVectorObj("shareList", shareList);

    errors += testSatisfyNodeDetail(orTree, shareList, verifSet, "orTree", i);
  }

  // AND TREE
  verifSet.clear();

  for (unsigned int i = 0; i < andSet.size(); i++) {   
    shareList.clear();
    for (unsigned int j = 0; j <= i; j++) {
      shareList.push_back(andSet[j]);
    }
    if (i == andSet.size() - 1) {
      verifSet.push_back(a3);
      verifSet.push_back(a4);
      verifSet.push_back(a1);
      verifSet.push_back(a2);
    }
    errors += testSatisfyNodeDetail(andTree, shareList, verifSet, "andTree", i);
  }



  // THR TREE
  vector< vector<ShareTuple> > witnessList;
  verifSet.clear();

  vector<ShareTuple> failVerifSet;
  vector<ShareTuple> passVerifSet1;
  vector<ShareTuple> passVerifSet2;

  witnessList.push_back(failVerifSet); // adding empty vectors for the first 2 cases
  witnessList.push_back(failVerifSet); // adding empty vectors for the first 2 cases

  passVerifSet1.push_back(t2);
  passVerifSet1.push_back(t1);
  passVerifSet1.push_back(t3);
  witnessList.push_back(passVerifSet1);

  passVerifSet2.push_back(t2);
  passVerifSet2.push_back(t4);
  passVerifSet2.push_back(t1);
  witnessList.push_back(passVerifSet2);

  for (unsigned int i = 0; i < thrSet.size(); i++) {   
    shareList.clear();
    for (unsigned int j = 0; j <= i; j++) {
      shareList.push_back(thrSet[j]);
    }

    errors += testSatisfyNodeDetail(thrTree, shareList, witnessList[i], "thrTree", i);
  }



  return errors;
}

int testGetNumShares() {
  int errors = 0;

  std::string base = "TestGetNumShares: ";

  std::string expr0 = "1";
  ShTreeAccessPolicy pol0(expr0, 3);
  test_diagnosis(base + expr0, pol0.getNumShares() == 1, errors);


  std::string expr1 = op_OR + "(1,2,3)";
  ShTreeAccessPolicy pol1(expr1, 3);
  test_diagnosis(base + expr1, pol1.getNumShares() == 3, errors);

  std::string expr2 = op_OR + "(" + op_AND + "(1,2)," + op_AND + "(3,4))";
  ShTreeAccessPolicy pol2(expr2, 4);
  test_diagnosis(base + expr2, pol2.getNumShares() == 4, errors);

  std::string expr3 = op_OR + "(" + op_AND + "(3,4,5),7," + op_AND + "(1,2)," + op_AND + "(4,1,6,5))";
  ShTreeAccessPolicy pol3(expr3, 7);
  test_diagnosis(base + expr3, pol3.getNumShares() == 10, errors);

  std::string expr4 = op_OR + "(1, " + op_OR + "(2, " + op_THR + "(2,1,2,3)), 4, " + op_AND + "(1,2))"; 
  ShTreeAccessPolicy pol4(expr4, 4);
  test_diagnosis(base + expr4, pol4.getNumShares() == 8, errors);

  return errors;
}

int testObtainCoveredFrags() {
  int errors = 0;

  std::string base = "testObtainCoveredFrags: ";
  
  vector<int> atts;
  atts.push_back(7);
  atts.push_back(8);
  atts.push_back(2);
  atts.push_back(4);
  atts.push_back(5);
  atts.push_back(3);
  
  vector<int> attFragIndices;
  vector<int> keyFragIndices;
  vector<std::string> coveredShareIDs;

  std::string expr = op_OR + "(1, " + op_OR + "(2, " + op_THR + "(2,1,2,3)), 4, " + op_AND + "(1,2))"; 
  shared_ptr<ShTreeAccessPolicy> policy = make_shared<ShTreeAccessPolicy>(expr, 5);
    
  vector<std::string> verifShareIDs;
  verifShareIDs.push_back("0:1:0");
  verifShareIDs.push_back("0:1:1:1");
  verifShareIDs.push_back("0:1:1:2");
  verifShareIDs.push_back("0:2");
  verifShareIDs.push_back("0:3:1");

  vector<int> verifKeyFragIDs;
  verifKeyFragIDs.push_back(1);
  verifKeyFragIDs.push_back(3);
  verifKeyFragIDs.push_back(4);
  verifKeyFragIDs.push_back(5);
  verifKeyFragIDs.push_back(7);

  vector<int> verifAttFragIDs;
  verifAttFragIDs.push_back(2);
  verifAttFragIDs.push_back(2);
  verifAttFragIDs.push_back(5);
  verifAttFragIDs.push_back(3);
  verifAttFragIDs.push_back(2);

  policy->obtainCoveredFrags(atts, attFragIndices, keyFragIndices, coveredShareIDs); 

  test_diagnosis(base + "number of covered IDs", coveredShareIDs.size() == verifShareIDs.size(), errors);
  for (unsigned int i = 0; i < coveredShareIDs.size(); i++) {
    test_diagnosis(base + "coveredID " + convertIntToStr(i), coveredShareIDs[i] == verifShareIDs[i], errors);
  }

  test_diagnosis(base + "number of keyFragIndices", keyFragIndices.size() == verifKeyFragIDs.size(), errors);
  for (unsigned int i = 0; i < keyFragIndices.size(); i++) {
    test_diagnosis(base + "keyFragIndex " + convertIntToStr(i), keyFragIndices[i] == verifKeyFragIDs[i], errors);
  }

  test_diagnosis(base + "number of attFragIndices", attFragIndices.size() == verifAttFragIDs.size(), errors);
  for (unsigned int i = 0; i < attFragIndices.size(); i++) {
    test_diagnosis(base + "attFragIndex " + convertIntToStr(i), attFragIndices[i] == verifAttFragIDs[i], errors);
  }
  
  return errors;
}


int runTests(std::string &testName) {
  testName = "Test ShTreeAccessPolicy";
  int errors = 0;

  // Policy tests
  ENHOUT("Secret sharing policy tests");
  errors += testParseTreeFromExpression();
  errors += testSatisfyNode();
//   errors += testEvaluate();
  errors += testGetNumShares();
  errors += testObtainCoveredFrags();
//   
//   // Secret Sharing tests
//   ENHOUT("Secret sharing scheme tests");
//   errors += testGetSharesForParticipants(pfc);
//   errors += testDistributeAndReconstruct(pfc);

  return errors;
}

int main() {
  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=10;

  time_t seed;            // crude randomisation. Check if this is the version that is crypto-secure.
  time(&seed);
  irand((long)seed);

  std::string test_name;
  int result = runTests(test_name);
  print_test_result(result,test_name);

  return 0;
}
