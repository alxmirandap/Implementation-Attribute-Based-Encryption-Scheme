#include "utils.h"
#include "BL.h"

class MockBLAccessPolicy : public BLAccessPolicy{

public:
  shared_ptr<TreeNode> mock_parsePolicy(){
    return parsePolicy();
  } 

  bool mock_satisfyNode(shared_ptr<TreeNode> node, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares) const{
    return satisfyNode(node, shares, satisfyingShares);
  }

  shared_ptr<TreeNode> mock_parseTreeFromExpression(std::string expr) {
    return parseTreeFromExpression(expr);
  }
};



int testParseTreeFromExpression() {
  int errors = 0;

  MockBLAccessPolicy testPolicy;

  std::string expr = "";
  shared_ptr<TreeNode> tree = testPolicy.mock_parseTreeFromExpression(expr);
  shared_ptr<TreeNode> emptyTree = TreeNode::makeTree(NodeContent::makeNILNode());
  test_diagnosis("testParseTreeFromExpression - [" + expr + "]", *tree == *emptyTree, errors);

  expr = "a1"; // literals must be integers, not strings
  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", true, errors);
  }

  expr =  op_OR + "(1,2,"; // missing closing )
  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", true, errors);
  }

  expr = op_AND + "(1,2,3),4"; // extra content beyond )
  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", true, errors);
  }


  expr = "1"; 
  shared_ptr<NodeContent> leafNode = NodeContent::makeLeafNode(1);
  // shared_ptr<TreeNode> rawTree = testPolicy.mock_parseTreeFromExpression(expr);
  // DEBUG("value of tree: " << rawTree);
  // DEBUG("tree-string: " << rawTree->to_string());
  
  // guard("Stopping", false);

  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    // DEBUG("leafNode: " << leafNode->to_string());
    // DEBUG("tree: " << tree->getNode()->to_string());
    // DEBUG("value of tree: " << tree);
    // DEBUG("tree-string: " << tree->to_string());
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", *(tree->getNode()) == *leafNode, errors);
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
  }

  expr = op_THR + "(3,1,2)"; // THR is not a recognized operator in BL
  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", true, errors);
  }

  expr = op_OR + "(1,2,3)"; 
  shared_ptr<NodeContent> orNode = NodeContent::makeOrNode(3);
  shared_ptr<NodeContent> leaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> leaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> leaf3 = NodeContent::makeLeafNode(3);

  shared_ptr<TreeNode> pTree = TreeNode::makeTree(orNode);
  pTree->appendChild(leaf1);
  pTree->appendChild(leaf2);
  pTree->appendChild(leaf3);

  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", (*pTree == *tree), errors);
    DEBUG("tree.toString: " << tree->to_string());
    DEBUG("pTree.toString: " << pTree->to_string());
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
    DEBUG("tree.toString: " << tree->to_string());
    DEBUG("pTree.toString: " << pTree->to_string());
    cerr << e.what() << std::endl;
  }


  expr = op_OR + "(1, " + op_OR + "(2, " + op_AND + "(1,2,3)), 4, " + op_AND + "(1,2))"; 
  shared_ptr<NodeContent> rootNode = NodeContent::makeOrNode(4);
  shared_ptr<NodeContent> orNode1 = NodeContent::makeOrNode(2);
  shared_ptr<NodeContent> andNode1 = NodeContent::makeAndNode(3);
  shared_ptr<NodeContent> andNode2 = NodeContent::makeAndNode(2);
  shared_ptr<NodeContent> baseleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> baseleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> baseleaf4 = NodeContent::makeLeafNode(4);
  shared_ptr<NodeContent> andleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> andleaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> andleaf3 = NodeContent::makeLeafNode(3);
  shared_ptr<NodeContent> orleaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> orleaf2 = NodeContent::makeLeafNode(2);

  shared_ptr<TreeNode> rootTree = TreeNode::makeTree(rootNode);
  shared_ptr<TreeNode> pTree2 = TreeNode::makeTree(orNode1);
  shared_ptr<TreeNode> pTree22 = TreeNode::makeTree(andNode1);
  shared_ptr<TreeNode> pTree4 = TreeNode::makeTree(andNode2);
  
  rootTree->appendChild(baseleaf1);
  pTree2->appendChild(baseleaf2);
  pTree22->appendChild(andleaf1);
  pTree22->appendChild(andleaf2);
  pTree22->appendChild(andleaf3);

  pTree4->appendChild(orleaf1);
  pTree4->appendChild(orleaf2);

  rootTree->appendTree(pTree2);
  rootTree->appendChild(baseleaf4);
  rootTree->appendTree(pTree4);
  pTree2->appendTree(pTree22);

  DEBUG("rootTree: " << rootTree->to_string());
  DEBUG("pTree2: " << pTree2->to_string());
  DEBUG("pTree22: " << pTree22->to_string());
  DEBUG("pTree4: " << pTree4->to_string());



  try {
    tree = testPolicy.mock_parseTreeFromExpression(expr);
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", *rootTree == *tree, errors);
    DEBUG("no exception branch");
    DEBUG("tree.toString: " << tree->to_string());
    DEBUG("pTree.toString: " << rootTree->to_string());
  } catch (std::exception &e) {
    test_diagnosis("testParseTreeFromExpression - [" + expr + "]", false, errors);
    DEBUG("exception branch!!!");
    DEBUG("tree.toString: " << tree->to_string());
    DEBUG("pTree.toString: " << rootTree->to_string());
    cerr << e.what() << std::endl;
  }

  return errors;
}

