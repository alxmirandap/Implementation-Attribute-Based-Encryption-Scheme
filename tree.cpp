/*
 * tree.cpp
 *
 *  Created on: 18 Aug 2014
 *      Author: uxah005
 */

#ifndef DEF_TREE
#include "tree.h"
#endif

//NodeContent::nilNode = NodeContent();

NodeContent::NodeContent():
  m_type(NodeContentType::nil)
{}

NodeContent::NodeContent(int leafValue):
  m_type(NodeContentType::leaf)
{
  m_leafValue = leafValue;
}

NodeContent::NodeContent(InnerNodeType nodeType, int arg1):
  m_type(NodeContentType::inner)
{
  m_innerNode.type = nodeType;
  m_innerNode.arg1 = arg1;
}

NodeContent::NodeContent(InnerNodeType nodeType, int arg1, int arg2):
  m_type(NodeContentType::inner)
{
  m_innerNode.type = nodeType;
  m_innerNode.arg1 = arg1;
  m_innerNode.arg2 = arg2;
}

// static NodeContent& getNILNode(){
//   static NodeContent nilNode;
//   return nilNode;
// }

shared_ptr<NodeContent> NodeContent::makeNILNode(){
  NodeContent *newNode = new NodeContent();
  shared_ptr<NodeContent> pNode(newNode);
  return pNode;
}


shared_ptr<NodeContent> NodeContent::makeOrNode(int arity){
  NodeContent *newNode = new NodeContent(InnerNodeType::OR, arity);
  shared_ptr<NodeContent> pNode(newNode);
  return pNode;
}

shared_ptr<NodeContent> NodeContent::makeAndNode(int arity){
  NodeContent *newNode = new NodeContent(InnerNodeType::AND, arity);
  shared_ptr<NodeContent> pNode(newNode);
  return pNode;
}

shared_ptr<NodeContent> NodeContent::makeThreshNode(int arity, int threshold){
  if (arity >= threshold) {
    NodeContent *newNode = new NodeContent(InnerNodeType::THR, arity, threshold);
    shared_ptr<NodeContent> pNode(newNode);
    return pNode;
  } else {
    return NodeContent::makeNILNode();
  }
}

shared_ptr<NodeContent> NodeContent::makeLeafNode(int leafValue) {
  NodeContent *newNode = new NodeContent(leafValue);
  shared_ptr<NodeContent> pNode(newNode);
  return pNode;
}

bool NodeContent::operator==(const NodeContent& rhs) const {
  //  DEBUG("Checking type");
  if (m_type != rhs.m_type) return false;
  switch(m_type) {
  case(NodeContentType::inner):
    //    DEBUG("Checking inner type");
    if (m_innerNode.type != rhs.m_innerNode.type) return false;
    //    DEBUG("Checking arg1");
    if (m_innerNode.arg1 != rhs.m_innerNode.arg1) return false;
    if (m_innerNode.type == InnerNodeType::THR) {
      //      DEBUG("Checking arg2");
      if (m_innerNode.arg2 != rhs.m_innerNode.arg2) return false;
    }
    break;
  case(NodeContentType::leaf):
    //    DEBUG("Checking leaf value");
    //    DEBUG("my value: " << m_leafValue << " - other's value: " << rhs.m_leafValue);
    if (m_leafValue != rhs.m_leafValue) return false;
    //    DEBUG("Test passed");
    break;
  case(NodeContentType::nil):
    return true;
    break;
  }
  //  REPORT("returning true");
  return true;
}

std::string NodeContent::to_string(){
  switch(m_type) {
  case NodeContentType::leaf:
    return std::to_string(m_leafValue); break;
  case NodeContentType::inner:
    switch(m_innerNode.type) {
    case InnerNodeType::OR: return "OR[" + std::to_string(m_innerNode.arg1) + "]"; break;
    case InnerNodeType::AND: return "AND[" + std::to_string(m_innerNode.arg1) + "]"; break;
    case InnerNodeType::THR: return "THR[" + std::to_string(m_innerNode.arg1) + "," + std::to_string(m_innerNode.arg2) + "]"; break;
    }
  default: return "nil";
  }
}

NodeContentType NodeContent::getType() {
  return m_type;
}

