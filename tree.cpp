#include "utils.h"
#include "tree.h"

NodeContent::NodeContent(int leafValue):
  m_type(NodeContentType::leaf), m_leafValue(leafValue)
{}

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
 
NodeContent NodeContent::makeOrNode(int arity){
  NodeContent newNode = NodeContent(InnerNodeType::OR, arity);
  return newNode;
}

NodeContent NodeContent::makeAndNode(int arity){
  NodeContent newNode = NodeContent(InnerNodeType::AND, arity);
  return newNode;
}

NodeContent NodeContent::makeThreshNode(int arity, int threshold){
  NodeContent newNode = NodeContent(InnerNodeType::THR, arity, threshold);
  return newNode;
}

std::string NodeContent::to_string(){
  switch(m_type) {
  case NodeContentType::leaf:
    return std::to_string(m_leafValue); break;
  default: return "";
  case NodeContentType::inner:
    switch(m_innerNode.type) {
    case InnerNodeType::OR: return "OR[" + std::to_string(m_innerNode.arg1) + "]"; break;
    case InnerNodeType::AND: return "AND[" + std::to_string(m_innerNode.arg1) + "]"; break;
    case InnerNodeType::THR: return "THR[" + std::to_string(m_innerNode.arg1) + "," + std::to_string(m_innerNode.arg2) + "]"; break;
    default: return "";
    }
  }
}

//===================================================

TreeNode::TreeNode(NodeContent node):
  m_node(node)
{
  m_children.clear();
}

vector<TreeNode> TreeNode::getChildren(){
  return m_children;
}

void TreeNode::appendChild(NodeContent node){
  TreeNode newNode = TreeNode(node);
  m_children.push_back(newNode);
}

std::string TreeNode::to_string() {
  if (m_children.empty()) {
    return m_node.to_string();
  } 

  stringstream ss;
  ss << m_node.to_string() << "(";
  for (unsigned int i = 0; i < m_children.size(); i++) {
    ss << m_children[i].to_string();
    if (i < m_children.size()-1) {
      ss << ", ";
    }      
    ss << ")";
  }
  return ss.str();
}