// this test tries several simple trees with different sets of shares. The argument of a share is its participant, identified by a simple integer.
// but a share has an implicit ID, defined by the policy that supports its issue. SatisfyNode verifies that the right shares are present, according to a policy.
// testEvaluate then has more complex policies

int testSatisfyNode() {
  int errors = 0;


  shared_ptr<NodeContent> nilNode = NodeContent::makeNILNode();
  shared_ptr<NodeContent> orNode = NodeContent::makeOrNode(3);
  shared_ptr<NodeContent> andNode = NodeContent::makeAndNode(2);
  shared_ptr<NodeContent> thrNode = NodeContent::makeThreshNode(4,3);

  shared_ptr<NodeContent> leaf1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> leaf2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> leaf3 = NodeContent::makeLeafNode(3);
  shared_ptr<NodeContent> leaf4 = NodeContent::makeLeafNode(4);

  std::shared_ptr<TreeNode> nilTree = TreeNode::makeTree(nilNode);
  std::shared_ptr<TreeNode> orTree = TreeNode::makeTree(orNode);
  std::shared_ptr<TreeNode> andTree = TreeNode::makeTree(andNode);
  std::shared_ptr<TreeNode> thrTree = TreeNode::makeTree(thrNode);

  orTree->appendChild(leaf1);
  orTree->appendChild(leaf2);
  orTree->appendChild(leaf3);
 
  andTree->appendChild(leaf3);
  andTree->appendChild(leaf4);

  thrTree->appendChild(leaf1);
  thrTree->appendChild(leaf2);
  thrTree->appendChild(leaf3);
  thrTree->appendChild(leaf4);

  ShareTuple o1(1, 0, "0:0");
  ShareTuple o2(2, 0, "0:1");
  ShareTuple o3(3, 0, "0:2");
 
  ShareTuple a3(3,0,"0:0");
  ShareTuple a4(4,0,"0:1");

  ShareTuple t1(1,0,"0:0");
  ShareTuple t2(2,0,"0:1");
  ShareTuple t3(3,0,"0:2");
  ShareTuple t4(4,0,"0:3");
  
  vector<ShareTuple> v_o1;
  v_o1.push_back(o1);

  vector<ShareTuple> v_o2;
  v_o2.push_back(o2);

  vector<ShareTuple> v_o3;
  v_o3.push_back(o3);

  vector<ShareTuple> v_o12;
  v_o12.push_back(o1);
  v_o12.push_back(o2);
  vector<ShareTuple> v_o13;
  v_o13.push_back(o1);
  v_o13.push_back(o3);

  vector<ShareTuple> v_a3;
  v_a3.push_back(a3);

  vector<ShareTuple> v_a4;
  v_a4.push_back(a4);

  vector<ShareTuple> v_a34;
  v_a34.push_back(a3);
  v_a34.push_back(a4);

  vector<ShareTuple> v_t1;
  v_t1.push_back(t1);

  vector<ShareTuple> v_t12;
  v_t12.push_back(t1);
  v_t12.push_back(t2);

  vector<ShareTuple> v_t123;
  v_t123.push_back(t1);
  v_t123.push_back(t2);
  v_t123.push_back(t3);

  vector<ShareTuple> v_t1234;
  v_t1234.push_back(t1);
  v_t1234.push_back(t2);
  v_t1234.push_back(t3);
  v_t1234.push_back(t4);

  vector<ShareTuple> v_oat;
  v_oat.push_back(o1);
  v_oat.push_back(t2);
  v_oat.push_back(a3);


  vector<ShareTuple> v_oa;
  v_oa.push_back(o3);
  v_oa.push_back(a4);

  vector<ShareTuple> v_tt;
  v_tt.push_back(t1);
  v_tt.push_back(t2);
  v_tt.push_back(t2);
  

  
  vector<ShareTuple> w_o1;
  w_o1.push_back(o1);

  vector<ShareTuple> w_o2;
  w_o2.push_back(o2);

  vector<ShareTuple> w_o3;
  w_o3.push_back(o3);

  vector<ShareTuple> w_o12;
  w_o12.push_back(o1);

  vector<ShareTuple> w_o13;
  w_o13.push_back(o1);


  vector<ShareTuple> w_a3;

  vector<ShareTuple> w_a4;

  vector<ShareTuple> w_a34;
  w_a34.push_back(a3);
  w_a34.push_back(a4);

  vector<ShareTuple> w_t1;
  w_t1.push_back(t1);

  vector<ShareTuple> w_t12;

  vector<ShareTuple> w_t123;
  w_t123.push_back(t1);
  w_t123.push_back(t2);
  w_t123.push_back(t3);

  vector<ShareTuple> w_t1234;
  w_t1234.push_back(t1);
  w_t1234.push_back(t2);
  w_t1234.push_back(t3);

  vector<ShareTuple> w_oat;

  vector<ShareTuple> w_oa;
  w_oa.push_back(o3);
  w_oa.push_back(a4);

  vector<ShareTuple> w_tt;
  
  vector<ShareTuple> witnessVector; 

  DEBUG("nil tree");  
  // NIL TREE
  bool b;
  bool verif_b;
  b = BLAccessPolicy::satisfyNode(nilTree, v_t1234, witnessVector);
  test_diagnosis("testSatisfyNode - [nilTree] [v_1234]", !b, errors);  
  test_diagnosis("testSatisfyNode - [nilTree] [v_1234] - witness vector", witnessVector.empty(), errors);
  witnessVector.clear();
   
  // OR TREE
  vector<vector<ShareTuple>> orTestList;
  orTestList.push_back(v_o1);
  orTestList.push_back(v_o2);  
  orTestList.push_back(v_o3);
  orTestList.push_back(v_o1);
  orTestList.push_back(v_o13);
  orTestList.push_back(v_oat);
  orTestList.push_back(v_oa);
  orTestList.push_back(v_tt);

  vector<vector<ShareTuple>> orWitnessList;
  orWitnessList.push_back(w_o1);
  orWitnessList.push_back(w_o2);  
  orWitnessList.push_back(w_o3);
  orWitnessList.push_back(w_o1);
  orWitnessList.push_back(w_o13);
  orWitnessList.push_back(w_o1);
  orWitnessList.push_back(w_o3);
  orWitnessList.push_back(w_t1);

  for (unsigned int i = 0; i < orTestList.size(); i++) {
    b = BLAccessPolicy::satisfyNode(orTree, orTestList[i], witnessVector);
    stringstream ss("");
    ss << "testSatisfyNode - [orTree]: " << i;
    if (orWitnessList[i].empty()) {
      verif_b = false;
    } else {
      verif_b = true;
    }
    test_diagnosis(ss.str(), b = verif_b, errors);
    test_diagnosis(ss.str() + " - Witness vector", witnessVector == orWitnessList[i], errors);
    
    ENHDEBUG("Checking vectors" << i);
    DEBUG("Sizes: " << witnessVector.size() << " - " << orWitnessList[i].size());
    for (unsigned int j = 0; j < witnessVector.size(); j++) {
      DEBUG("Witness: " << witnessVector[j].to_string());
      DEBUG("Returned: " << orWitnessList[i][j].to_string());
    }
    witnessVector.clear();
  }

  // AND TREE

  ENHDEBUG("AND POLICY");
  DEBUG("And tree: " << andTree->to_string());
  shared_ptr<TreeNode> child = andTree->getChild(0);
  shared_ptr<NodeContent> node = child->getNode();
  DEBUG("child 0 node: " << child->to_string());
  DEBUG("Node ID " << node->getNodeID());
  

  vector<vector<ShareTuple>> andTestList;
  andTestList.push_back(v_a3);
  andTestList.push_back(v_a4);  
  andTestList.push_back(v_a34);
  andTestList.push_back(v_oat);
  andTestList.push_back(v_oa);
  andTestList.push_back(v_tt);

  vector<vector<ShareTuple>> andWitnessList;
  andWitnessList.push_back(w_a3);
  andWitnessList.push_back(w_a4);  
  andWitnessList.push_back(w_a34);
  andWitnessList.push_back(w_oat);
  andWitnessList.push_back(w_oa);
  andWitnessList.push_back(w_tt);

  for (unsigned int i = 0; i < andTestList.size(); i++) {
    b = BLAccessPolicy::satisfyNode(andTree, andTestList[i], witnessVector);
    stringstream ss("");
    ss << "testSatisfyNode - [andTree]: " << i;
    if (andWitnessList[i].empty()) {
      verif_b = false;
    } else {
      verif_b = true;
    }
    test_diagnosis(ss.str(), b = verif_b, errors);
    test_diagnosis(ss.str() + " - Witness vector", witnessVector == andWitnessList[i], errors);
    
    ENHDEBUG("Checking vectors" << i);
    DEBUG("Sizes: " << witnessVector.size() << " - " << andWitnessList[i].size());
    for (unsigned int j = 0; j < witnessVector.size(); j++) {
      DEBUG("Witness: " << witnessVector[j].to_string());
      DEBUG("Returned: " << andWitnessList[i][j].to_string());
    }
    witnessVector.clear();
  }


  return errors;
}

int testEvaluate() {
  int errors = 0;



  return errors;
}

int runTests(std::string &testName) {
  testName = "Test BLAccessPolicy";
  int errors = 0;

  //  errors += testParseTreeFromExpression();
  errors += testSatisfyNode();
  //   errors += testEvaluate();
  return errors;
}

int main() {
  PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
  miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)

  mip->IOBASE=10;

  std::string test_name;
  int result = runTests(test_name);
  print_test_result(result,test_name);

  return 0;
}
