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
  m_type(NodeContentType::nil),
  m_nodeID("0")
{}

NodeContent::NodeContent(int leafValue):
  m_type(NodeContentType::leaf),
  m_nodeID("0")
{
  m_leafValue = leafValue;
}

NodeContent::NodeContent(InnerNodeType nodeType, int arg1):
  m_type(NodeContentType::inner),
  m_nodeID("0")
{
  m_innerNode.type = nodeType;
  m_innerNode.arg1 = arg1;
}

NodeContent::NodeContent(InnerNodeType nodeType, int arg1, int arg2):
  m_type(NodeContentType::inner),
  m_nodeID("0")
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

// NodeContent* NodeContent::makeLeafNodeRaw(int leafValue) {
//   NodeContent* newNode = new NodeContent(leafValue);
//   return newNode;
// }

bool NodeContent::operator==(const NodeContent& rhs) const {
  // id should not be compared, as it is not intrinsic to the object. It just reflects the Node's position in a tree
  //  if (m_nodeID != rhs.m_nodeID) return false;
  if (m_type != rhs.m_type) return false;
  switch(m_type) {
  case(NodeContentType::inner):
    if (m_innerNode.type != rhs.m_innerNode.type) return false;
    if (m_innerNode.arg1 != rhs.m_innerNode.arg1) return false;
    if (m_innerNode.type == InnerNodeType::THR) {
      if (m_innerNode.arg2 != rhs.m_innerNode.arg2) return false;
    }
    break;
  case(NodeContentType::leaf):
    if (m_leafValue != rhs.m_leafValue) return false;
    break;
  case(NodeContentType::nil):
    return true;
    break;
  }
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

std::string NodeContent::getNodeID(){
  return m_nodeID;
}

void NodeContent::setNodeID(std::string nodeID){
  m_nodeID = nodeID;
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
  m_node(NodeContent::makeNILNode())
{

  m_children.clear();
}


TreeNode::TreeNode(shared_ptr<NodeContent> node):
  m_node(node)
{
  m_children.clear();
}

shared_ptr<TreeNode> TreeNode::makeTree(shared_ptr<NodeContent> node){
  TreeNode *treeNode = new TreeNode(node);
  shared_ptr<TreeNode> pTree(treeNode);
  return pTree;
}

bool TreeNode::operator==(const TreeNode& rhs) const {
  //  DEBUG("testing node");
  if (!(*m_node == *rhs.m_node)) return false;
  //  DEBUG("testing children size");
  if (m_children.size() != rhs.m_children.size()) return false;
  for (unsigned int i = 0; i < m_children.size(); i++) {
    //    DEBUG("testing child " << i);
    if (!(*m_children[i] == *rhs.m_children[i])) return false;
  }
  //  DEBUG("success: returning true");
  return true;
}


TreeNode& TreeNode::operator=(const TreeNode& other)
{
  m_node = other.m_node;
  m_children = other.m_children;
  return *this;
}


shared_ptr<NodeContent> TreeNode::getNode(){
  return m_node;
}

// vector<TreeNode>& TreeNode::getChildren(){
//   return m_children;
// }

std::shared_ptr<TreeNode> TreeNode::getChild(unsigned int i){
  stringstream ss;
  ss << "Wrong child access. Size: " << m_children.size() << "; Requested index: " << i;
  guard(ss.str(), i < m_children.size());
  //  DEBUG("Address of returned child: " << m_children[i]);
  //  DEBUG("Address of returned child2: " << &(*m_children[i]));
  return m_children[i];
}


bool TreeNode::appendChild(shared_ptr<NodeContent> node){
  switch (m_node->getType()){
  case NodeContentType::nil:
    DEBUG("nil node");
    m_node = node; break;
    node->setNodeID("0");
  case NodeContentType::leaf:
    DEBUG("leaf node");
    throw std::runtime_error("An attempt was made to append a child to a leaf node");
    break;
  case NodeContentType::inner:
    //  DEBUG("Inner node");
    if (m_children.size() == m_node->getArity()) return false;

    //    TreeNode newNode(node);
    shared_ptr<TreeNode> pNode = TreeNode::makeTree(node);
    //    ENHDEBUG("Appending tree node: " << &newNode);
    m_children.push_back(pNode);
    int count = m_children.size()-1;
    stringstream ss;
    ss << m_node->getNodeID() << ":" << count;
    node->setNodeID(ss.str());
    break;
  }
  //  DEBUG("out of switch");
  return true;
}

bool TreeNode::appendTree(shared_ptr<TreeNode> pTree){
  switch (m_node->getType()){
  case NodeContentType::nil:
    m_node = pTree->m_node;
    m_children = pTree->m_children;
  case NodeContentType::leaf:
    throw std::runtime_error("An attempt was made to append a tree to a leaf node");
    break;
  case NodeContentType::inner:
    if (m_children.size() == m_node->getArity()) return false;
    
    //    ENHDEBUG("Appending tree node: " << &newNode);
    m_children.push_back(pTree);
    //     shared_ptr<TreeNode> storedNode = getChild(m_children.size()-1);
    //     ENHDEBUG("Address in vector: " << &storedNode);
    //     ENHDEBUG("Address in vector without &: " << storedNode);
    //     int count = m_children.size()-1;
    pTree->updateID(m_node->getNodeID(), m_children.size()-1);
    break;
  }
  return true;
}


void TreeNode::updateID(std::string parentID, int count){
  std::string currentID = m_node->getNodeID();
  int index = currentID.find(":");
  std::string tailID;
  if (index == -1) {
    tailID = "";
  } else {
    tailID = currentID.substr(index);
  }

  stringstream ss;
  ss << parentID << ":" << count << tailID;
  m_node->setNodeID(ss.str());

  for (unsigned int i = 0; i < getNumChildren(); i++) {
    getChild(i)->updateID(parentID, count);
  }
}

std::string TreeNode::to_string() {
  if (m_children.empty()) {
    return m_node->to_string();
  }

  stringstream ss;
  ss << m_node->to_string() << "(";
  for (unsigned int i = 0; i < getNumChildren(); i++) {
    ss << getChild(i)->to_string();
    if (i < m_children.size()-1) {
      ss << ", ";
    }
  }
  ss << ")";
  return ss.str();
}

unsigned int TreeNode::getNumLeaves(){
  if (m_node->getType() == NodeContentType::leaf) {
    //    OUT(m_node.getNodeID() << " --- getNumLeaves: this is a leaf, returning 1");
    return 1;
  }
  //  DEBUG(m_node.getNodeID() << " --- getNumLeaves: not a leaf, descending");
  int nleaves = 0;
  for (unsigned int i = 0; i < m_children.size(); i++) {
    int subleaves = getChild(i)->getNumLeaves();
    nleaves += subleaves;
    //    DEBUG(m_node.getNodeID() << " --- Current leaves count: " << nleaves);
  }
  return nleaves;
}


// unsigned int TreeNode::getNumLeavesDEBUG(int level){
//   ENHDEBUG("////////////////////////");
//   ENHDEBUG("entry point. level: " << level);
//   DEBUG("getNumLeaves for node: " << &m_node  << " || Level: " << level);
//   if (m_node->getType() == NodeContentType::leaf) {
//     DEBUG("Leaf: return 1");
//     return 1;
//   }
//   DEBUG("Inner node. Num Children: " << m_children.size());
//   int nleaves = 0;
//   for (unsigned int i = 0; i < m_children.size(); i++) {
//     DEBUG("CALLING AGAIN");
//     int subleaves = getChild(i)->getNumLeavesDEBUG(level + 1);
//     DEBUG("num leaves in child: " << i << " = " << subleaves);
//     DEBUG("----------------");
//     nleaves += subleaves;
//   }
//   ENHDEBUG("Total leaves in level: " << level << " = " << nleaves);
//   DEBUG("================");
//   return nleaves;
// }



unsigned int TreeNode::getNumChildren(){
  return m_children.size();
}


