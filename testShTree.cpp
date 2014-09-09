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

  // Leaf Node labels: "<the Id of the parent> := <the participant>"

  ShareTuple o1(1, 0, "0:0:=1");
  ShareTuple o2(2, 0, "0:1:=2");
  ShareTuple o3(3, 0, "0:2:=3");
  ShareTuple o4(4, 0, "0:3:=4");

  ShareTuple a3(3,0,"0:0:=3");
  ShareTuple a4(4,0,"0:1:=4");
  ShareTuple a1(1,0,"0:2:=1");
  ShareTuple a2(2,0,"0:3:=2");

  ShareTuple t2(2,0,"0:0:=2");
  ShareTuple t4(4,0,"0:1:=4");
  ShareTuple t1(1,0,"0:2:=1");
  ShareTuple t3(3,0,"0:3:=3");

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

int testVectors(vector<vector<ShareTuple> > testRun, vector<vector<ShareTuple> > witnessRun, ShTreeAccessPolicy pol, std::string expr) {
  int errors = 0;
  ENHDEBUG("===============");
  ENHOUT("Testing vectors");

  vector<ShareTuple> witnessVector;
  //  for (unsigned int i = 0; i < testRun.size(); i++) {
  for (unsigned int i = 1; i < 2; i++) {
    witnessVector.clear();

    bool b = pol.evaluate(testRun[i], witnessVector);
    DEBUG("Returned from evaluate: " << b);

        debugVectorObj("---> witness list: ", witnessRun[i]);
        debugVectorObj("---> real witnesses: ", witnessVector);

    bool verif_b;
    stringstream ss("");
    ss << "testEvaluate [" << expr << "]: " << i;
    if (witnessRun[i].empty()) {
      verif_b = false;
      DEBUG("Witness vector is empty. Target: " << verif_b);
    } else {
      verif_b = true;
      DEBUG("Witness vector has elements. Target: " << verif_b);
    }
    test_diagnosis(ss.str(), b == verif_b, errors);
    test_diagnosis(ss.str() + " - Witness vector", witnessVector == witnessRun[i], errors);
  }
  return errors;
}

int testExpr1() {
  int errors = 0;

  std::string expr = "";
  ShTreeAccessPolicy pol(expr, 0);

  ShareTuple s11(1,0,"1:1");
  ShareTuple s12(2,0,"2:2");
  ShareTuple s13(3,0,"3:3");
  ShareTuple s14(4,0,"4:4");
  ShareTuple s15(5,0,"5:5");

  vector<ShareTuple> shareList;
  shareList.push_back(s11);
  shareList.push_back(s12);
  shareList.push_back(s13);
  shareList.push_back(s14);
  shareList.push_back(s15);

  vector<ShareTuple> emptyList;
  vector<ShareTuple> currentList;
  vector< vector<ShareTuple> > testRun;
  vector< vector<ShareTuple> > witnessRun;

  for (unsigned int i = 0; i < 5; i++) {  
    currentList.push_back(shareList[i]);
    testRun.push_back(currentList);
    witnessRun.push_back(emptyList);
  }
  errors += testVectors(testRun, witnessRun, pol, expr);    
  return errors;
}


int testExpr2() {
  int errors = 0;

  std::string expr = "4";
  ShTreeAccessPolicy pol(expr, 0);

  ShareTuple s11(1,0,"0:=1");
  ShareTuple s12(2,0,"0:=2");
  ShareTuple s13(3,0,"0:=3");
  ShareTuple s14(4,0,"0:=4");
  ShareTuple s15(5,0,"0:=5");

  vector<ShareTuple> shareList;
  shareList.push_back(s11);
  shareList.push_back(s12);
  shareList.push_back(s13);
  shareList.push_back(s14);
  shareList.push_back(s15);

  vector<ShareTuple> witnessList;
  vector<ShareTuple> currentList;
  vector< vector<ShareTuple> > testRun;
  vector< vector<ShareTuple> > witnessRun;

  for (unsigned int i = 0; i < 5; i++) {  
    currentList.push_back(shareList[i]);
    testRun.push_back(currentList);
    if (i == 3) {
      witnessList.push_back(shareList[i]);
    }
    witnessRun.push_back(witnessList);
  }
  errors += testVectors(testRun, witnessRun, pol, expr);    
  return errors;
}

