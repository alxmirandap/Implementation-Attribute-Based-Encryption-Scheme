/*
 * tree.h
 *
 *  Created on: 18 Aug 2014
 *      Author: uxah005
 */

#ifndef DEF_TREE
#define DEF_TREE

#ifndef DEF_UTILS
#include "utils.h"
#endif

enum InnerNodeType {OR, AND, THR};
enum NodeContentType {inner, leaf, nil};


class NodeContent {
  NodeContent();
  NodeContent(int leafValue);
  NodeContent(InnerNodeType nodeType, int arg1);
  NodeContent(InnerNodeType nodeType, int arg1, int arg2);

  NodeContentType m_type;
  std::string m_nodeID;
  // this uniquely identifies each node in the tree. A simple scheme is to describe the path to root.
  // Each node concatenates ":" and its position within children to the Id of its parent. For example, root would be "0". Immediate children of root would be
  // "0:0", "0:1", "0:2". Children of the second child would be "0:1:0", "0:1:1" and so on.

  union{
    int m_leafValue;
    struct{ // inner nodes usually need some more values besides their type
      InnerNodeType type;
      int arg1; // This represents arity. It is used for AND, OR and Threshold nodes.
      int arg2; // This represents threshold. It is used only for threshold nodes.
    } m_innerNode;
  };

 public:
  static std::shared_ptr<NodeContent> makeNILNode();
  static std::shared_ptr<NodeContent> makeOrNode(int arity);
  static std::shared_ptr<NodeContent> makeAndNode(int arity);
  static std::shared_ptr<NodeContent> makeThreshNode(int arity, int threshold);
  static std::shared_ptr<NodeContent> makeLeafNode(int leafValue);
  //  static NodeContent* makeLeafNodeRaw(int leafValue);

  bool operator==(const NodeContent& rhs) const;

  NodeContentType getType();
  int getLeafValue();
  std::string getNodeID();
  void setNodeID(std::string nodeID);

  InnerNodeType getInnerNodeType();
  unsigned int getThreshold();
  unsigned int getArity();

  string to_string();
};

//===========================================

class TreeNode {
  //NodeContent &m_node;
  std::shared_ptr<NodeContent> m_node;
  vector<std::shared_ptr<TreeNode>> m_children; // vector of pointers for TreeNodes.
  TreeNode(std::shared_ptr<NodeContent> node);

 public:
  TreeNode();
  static std::shared_ptr<TreeNode> makeTree(std::shared_ptr<NodeContent> node);
  std::shared_ptr<NodeContent> getNode();
  //  vector<TreeNode>& getChildren();
  std::shared_ptr<TreeNode> getChild(unsigned int i);
  bool operator==(const TreeNode& rhs) const;
  TreeNode& operator=(const TreeNode& rhs);
  bool appendChild(std::shared_ptr<NodeContent> node);
  bool appendTree(std::shared_ptr<TreeNode> tree);
  std::string to_string();
  unsigned int getNumLeaves();
  unsigned int getNumChildren();
  unsigned int getNumLeavesDEBUG(int level);
  void updateID(std::string parentID, int count);
};


#endif /* TREE_H_ */
