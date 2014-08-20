#ifndef DEF_TREE
#include "tree.h"
#endif


  std::string message;
  std::string base;
  int idata;
  std::string sdata;
  InnerNodeType tdata;


int testNode(int errors, shared_ptr<NodeContent> node, std::string base, NodeContentType type, std::string nodeID,
	     bool excepLeafValue, bool excepInnerType, bool excepArity, bool excepThreshold,
	     int leafValue = 0, InnerNodeType innerType = InnerNodeType::OR, int arity = 0, int threshold = 0) {

  //  DEBUG("testNode invoked: " << base);

  //  DEBUG("testNode errors: " << errors);
  message = base + "type";
  test_diagnosis(message, node->getType() == type, errors);

  message = base + "getNodeID";
  //  DEBUG("getNodeID: " << node->getNodeID());
  test_diagnosis(message, node->getNodeID() == nodeID, errors);

  try {
    message = base + "getLeafValue";
    idata = node->getLeafValue();
    if (excepLeafValue) {
      test_diagnosis(message + " should have thrown exception", false, errors);
    } else {
      test_diagnosis(message + " should have returned right value", idata == leafValue, errors);
    }
  } catch (std::exception &e) {
    if (excepLeafValue) test_diagnosis(message + " should have thrown exception", true, errors);
  }

  try {
    message = base + "getInnerType";
    tdata = node->getInnerNodeType();
    if (excepInnerType) {
      test_diagnosis(message + " should have thrown exception", false, errors);
    } else {
      test_diagnosis(message + " should have returned right value", tdata == innerType, errors);
    }
  } catch (std::exception &e) {
    if (excepInnerType) test_diagnosis(message + " should have thrown exception", true, errors);
  }

  try {
    message = base + "getArity";
    idata = node->getArity();
    if (excepArity) {
      test_diagnosis(message + " should have thrown exception", false, errors);
    } else {
      test_diagnosis(message + " should have returned right value", idata == arity, errors);
    }
  } catch (std::exception &e) {
    if (excepArity) test_diagnosis(message + " should have thrown exception", true, errors);
  }

  try {
    //    DEBUG("node: " << node->to_string());
    message = base + "getThreshold";
    //    DEBUG("calling threshold");
    idata = node->getThreshold();
    //    DEBUG("Returned: " << idata);
    if (excepThreshold) {
      test_diagnosis(message + " should have thrown exception", false, errors);
    } else {
      test_diagnosis(message + " should have returned right value", idata == threshold, errors);
    }
  } catch (std::exception &e) {
    //    DEBUG("Exception thrown");
    if (excepThreshold) test_diagnosis(message + " should have thrown exception", true, errors);
  }

  message = base + "Threshold vs Arity";
  //DEBUG("Maybe throwing exception?");
  if ( (!excepArity) && (!excepThreshold) ) {
    //DEBUG("Calling threshold and Arity");
    test_diagnosis(message, node->getThreshold() <= node->getArity(), errors);
  }

  //  DEBUG("testNode finished: ");
  //  DEBUG("-------------------");
  //  DEBUG("exitting testNode errors: " << errors);
  return errors;
}

int testResets() {
  int errors = 0;
  base = "Test Reset - [OR]: ";
  shared_ptr<NodeContent> orNode = NodeContent::makeOrNode(5);
  orNode.reset();
  test_diagnosis(base, true, errors);

  base = "Test Reset - [AND]: ";
  shared_ptr<NodeContent> andNode = NodeContent::makeAndNode(12);
  andNode.reset();
  test_diagnosis(base, true, errors);

  base = "Test Reset - [THR]: ";
  shared_ptr<NodeContent> thrNode = NodeContent::makeThreshNode(7,3);
  thrNode.reset();
  test_diagnosis(base, true, errors);

  base = "Test Reset - [Leaf]: ";
  shared_ptr<NodeContent> leafNode = NodeContent::makeLeafNode(15);
  leafNode.reset();
  test_diagnosis(base, true, errors);

  base = "Test Reset - [Nil]: ";
  shared_ptr<NodeContent> nilNode = NodeContent::makeNILNode();
  nilNode.reset();
  test_diagnosis(base, true, errors);

  return errors;
}