int testExpr3() {
  int errors = 0;

  std::string expr = op_THR + "(2, 1, " + op_THR + "(1, 2, " + op_THR + "(2,5,2,3,4)), 4, " + op_THR + "(2,6,1))";
  //  OUT("Expr3: " << expr);
  ShTreeAccessPolicy pol(expr, 6);

  ShareTuple s11(1,0,"0:0:=1");
  ShareTuple s22(2,0,"0:1:0:=2");
  ShareTuple s35(5,0,"0:1:1:0:=5");
  ShareTuple s32(2,0,"0:1:1:1:=2");
  ShareTuple s33(3,0,"0:1:1:2:=3");
  ShareTuple s34(4,0,"0:1:1:3:=4");
  ShareTuple s44(4,0,"0:2:=4");
  ShareTuple s56(6,0,"0:3:0:=6");
  ShareTuple s51(1,0,"0:3:1:=1");

  vector<ShareTuple> sl;
  sl.push_back(s11);
  sl.push_back(s22);
  sl.push_back(s35);
  sl.push_back(s32);
  sl.push_back(s33);
  sl.push_back(s34);
  sl.push_back(s44);
  sl.push_back(s56);
  sl.push_back(s51);

  vector<ShareTuple> run1;
  run1.push_back(sl[0]);
  run1.push_back(sl[6]);

  vector<ShareTuple> run2;
  run2.push_back(sl[7]);
  run2.push_back(sl[8]);
  run2.push_back(sl[1]);
  run2.push_back(sl[4]);
  run2.push_back(sl[5]);

  vector<ShareTuple> run3;
  run3.push_back(sl[7]);
  run3.push_back(sl[8]);
  run3.push_back(sl[1]);
  run3.push_back(sl[2]);
  run3.push_back(sl[4]);

  vector<ShareTuple> run4;
  run4.push_back(sl[5]);
  run4.push_back(sl[6]);
  run4.push_back(sl[1]);

  vector<ShareTuple> run5;
  run5.push_back(sl[6]);
  run5.push_back(sl[8]);
  run5.push_back(sl[3]);

  vector< vector<ShareTuple> > testRun;
  testRun.push_back(run1);
  testRun.push_back(run2);
  testRun.push_back(run3);
  testRun.push_back(run4);
  testRun.push_back(run5);

  vector< vector<ShareTuple> > witnessRun;
  vector<ShareTuple> witnessList;
  witnessList.push_back(sl[0]);
  witnessList.push_back(sl[6]);
  witnessRun.push_back(witnessList);
  witnessList.clear();

  witnessList.push_back(sl[1]);
  witnessList.push_back(sl[7]);
  witnessList.push_back(sl[8]);
  witnessRun.push_back(witnessList);
  witnessList.clear();

  witnessList.push_back(sl[1]);
  witnessList.push_back(sl[7]);
  witnessList.push_back(sl[8]);
  witnessRun.push_back(witnessList);
  witnessList.clear();

  witnessList.push_back(sl[1]);
  witnessList.push_back(sl[6]);
  witnessRun.push_back(witnessList);
  witnessList.clear();

  witnessRun.push_back(witnessList);
  witnessList.clear();

  errors += testVectors(testRun, witnessRun, pol, expr);    
  return errors;
}




