enum InnerNodeType {OR, AND, THR};
enum NodeContentType {inner, leaf};

class NodeContent {
 private:
  NodeContentType m_type;
  union{
    int m_leafValue;
    struct{ // inner nodes usually need some more values besides their type
      InnerNodeType type;
      int arg1; // This represents arity. It is used for AND, OR and Threshold nodes.
      int arg2; // This represents threshold. It is used only for threshold nodes.
    } m_innerNode;
  };

  NodeContent(int leafValue);
  NodeContent(InnerNodeType nodeType, int arg1);
  NodeContent(InnerNodeType nodeType, int arg1, int arg2);

 public:
  static NodeContent makeOrNode(int arity);
  static NodeContent makeAndNode(int arity);
  static NodeContent makeThreshNode(int arity, int threshold);

  string to_string();
};

//===========================================

class TreeNode {
  NodeContent m_node;
  vector<TreeNode> m_children;

 public:
  TreeNode(NodeContent node);
  vector<TreeNode> getChildren();
  void appendChild(NodeContent node);
  std::string to_string();
};