int testMakeNode() {
  int errors = 0;

  base = "Test MakeNode - [Nil]: ";
  shared_ptr<NodeContent> nilNode = NodeContent::makeNILNode();
  errors = testNode(errors, nilNode, base, NodeContentType::nil, "0", true, true, true, true);
  nilNode.reset();

  base = "Test MakeNode - [OR]: ";
  shared_ptr<NodeContent> orNode = NodeContent::makeOrNode(5);
  errors = testNode(errors, orNode, base, NodeContentType::inner, "0", true, false, false, true, 0, InnerNodeType::OR, 5);
  orNode.reset();

  base = "Test MakeNode - [AND]: ";
  shared_ptr<NodeContent> andNode = NodeContent::makeAndNode(12);
  errors = testNode(errors, andNode, base, NodeContentType::inner, "0", true, false, false, true, 0, InnerNodeType::AND, 12);
  andNode.reset();

  base = "Test MakeNode - [THR]: ";
  shared_ptr<NodeContent> thrNode = NodeContent::makeThreshNode(7,3);
  errors = testNode(errors, thrNode, base, NodeContentType::inner, "0", true, false, false, false, 0, InnerNodeType::THR, 7, 3);
  thrNode.reset();

  base = "Test MakeNode - [Bad THR]: ";
  shared_ptr<NodeContent> badthrNode = NodeContent::makeThreshNode(7,13);
  errors = testNode(errors, badthrNode, base, NodeContentType::nil, "0", true, true, true, true);
  badthrNode.reset();

  base = "Test MakeNode - [Leaf]: ";
  shared_ptr<NodeContent> leafNode = NodeContent::makeLeafNode(15);
  errors = testNode(errors, leafNode, base, NodeContentType::leaf, "0", false, true, true, true, 15);
  leafNode.reset();

  //  ENHDEBUG("End of testMakeNode");

  return errors;
}


int testChildren(){  //append and get numbers
  int errors = 0;

  shared_ptr<NodeContent> root = NodeContent::makeAndNode(3);
  shared_ptr<TreeNode> tree = TreeNode::makeTree(root);

  test_diagnosis("testChildren - root", root == tree->getNode(), errors);
  test_diagnosis("testChildren - root ID", root->getNodeID() == "0", errors);
  test_diagnosis("testChildren - root numChildren", tree->getNumChildren() == 0, errors);

  shared_ptr<NodeContent> child1 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> child2 = NodeContent::makeLeafNode(2);
  shared_ptr<NodeContent> child3 = NodeContent::makeLeafNode(3);
  shared_ptr<NodeContent> child4 = NodeContent::makeLeafNode(4);

  test_diagnosis("testChildren - child1 ID", child1->getNodeID() == "0", errors);
  test_diagnosis("testChildren - child1 append", tree->appendChild(child1) == true, errors);
  test_diagnosis("testChildren - child1 ID", child1->getNodeID() == "0:0", errors);
  test_diagnosis("testChildren - child1 numChildren", tree->getNumChildren() == 1, errors);

  test_diagnosis("testChildren - child2 ID", child2->getNodeID() == "0", errors);
  test_diagnosis("testChildren - child2 append", tree->appendChild(child2) == true, errors);
  test_diagnosis("testChildren - child2 ID", child2->getNodeID() == "0:1", errors);
  test_diagnosis("testChildren - child2 numChildren", tree->getNumChildren() == 2, errors);

  test_diagnosis("testChildren - child3 ID", child3->getNodeID() == "0", errors);
  test_diagnosis("testChildren - child3 append", tree->appendChild(child3) == true, errors);
  test_diagnosis("testChildren - child3 ID", child3->getNodeID() == "0:2", errors);
  test_diagnosis("testChildren - child3 numChildren", tree->getNumChildren() == 3, errors);

  test_diagnosis("testChildren - child4 ID", child4->getNodeID() == "0", errors);
  test_diagnosis("testChildren - child4 append", tree->appendChild(child4) == false, errors);
  test_diagnosis("testChildren - child4 ID", child4->getNodeID() == "0", errors);
  test_diagnosis("testChildren - child4 numChildren", tree->getNumChildren() == 3, errors); // can not add more than arity nodes

  shared_ptr<TreeNode> leafTree = TreeNode::makeTree(child3);

  try {
    test_diagnosis("testChildren - leaf node append", leafTree->appendChild(child4) == false, errors); // can not add children to a leaf node
    test_diagnosis("testChildren - leaf node append: exception thrown", false, errors);
  } catch (std::exception &e) {
    test_diagnosis("testChildren - leaf node append: exception thrown", true, errors);
  }
  test_diagnosis("testChildren - child3 numChildren", leafTree->getNumChildren() == 0, errors);

  return errors;
}