int testEvaluate() {
  int errors = 0;

  //  errors += testExpr1(); // nil tree
  //  errors += testExpr2(); // leaf tree
  errors += testExpr3(); // multi-level tree

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
  verifShareIDs.push_back("0:1:0:=2");
  verifShareIDs.push_back("0:1:1:1:=2");
  verifShareIDs.push_back("0:1:1:2:=3");
  verifShareIDs.push_back("0:2:=4");
  verifShareIDs.push_back("0:3:1:=2");

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

  debugVector("coveredShareIDs: ", coveredShareIDs);
  debugVector("verifShareIDs: ", verifShareIDs);

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

int testGetSharesForParticipants() {
  int errors = 0;

  ENHDEBUG("Creating instances");
  //  std::string expr = op_THR + "(1, 1, " + op_THR + "(3,2,3,4), " + op_THR + "(2,2,5, " + op_THR + "(1,4,5)))";
  //  shared_ptr<ShTreeAccessPolicy> policy = make_shared<ShTreeAccessPolicy>(expr, 5);
  //  ShTreeSS testScheme(policy, pfc.order(), pfc);

  ENHDEBUG("Creating shares");
  ShareTuple s1a(1, 0, "1a");
  ShareTuple s1b(1, 0, "1b");
  ShareTuple s2a(2, 0, "2a");
  ShareTuple s3a(3, 0, "3a");
  ShareTuple s3b(3, 0, "3b");
  ShareTuple s3c(3, 0, "3c");
  ShareTuple s4a(4, 0, "4a");
  ShareTuple s5a(5, 0, "5a");

  vector<ShareTuple> shares;

  shares.push_back(s1a);
  shares.push_back(s1b);
  shares.push_back(s2a);
  shares.push_back(s3a);
  shares.push_back(s3b);
  shares.push_back(s3c);
  shares.push_back(s4a);
  shares.push_back(s5a);

  vector<int> parts313;
  vector<int> parts276;
  vector<int> parts45;

  parts313.push_back(3);
  parts313.push_back(1);
  parts313.push_back(3);

  parts276.push_back(2);
  parts276.push_back(7);
  parts276.push_back(6);

  parts45.push_back(4);
  parts45.push_back(5);

  vector<ShareTuple> verif313;
  vector<ShareTuple> verif276;
  vector<ShareTuple> verif45;

  verif313.push_back(s1a);
  verif313.push_back(s1b);
  verif313.push_back(s3a);
  verif313.push_back(s3b);
  verif313.push_back(s3c);

  verif276.push_back(s2a);
  
  verif45.push_back(s4a);
  verif45.push_back(s5a);
  
  std::string base = "testGetSharesForParticipants - ";

  ENHDEBUG("Calling diagnosis");
  DEBUG("call 1");
  vector<ShareTuple> returnedShares = SecretSharing::getSharesForParticipants(parts313, shares);
  test_diagnosis(base + "[313]", returnedShares == verif313, errors);

  DEBUG("call 2");
  returnedShares = SecretSharing::getSharesForParticipants(parts276, shares);
  test_diagnosis(base + "[276]", returnedShares == verif276, errors);

  DEBUG("call 3");
  returnedShares = SecretSharing::getSharesForParticipants(parts45, shares);
  test_diagnosis(base + "[45]", returnedShares == verif45, errors);

  ENHDEBUG("Finished diagnosis");	
  return errors;
}

int testReconFromShares(vector<int> party, std::string message,
			ShTreeSS scheme, vector<ShareTuple> shares, bool goodParty,
			unsigned int expectedNumberShares, Big orig_secret) {

  int errors = 0;
  ENHDEBUG("testReconFromShares");
  debugVector("Party", party);
  debugVectorObj("Shares", shares);
  

  vector<ShareTuple> partShares;
  partShares = SecretSharing::getSharesForParticipants(party, shares);

  debugVectorObj("Participant Shares", partShares);

  vector<ShareTuple> witnessShares;
  bool success = scheme.evaluate(partShares, witnessShares);
  test_diagnosis(message + "evaluation", success == goodParty, errors);

  debugVectorObj("Witness Shares", witnessShares);

  test_diagnosis(message + "number of shares", witnessShares.size() == expectedNumberShares, errors);
  DEBUG("number of witness shares: " << witnessShares.size());
  DEBUG("expected number of shares: " << expectedNumberShares);
  ENHDEBUG("Calling reconstruction");
  Big reced_secret = scheme.reconstruct(witnessShares);
  ENHDEBUG("Finished reconstruction");
  if (goodParty) {
    test_diagnosis(message + "reconstruction", reced_secret == orig_secret, errors);
  } else {
    test_diagnosis(message + "reconstruction", reced_secret != orig_secret, errors);
  }

  return errors;
}

int testSmallDistributeAndReconstruct(PFC &pfc){
  int errors = 0;

  Big s = 7;
  Big order = 11;

  std::string expr = op_THR + "(3, 1,2,3,4,5)";
  shared_ptr<ShTreeAccessPolicy> policy = make_shared<ShTreeAccessPolicy>(expr, 5);
  ShTreeSS testScheme(policy, order, pfc);

  std::string base = "testSmallDistributeAndReconstruct ";
  
  vector<int> party; 
  party.push_back(3);
  party.push_back(5);
  party.push_back(1);

   
  vector<Big> randomness;
  randomness.push_back(3);
  randomness.push_back(4);

  REPORT("Calling distribute_determ");
  vector<ShareTuple> shares = testScheme.distribute_determ(s, randomness);

  debugVectorObj("returned shares", shares);
  
  test_diagnosis(base + "number of shares:", shares.size() == policy->getNumShares(), errors);
    
  
  errors += testReconFromShares(party, base + ": ", testScheme, shares, true, 3, s); 

  return errors;
}


int testDistributeAndReconstruct(PFC &pfc){
  int errors = 0;
  REPORT("testDistribute and Reconstruct");
  int niter = 5;
    
  DEBUG("Initting s");
  Big s;
  DEBUG("Initting old_s");
  Big old_s=0;
  DEBUG("Obtaining order");
  Big order = pfc.order();


  DEBUG("Creating instances");
  std::string expr = op_THR + "(1, 1, " + op_THR + "(3,2,3,4), " + op_THR + "(2,2,1, " + op_THR + "(1,4,5)))";
  shared_ptr<ShTreeAccessPolicy> policy = make_shared<ShTreeAccessPolicy>(expr, 5);
  ShTreeSS testScheme(policy, pfc.order(), pfc);

  std::string base = "testDistributeAndReconstruct ";
  
  vector<int> party1; // should succeed
  vector<int> party234; // should succeed
  vector<int> party25; // should succeed
  vector<int> party42; // should succeed
  vector<int> badparty23; // should fail
  vector<int> badparty53; // should fail

  party1.push_back(1);
  
  party234.push_back(2);
  party234.push_back(3);
  party234.push_back(4);

  party25.push_back(2);
  party25.push_back(5);

  party42.push_back(4);
  party42.push_back(2);
  
  badparty23.push_back(2);
  badparty23.push_back(3);

  badparty53.push_back(5);
  badparty53.push_back(3);
  
  
  old_s = 0;
  for (int j = 0; j < niter; j++){
    OUT(base + "Iteration: " << j);
    pfc.random(s);
    s = s % order;
    
    DEBUG("s: " << s << "\t old s: " << old_s);
    guard("s should be random, and different from the last value or 0", s != old_s); // the probability that s is 0 or the old value should be negligible

    REPORT("Calling distribute_random");
    vector<ShareTuple> shares = testScheme.distribute_random(s);
    
    test_diagnosis(base + "number of shares:", shares.size() == policy->getNumShares(), errors);
    
    errors += testReconFromShares(party1, base + "[1]: ", testScheme, shares, true, 1, s);
    errors += testReconFromShares(party234, base + "[234]: ", testScheme, shares, true, 3, s);
    errors += testReconFromShares(party25, base + "[25]: ", testScheme, shares, true, 2, s);
    errors += testReconFromShares(party42, base + "[42]: ", testScheme, shares, true, 2, s);
    errors += testReconFromShares(badparty23, base + "[23]: ", testScheme, shares, false, 0, s);
    errors += testReconFromShares(badparty53, base + "[53]: ", testScheme, shares, false, 0, s);
        
    old_s = s;
  }
    return errors;
}


int testUpdateSet(){
  int errors = 0;

  std::map<std::string, vector<ShareTuple> > map;
  std::string prefix1 = "prefix1"; 
  std::string prefix2 = "prefix2"; 
  ShareTuple s1(1,0,"a");
  ShareTuple s2(2,0,"b");
  ShareTuple s3(3,0,"c");

  vector<ShareTuple> vec1;
  vector<ShareTuple> vec2;
  vec1.push_back(s1);
  vec2.push_back(s2);

  map[prefix1] = vec1;
  map[prefix2] = vec2;

  test_diagnosis("testUpdateSet - initial state size", map.size() == 2, errors);
  test_diagnosis("testUpdateSet - initial state vec1 size", map.find(prefix1)->second.size() == 1, errors);
  test_diagnosis("testUpdateSet - initial state vec2 size", map.find(prefix2)->second.size() == 1, errors);
  ShTreeSS::updateSet(map, prefix2, s3);
  test_diagnosis("testUpdateSet - after state size", map.size() == 2, errors);
  test_diagnosis("testUpdateSet - initial state vec1 size", map.find(prefix1)->second.size() == 1, errors);
  test_diagnosis("testUpdateSet - initial state vec2 size", map.find(prefix2)->second.size() == 2, errors);

  std::map<std::string, vector<ShareTuple> >::iterator it;
  it = map.find(prefix2);
  
  test_diagnosis("testUpdateSet - vec2 contents 0", it->second[0] == s2, errors);
  test_diagnosis("testUpdateSet - vec2 contents 1", it->second[1] == s3, errors);
    
  return errors;
}

int testAddNewSet(){
  int errors = 0;

  std::map<std::string, vector<ShareTuple> > map;
  std::string prefix1 = "prefix1"; 
  std::string prefix2 = "prefix2"; 
  ShareTuple s1(1,0,"a");
  ShareTuple s2(2,0,"b");

  vector<ShareTuple> vec1;
  vector<ShareTuple> vec2;
  vec1.push_back(s1);

  map[prefix1] = vec1;

  test_diagnosis("testAddNewSet - initial state size", map.size() == 1, errors);
  test_diagnosis("testAddNewSet - initial state vec1 size", map.find(prefix1)->second.size() == 1, errors);
  ShTreeSS::addNewSet(map, prefix2, s2);
  test_diagnosis("testAddNewSet - after state size", map.size() == 2, errors);
  test_diagnosis("testAddNewSet - initial state vec1 size", map.find(prefix1)->second.size() == 1, errors);
  test_diagnosis("testAddNewSet - initial state vec2 size", map.find(prefix2)->second.size() == 1, errors);

  std::map<std::string, vector<ShareTuple> >::iterator it;
  it = map.find(prefix2);
  
  test_diagnosis("testUpdateSet - vec2 contents 0", it->second[0] == s2, errors);

  return errors;
}

int testPutShareInSet(){
  int errors = 0;

  std::map<std::string, vector<ShareTuple> > map;
  std::string prefix1 = "prefix1"; 
  std::string prefix2 = "prefix2"; 
  ShareTuple s1(1,0,"a");
  ShareTuple s2(2,0,"b");
  ShareTuple s3(3,0,"c");

  ShTreeSS::putShareInSet(map, prefix1, s1);
  ShTreeSS::putShareInSet(map, prefix2, s2);

  test_diagnosis("testPutShareInSet - initial state size", map.size() == 2, errors);
  test_diagnosis("testPutShareInSet - initial state vec1 size", map.find(prefix1)->second.size() == 1, errors);
  test_diagnosis("testPutShareInSet - initial state vec2 size", map.find(prefix2)->second.size() == 1, errors);
  ShTreeSS::putShareInSet(map, prefix2, s3);  
  test_diagnosis("testPutShareInSet - after state size", map.size() == 2, errors);
  test_diagnosis("testPutShareInSet - initial state vec1 size", map.find(prefix1)->second.size() == 1, errors);
  test_diagnosis("testPutShareInSet - initial state vec2 size", map.find(prefix2)->second.size() == 2, errors);

  std::map<std::string, vector<ShareTuple> >::iterator it;
  it = map.find(prefix1);
  test_diagnosis("testPutShareInSet - vec1 contents 0", it->second[0] == s1, errors);

  it = map.find(prefix2);
  test_diagnosis("testPutShareInSet - vec2 contents 0", it->second[0] == s2, errors);
  test_diagnosis("testPutShareInSet - vec2 contents 1", it->second[1] == s3, errors);

  return errors;
}

int testGetSetPrefix(){
  int errors = 0;

  std::string s1 = "0";
  std::string s2 = "0:=7";
  std::string s3 = "0:1:3";
  std::string s4 = "0:1:3:=19";
  std::string s5 = "0:1:3:9:7";
  std::string s6 = "0:1:3:9:7:=21";


  std::string vs1 = "";
  std::string vs2 = "";
  std::string vs3 = "0:1";
  std::string vs4 = "0:1";
  std::string vs5 = "0:1:3:9";
  std::string vs6 = "0:1:3:9";


  vector<std::string> testCases;
  testCases.push_back(s1);
  testCases.push_back(s2);
  testCases.push_back(s3);
  testCases.push_back(s4);
  testCases.push_back(s5);
  testCases.push_back(s6);

  vector<std::string> verifCases;
  verifCases.push_back(vs1);
  verifCases.push_back(vs2);
  verifCases.push_back(vs3);
  verifCases.push_back(vs4);
  verifCases.push_back(vs5);
  verifCases.push_back(vs6);

  for (unsigned int i = 0; i < testCases.size(); i++) {
    test_diagnosis("testGetSetPrefix - " + convertIntToStr(i), ShTreeSS::getSetPrefix(testCases[i]) == verifCases[i], errors);
  }

  return errors;
}

int testLagrangeCoefficient(PFC &pfc) {
  int errors = 0;

  vector<ShareTuple> shares;
  
  
  Big order = pfc.order();
  
  int x0 = 4;
  int x1 = 13;
  int x2 = 27;
  int x3 = 8;

  ShareTuple s1(1,0,"0:" + convertIntToStr(x0-1) + ":=x");
  ShareTuple s2(2,0,"0:" + convertIntToStr(x1-1) + ":=y");
  ShareTuple s3(3,0,"0:" + convertIntToStr(x2-1) + ":=z");
  ShareTuple s4(4,0,"0:" + convertIntToStr(x3-1) + ":=w");

  shares.push_back(s1);
  shares.push_back(s2);
  shares.push_back(s3);
  shares.push_back(s4);

  Big F01 = moddiv(order - x1,(Big)(order + x0 - x1),order);
  Big F02 = moddiv(order - x2,(Big)(order + x0 - x2),order);
  Big F03 = moddiv(order - x3,(Big)(order + x0 - x3),order);

  Big F10 = moddiv(order - x0,(Big)(order + x1 - x0),order);
  Big F12 = moddiv(order - x2,(Big)(order + x1 - x2),order);
  Big F13 = moddiv(order - x3,(Big)(order + x1 - x3),order);

  Big F20 = moddiv(order - x0,(Big)(order + x2 - x0),order);
  Big F21 = moddiv(order - x1,(Big)(order + x2 - x1),order);
  Big F23 = moddiv(order - x3,(Big)(order + x2 - x3),order);

  Big F30 = moddiv(order - x0,(Big)(order + x3 - x0),order);
  Big F31 = moddiv(order - x1,(Big)(order + x3 - x1),order);
  Big F32 = moddiv(order - x2,(Big)(order + x3 - x2),order);

  Big L0 =   ShTreeAccessPolicy::computeLagrangeCoefficient(0, shares, order);
  test_diagnosis("testLagrangeCoefficient: L0", L0 == modmult( modmult(F01, F02, order), F03, order), errors);

  Big L1 =   ShTreeAccessPolicy::computeLagrangeCoefficient(1, shares, order);
  test_diagnosis("testLagrangeCoefficient: L1", L1 == modmult( modmult(F10, F12, order), F13, order), errors);

  Big L2 =   ShTreeAccessPolicy::computeLagrangeCoefficient(2, shares, order);
  test_diagnosis("testLagrangeCoefficient: L2", L2 == modmult( modmult(F20, F21, order), F23, order), errors);

  Big L3 =   ShTreeAccessPolicy::computeLagrangeCoefficient(3, shares, order);
  test_diagnosis("testLagrangeCoefficient: L3", L3 == modmult( modmult(F30, F31, order), F32, order), errors);

  return errors;
}

int testExtractPublicInfoFromID() {
  int errors = 0;

  std::string s1 = "0:1:4";
  std::string s2 = "0:1:4:=8";
  std::string s3 = "0:=9";

  test_diagnosis("testExtractPublicInfoFromID - " + s1, 
		 ShTreeAccessPolicy::extractPublicInfoFromChildNo(ShTreeAccessPolicy::extractChildNoFromID(s1)) == 5 , errors);
  test_diagnosis("testExtractPublicInfoFromID - " + s2, 
		 ShTreeAccessPolicy::extractPublicInfoFromChildNo(ShTreeAccessPolicy::extractChildNoFromID(s2)) == 5 , errors);

  try {
    ShTreeAccessPolicy::extractPublicInfoFromChildNo(ShTreeAccessPolicy::extractChildNoFromID(s3));
    test_diagnosis("testExtractPublicInfoFromID - exception " + s3, false, errors);
  } catch (std::runtime_error &e) {
    test_diagnosis("testExtractPublicInfoFromID - exception " + s3, true, errors);
  }


  return errors;
}

int testextractPrefixAndNoFromID() {
  int errors = 0;

  std::string s1 = "0:1:7:4:=3";
  std::string s2 = "1:7:4:=3";
  std::string s3 = "7:4:=3";
  std::string s4 = "4:=3";
  std::string s5 = "4:=3:";

  std::string prefix;
  int childNo;

  bool success = ShTreeAccessPolicy::extractPrefixAndNoFromID(s1, prefix, childNo);
  test_diagnosis("testextractPrefixAndNoFromID: " + s1 + " [success]", success, errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s1 + " [s1]", s1 == "1:7:4:=3", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s1 + " [prefix]", prefix == "0:", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s1 + " [childNo]", childNo == 1, errors);

  success = ShTreeAccessPolicy::extractPrefixAndNoFromID(s2, prefix, childNo);
  test_diagnosis("testextractPrefixAndNoFromID: " + s2 + " [success]", success, errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s2 + " [s2]", s2 == "7:4:=3", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s2 + " [prefix]", prefix == "1:", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s2 + " [childNo]", childNo == 7, errors);

  success = ShTreeAccessPolicy::extractPrefixAndNoFromID(s3, prefix, childNo);
  test_diagnosis("testextractPrefixAndNoFromID: " + s3 + " [success]", success, errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s3 + " [s3]", s3 == "4:=3", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s3 + " [prefix]", prefix == "7:", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s3 + " [childNo]", childNo == 4, errors);

  success = ShTreeAccessPolicy::extractPrefixAndNoFromID(s4, prefix, childNo);
  test_diagnosis("testextractPrefixAndNoFromID: " + s4 + " [success]", !success, errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s4 + " [s4]", s4 == "4:=3", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s4 + " [prefix]", prefix == "7:", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s4 + " [childNo]", childNo == 4, errors);

  success = ShTreeAccessPolicy::extractPrefixAndNoFromID(s5, prefix, childNo);
  test_diagnosis("testextractPrefixAndNoFromID: " + s5 + " [success]", !success, errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s5 + " [s5]", s5 == "4:=3:", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s5 + " [prefix]", prefix == "7:", errors);
  test_diagnosis("testextractPrefixAndNoFromID: " + s5 + " [childNo]", childNo == 4, errors);

  return errors;
}

int testStoreSharePrefixes() {
  int errors = 0;

  std::string s1 = "0:1:0:=2";
  std::string s2 = "0:1:1:=3";
  std::string s3 = "0:1:2:=4";
  std::string s5 = "0:2:2:1:=4";
  std::string s4 = "0:2:1:=2";


  std::map<std::string, vector<int> > setChildNos;

  ShTreeAccessPolicy::storeSharePrefixes(setChildNos, s1);

  //  REPORT("s1");
  vector<int> vec = setChildNos["0:"];
  vector<int> verif0;
  verif0.push_back(1);
  debugVector("vec [0:]", vec);
  debugVector("verif0", verif0);
  test_diagnosis("testStoreSharePrefixes: " + s1 + "[0:]", verif0 == vec, errors);

  vec = setChildNos["0:1:"];
  vector<int> verif01;
  verif01.clear();
  verif01.push_back(0);
  debugVector("vec [0:1:]", vec);
  debugVector("verif01", verif01);
  test_diagnosis("testStoreSharePrefixes: " + s1 + "[0:1:]", verif01 == vec, errors);

  //  REPORT("s2");
  ShTreeAccessPolicy::storeSharePrefixes(setChildNos, s2);
  vec = setChildNos["0:"];
  debugVector("vec [0:]", vec);
  debugVector("verif0", verif0);
  test_diagnosis("testStoreSharePrefixes: " + s2 + "[0:]", verif0 == vec, errors);

  vec = setChildNos["0:1:"];
  verif01.push_back(1);
  debugVector("vec  [0:1:]", vec);
  debugVector("verif01", verif01);
  test_diagnosis("testStoreSharePrefixes: " + s2 + "[0:1:]", verif01 == vec, errors);

  //   REPORT("s3");
  ShTreeAccessPolicy::storeSharePrefixes(setChildNos, s3);
  vec = setChildNos["0:"];
  test_diagnosis("testStoreSharePrefixes: " + s3 + "[0:]", verif0 == vec, errors);
  vec = setChildNos["0:1:"];
  verif01.push_back(2);
  test_diagnosis("testStoreSharePrefixes: " + s3 + "[0:1:]", verif01 == vec, errors);

  //   REPORT("s4");
   ShTreeAccessPolicy::storeSharePrefixes(setChildNos, s4);
   vec = setChildNos["0:"];
   verif0.push_back(2);
   test_diagnosis("testStoreSharePrefixes: " + s4 + "[0:]", verif0 == vec, errors);
   vec = setChildNos["0:1:"];
   test_diagnosis("testStoreSharePrefixes: " + s4 + "[0:1:]", verif01 == vec, errors);
   debugVector("vec [0:1:]", vec);
   debugVector("verif01", verif01);
   vector<int> verif02;
   vec = setChildNos["0:2:"];
   verif02.push_back(1);
   test_diagnosis("testStoreSharePrefixes: " + s4 + "[0:2:]", verif02 == vec, errors);

   //   REPORT("s5");
  ShTreeAccessPolicy::storeSharePrefixes(setChildNos, s5);
  vec = setChildNos["0:"];
  test_diagnosis("testStoreSharePrefixes: " + s5 + "[0:]", verif0 == vec, errors);
  vec = setChildNos["0:1:"];
  test_diagnosis("testStoreSharePrefixes: " + s5 + "[0:1:]", verif01 == vec, errors);
  vec = setChildNos["0:2:"];
  verif02.push_back(2);
  test_diagnosis("testStoreSharePrefixes: " + s4 + "[0:2:]", verif02 == vec, errors);
  vector<int> verif022;
  vec = setChildNos["0:2:2:"];
  verif022.push_back(1);
  test_diagnosis("testStoreSharePrefixes: " + s5 + "[0:2:2:]", verif022 == vec, errors);

  return errors;
}

int runTests(std::string &testName, PFC &pfc) {
  testName = "Test ShTreeAccessPolicy";
  int errors = 0;

  // Policy tests
  ENHOUT("Secret sharing policy tests");
  errors += testParseTreeFromExpression();
  errors += testSatisfyNode();
  errors += testEvaluate();
  errors += testGetNumShares();
  errors += testObtainCoveredFrags(); 
  
  ENHOUT("Secret sharing static utils tests");
  errors += testextractPrefixAndNoFromID();
  errors += testStoreSharePrefixes();
  errors += testUpdateSet();
  errors += testAddNewSet();
  errors += testPutShareInSet();
  errors += testGetSetPrefix();
  errors += testGetSharesForParticipants();
  errors += testExtractPublicInfoFromID();
  
  // Secret Sharing tests
  ENHOUT("Secret sharing scheme tests");
  errors += testLagrangeCoefficient(pfc);
  errors += testSmallDistributeAndReconstruct(pfc);
  errors += testDistributeAndReconstruct(pfc);

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
  int result = runTests(test_name, pfc);
  print_test_result(result,test_name);

  return 0;
}