int NodeContent::getLeafValue() {
  if (m_type != NodeContentType::leaf) {
    throw std::runtime_error("The function getLeafValue was called on a non-leaf node");
  }
  return m_leafValue;
}


InnerNodeType NodeContent::getInnerNodeType(){
  if (m_type != NodeContentType::inner) {
    throw std::runtime_error("The function getInnerNodeType was called on a non-inner node");
  }
  return m_innerNode.type;
}

unsigned int NodeContent::getArity() {
  if (m_type != NodeContentType::inner) {
    throw std::runtime_error("The function getArity was called on a non-inner node");
  }
  return m_innerNode.arg1;
}

unsigned int NodeContent::getThreshold(){
  //  DEBUG("inner type: " << m_innerNode.type);
  if ( (m_type != NodeContentType::inner) || (m_innerNode.type != InnerNodeType::THR) ) {
    throw std::runtime_error("The function getThreshold was called on a non-threshold node");
  }
  return m_innerNode.arg2;
}

//===================================================

TreeNode::TreeNode():
  m_node(NodeContent::makeNILNode()),
  m_nodeID("")
  //  ,m_parent(NULL)
{
  m_children.clear();
}

std::string TreeNode::initNodeID(shared_ptr<NodeContent> node) {
 switch (node->getType()) {
  case NodeContentType::nil:
    return ""; break;
  case NodeContentType::leaf:
    return "0:=" + convertIntToStr(node->getLeafValue());
    break;
  case NodeContentType::inner:
    return "0";
    break;
 default: return ""; //should never reach here
  }
 
}

TreeNode::TreeNode(shared_ptr<NodeContent> node):
  m_node(node)
{
  m_nodeID = initNodeID(node);
  m_children.clear();
}

shared_ptr<TreeNode> TreeNode::makeTree(shared_ptr<NodeContent> node){
  TreeNode *treeNode = new TreeNode(node);
  shared_ptr<TreeNode> pTree(treeNode);
  return pTree;
}

bool TreeNode::operator==(const TreeNode& rhs) const {
  if (!(*m_node == *rhs.m_node)) return false;
  // Does not check IDs. The reason is that two subtrees of different trees might still be equal if they have the same shape and elements, but due to their
  // different positions they will have different IDs.
  //  if (getNodeID() != rhs.getNodeID()) return false;
  if (m_children.size() != rhs.m_children.size()) return false;
  for (unsigned int i = 0; i < m_children.size(); i++) {
    if (!(*m_children[i] == *rhs.m_children[i])) return false;
  }
  return true;
}


TreeNode& TreeNode::operator=(const TreeNode& other)
{
  if (this == &other) return *this;
  m_node = other.m_node;
  m_nodeID = other.m_nodeID;
  m_children = other.m_children;  
  return *this;
}

shared_ptr<NodeContent> TreeNode::getNode(){
  return m_node;
}


std::shared_ptr<TreeNode> TreeNode::getChild(unsigned int i){  
  stringstream ss;
  ss << "Wrong child access. Size: " << m_children.size() << "; Requested index: " << i << endl;
  if (i >= m_children.size()) {
    throw std::range_error(ss.str());  
  }
  return m_children[i];
}

std::string TreeNode::findIDForNode(const std::string& parent, int childno, shared_ptr<NodeContent> node) {
  NodeContentType type = node->getType();
  std::string value;
  if (type == NodeContentType::leaf) {
    return findIDForNode(parent, childno, type, convertIntToStr(node->getLeafValue()));
  } else {
    return findIDForNode(parent, childno, type);
  }
}


std::string TreeNode::findIDForNode(const std::string& parent, int childno, NodeContentType type, const std::string& value) {
  if (type == NodeContentType::nil) {
    return "";
  }

  if (type == NodeContentType::leaf) {
    if (parent == "") {
      return ":=" + value;
    } else {
      return parent + ":" + convertIntToStr(childno) + ":=" + value;
    }
  }

  if (type == NodeContentType::inner) {
    return parent + ":" + convertIntToStr(childno);
  }

  return "";
}