// Important notice:
// the TreeNode object stores an array of pointers to its subtrees. This is so that, after the creation of the tree and the insertion of elements into it,
// we can access a subtree from an outside function and still be confident we have the object that is stored, and not a copy of it.
// it is important to not make shortcuts when we do this. If we obtain a pointer to an object and store its content in a variable of the correct type,
// we are in effect creating a copy and losing the pointer behaviour. that means that if we then alter directly the copy we just used we are not making changes
// to the object stored in the original TreeNode, which is probably the wrong behaviour.

// for an example, notice the following code:
// int x = 12;
// int* y = &x;

// *y = 21;

// int z = *y;

// std::cout << "x: " << x << std::endl;
// std::cout << "y: " << *y << std::endl;
// std::cout << "z: " << z << std::endl;

// *y = 32;

// std::cout << "x: " << x << std::endl;
// std::cout << "y: " << *y << std::endl;
// std::cout << "z: " << z << std::endl;
// this returns
// x: 21
// y: 21
// z: 21
// x: 32
// y: 32
// z: 21


int testTreeConstruction(){ // computing the number of leaves and checking the general construction of the tree
  int errors = 0;

  shared_ptr<NodeContent> root = NodeContent::makeOrNode(2);

  shared_ptr<NodeContent> node1 = NodeContent::makeLeafNode(12);
  shared_ptr<NodeContent> node2 = NodeContent::makeAndNode(3);

  shared_ptr<NodeContent> node21 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> node22 = NodeContent::makeThreshNode(2,1);
  shared_ptr<NodeContent> node23 = NodeContent::makeOrNode(3);

  shared_ptr<NodeContent> node221 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> node222 = NodeContent::makeLeafNode(2);

  shared_ptr<NodeContent> node231 = NodeContent::makeLeafNode(1);
  shared_ptr<NodeContent> node232 = NodeContent::makeLeafNode(10);
  shared_ptr<NodeContent> node233 = NodeContent::makeLeafNode(100);

  shared_ptr<TreeNode> tree = TreeNode::makeTree(root);
  shared_ptr<TreeNode> verifTree1 = TreeNode::makeTree(node1);
  shared_ptr<TreeNode> verifTree2 = TreeNode::makeTree(node2);

  DEBUG("append node1");
  tree->appendChild(node1);
  test_diagnosis("testConstruction - nodeID [node1]", node1->getNodeID() == "0:0", errors);
  DEBUG("append node2");
  tree->appendChild(node2);
  test_diagnosis("testConstruction - nodeID [node2]", node2->getNodeID() == "0:1", errors);

  shared_ptr<TreeNode> subtree1 = tree->getChild(0);
  shared_ptr<TreeNode> subtree2 = tree->getChild(1);

  test_diagnosis("testConstruction - getChildren() - subtree1", *verifTree1 == *subtree1, errors);
  test_diagnosis("testConstruction - getChildren() - subtree2", *verifTree2 == *subtree2, errors);
  test_diagnosis("testConstruction - getNode() - subtree1", node1 == subtree1->getNode(), errors);
  test_diagnosis("testConstruction - getNode() - subtree2", node2 == subtree2->getNode(), errors);

  DEBUG("append node21");
  subtree2->appendChild(node21);
  test_diagnosis("testConstruction - nodeID [node21]", node21->getNodeID() == "0:1:0", errors);

  DEBUG("append node22");
  subtree2->appendChild(node22);
  test_diagnosis("testConstruction - nodeID [node22]", node22->getNodeID() == "0:1:1", errors);

  DEBUG("append node23");
  subtree2->appendChild(node23);
  test_diagnosis("testConstruction - nodeID [node23]", node23->getNodeID() == "0:1:2", errors);

  shared_ptr<TreeNode> subtree21 = subtree2->getChild(0);
  test_diagnosis("testConstruction - getNode() - subtree21", node21 == subtree21->getNode(), errors);
  shared_ptr<TreeNode> subtree22 = subtree2->getChild(1); // storing the pointee of this pointer in a new variable creates a new copy. Adding to this copy is not
  // reflected then in the object pointed by the pointer in the vectro
  // therefore, the definition TreeNode subtree22 = *subtree2.getChild(1); would lead to errors.
  test_diagnosis("testConstruction - getNode() - subtree22", node22 == subtree22->getNode(), errors);
  shared_ptr<TreeNode> subtree23 = subtree2->getChild(2);
  test_diagnosis("testConstruction - getNode() - subtree23", node23 == subtree23->getNode(), errors);

  DEBUG("append node221");
  subtree22->appendChild(node221);
  test_diagnosis("testConstruction - nodeID [node221]", node221->getNodeID() == "0:1:1:0", errors);

  DEBUG("append node222");
  subtree22->appendChild(node222);
  test_diagnosis("testConstruction - nodeID [node222]", node222->getNodeID() == "0:1:1:1", errors);

  DEBUG("append node231");
  subtree23->appendChild(node231);
  test_diagnosis("testConstruction - nodeID [node231]", node231->getNodeID() == "0:1:2:0", errors);

  DEBUG("append node232");
  subtree23->appendChild(node232);
  test_diagnosis("testConstruction - nodeID [node232]", node232->getNodeID() == "0:1:2:1", errors);

  DEBUG("append node233");
  subtree23->appendChild(node233);
  test_diagnosis("testConstruction - nodeID [node233]", node233->getNodeID() == "0:1:2:2", errors);

  shared_ptr<TreeNode> subtree221 = subtree22->getChild(0);
  test_diagnosis("testConstruction - getNode() - subtree221", node221 == subtree221->getNode(), errors);
  shared_ptr<TreeNode> subtree222 = subtree22->getChild(1);
  test_diagnosis("testConstruction - getNode() - subtree222", node222 == subtree222->getNode(), errors);

  shared_ptr<TreeNode> subtree231 = subtree23->getChild(0);
  test_diagnosis("testConstruction - getNode() - subtree231", node231 == subtree231->getNode(), errors);
  shared_ptr<TreeNode> subtree232 = subtree23->getChild(1);
  test_diagnosis("testConstruction - getNode() - subtree232", node232 == subtree232->getNode(), errors);
  shared_ptr<TreeNode> subtree233 = subtree23->getChild(2);
  test_diagnosis("testConstruction - getNode() - subtree233", node233 == subtree233->getNode(), errors);

  DEBUG("====================");

  test_diagnosis("Test - Num Leaves: tree", tree->getNumLeaves() == 7, errors);
  test_diagnosis("Test - Num Leaves: subtree1", subtree1->getNumLeaves() == 1, errors);

  ENHDEBUG("subtree2 children: " << subtree2->getNumChildren());
  ENHDEBUG("subtree21 children: " << subtree2->getChild(0)->getNumChildren());
  ENHDEBUG("subtree21 children: " << subtree21->getNumChildren());
  ENHDEBUG("subtree22 children: " << subtree2->getChild(1)->getNumChildren());
  ENHDEBUG("subtree22 children: " << subtree22->getNumChildren());
  ENHDEBUG("subtree23 children: " << subtree2->getChild(2)->getNumChildren());
  ENHDEBUG("subtree23 children: " << subtree23->getNumChildren());

  ENHDEBUG("Addresses: subtree22 real: " << &subtree22);
  ENHDEBUG("Addresses: subtree22 from getchildren: " << subtree2->getChild(1));

  //  DEBUG("subtree2 num leaves: " << subtree2->getNumLeavesDEBUG(0));
  //  DEBUG("subtree2 num leaves: " << subtree23->getNumLeavesDEBUG(10));
  test_diagnosis("Test - Num Leaves: subtree2", subtree2->getNumLeaves() == 6, errors);
  test_diagnosis("Test - Num Leaves: subtree21", subtree21->getNumLeaves() == 1, errors);
  test_diagnosis("Test - Num Leaves: subtree22", subtree22->getNumLeaves() == 2, errors);
  test_diagnosis("Test - Num Leaves: subtree23", subtree23->getNumLeaves() == 3, errors);
  test_diagnosis("Test - Num Leaves: subtree221", subtree221->getNumLeaves() == 1, errors);
  test_diagnosis("Test - Num Leaves: subtree222", subtree222->getNumLeaves() == 1, errors);
  test_diagnosis("Test - Num Leaves: subtree231", subtree231->getNumLeaves() == 1, errors);
  test_diagnosis("Test - Num Leaves: subtree232", subtree232->getNumLeaves() == 1, errors);
  test_diagnosis("Test - Num Leaves: subtree233", subtree233->getNumLeaves() == 1, errors);

  test_diagnosis("Test - Num Children: tree", tree->getNumChildren() == 2, errors);
  test_diagnosis("Test - Num Children: subtree1", subtree1->getNumChildren() == 0, errors);
  test_diagnosis("Test - Num Children: subtree2", subtree2->getNumChildren() == 3, errors);
  test_diagnosis("Test - Num Children: subtree21", subtree21->getNumChildren() == 0, errors);
  test_diagnosis("Test - Num Children: subtree22", subtree22->getNumChildren() == 2, errors);
  test_diagnosis("Test - Num Children: subtree23", subtree23->getNumChildren() == 3, errors);
  test_diagnosis("Test - Num Children: subtree221", subtree221->getNumChildren() == 0, errors);
  test_diagnosis("Test - Num Children: subtree222", subtree222->getNumChildren() == 0, errors);
  test_diagnosis("Test - Num Children: subtree231", subtree231->getNumChildren() == 0, errors);
  test_diagnosis("Test - Num Children: subtree232", subtree232->getNumChildren() == 0, errors);
  test_diagnosis("Test - Num Children: subtree233", subtree233->getNumChildren() == 0, errors);


  return errors;
}

int testAppendTree(){
  int errors = 0;

  test_diagnosis("Not implemented", false, errors);

  return errors;
}

int runTests(std::string &testName) {
  testName = "Test Tree";
  int errors = 0;

  errors += testResets();
  errors += testMakeNode();
  errors += testChildren();
  errors += testTreeConstruction();
  errors += testAppendTree();

  DEBUG("Returning from runTests");

  return errors;
}


int main() {
  std::string test_name;
  int result = runTests(test_name);

  DEBUG("Printing testResults");
  print_test_result(result,test_name);

  return 0;
}