bool TreeNode::appendChild(shared_ptr<NodeContent> node){
  //  DEBUG("appendChild entry point");
  switch (m_node->getType()){
  case NodeContentType::nil:
     {
       //       DEBUG("nil node");
       setNodeID(initNodeID(node));
       m_node = node;
     }
     break;
  case NodeContentType::leaf:
    {
      //      DEBUG("leaf node");
      throw std::runtime_error("An attempt was made to append a child to a leaf node");
    }
    break;
   case NodeContentType::inner:
     {
       //       DEBUG("Inner node");
       if (m_children.size() == m_node->getArity()) return false;

       shared_ptr<TreeNode> pNode = TreeNode::makeTree(node);
       m_children.push_back(pNode);
       int count = m_children.size()-1;
       
       std::string newId = findIDForNode(m_nodeID, count, node);
       //       DEBUG("New ID: " << newId);
       pNode->setNodeID(newId);
     }
     break;
   }
   return true;
}

bool TreeNode::appendTree(shared_ptr<TreeNode> pTree){
  switch (m_node->getType()){
  case NodeContentType::nil:
    m_node = pTree->m_node;
    m_children = pTree->m_children;
    setNodeID(initNodeID(m_node));    
    break;
  case NodeContentType::leaf:
    throw std::runtime_error("An attempt was made to append a tree to a leaf node");
    break;
  case NodeContentType::inner:
    if (m_children.size() == m_node->getArity()) return false;
    
    m_children.push_back(pTree);
    pTree->updateID(getNodeID(), m_children.size()-1);
    break;
  }
  return true;
}


void TreeNode::updateID(std::string parentID, int count){
  // this is only called after an insert. The tree on which this is called is a sub-tree of someone else.
  // this tree might be a free hanging leaf or a whole tree
  // in any case, the logic is the same: whatever is before the first ":" is replaced by parentID + count. There is no need to treat leaves separately
  // Examples: 
  // a tree with a leaf 0:1:=7 is added to a tree 0:2 at position 3. ---> 0:2:3:1:=7
  // --- a rootleaf 0:=7 is added to the same tree and position. ---> 0:2:3:=7

  std::string newID;
  std::string currentID = getNodeID();
  int index = currentID.find(":");
  std::string tailID;
  if (index == -1) {
    tailID = "";
  } else {
    tailID = currentID.substr(index);
  }
  newID = parentID + ":" + convertIntToStr(count) + tailID;
  //  DEBUG("Setting new ID: " << newID);
  setNodeID(newID);

  for (unsigned int i = 0; i < getNumChildren(); i++) {
    getChild(i)->updateID(parentID, count);
  }
}

std::string TreeNode::full_to_string() {
  if (m_children.empty()) {
    return "{" +  getNodeID() + " || " + m_node->to_string() + "}";
  }

  stringstream ss;
  ss << m_node->to_string() << "(";
  for (unsigned int i = 0; i < getNumChildren(); i++) {
    ss << "{" + getNodeID() + " || " + getChild(i)->to_string() + "}";
    if (i < m_children.size()-1) {
      ss << ", ";
    }
  }
  ss << ")";
  return ss.str();
}

std::string TreeNode::to_string() {
  if (m_children.empty()) {
    return m_node->to_string();
  }

  stringstream ss;
  ss << m_node->to_string() << "(";
  for (unsigned int i = 0; i < getNumChildren(); i++) {
    ss <<getChild(i)->to_string();
    if (i < m_children.size()-1) {
      ss << ", ";
    }
  }
  ss << ")";
  return ss.str();
}

unsigned int TreeNode::getNumLeaves(){
  if (m_node->getType() == NodeContentType::leaf) {
    return 1;
  }
  int nleaves = 0;
  for (unsigned int i = 0; i < m_children.size(); i++) {
    int subleaves = getChild(i)->getNumLeaves();
    nleaves += subleaves;
  }
  return nleaves;
}


unsigned int TreeNode::getNumChildren(){
  return m_children.size();
}


std::string TreeNode::getNodeID() const{
  return m_nodeID;
}

void TreeNode::setNodeID(const std::string nodeID){
  m_nodeID = nodeID;
}

bool TreeNode::isLeaf() {
  return (m_node->getType() == NodeContentType::leaf);
}

bool TreeNode::isInner() {
  return (m_node->getType() == NodeContentType::inner);
}

bool TreeNode::isNil() {
  return (m_node->getType() == NodeContentType::nil);
}
