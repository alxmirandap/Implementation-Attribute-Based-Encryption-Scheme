#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#ifndef DEF_TREE
#include "tree.h"
#endif

#define SH_TREE


class ShTreeAccessPolicy : public AccessPolicy{
  std::string m_description; // to facilitate parsing, the description should be input in prefix, that is functional, notation.
  shared_ptr<TreeNode> m_treePolicy;
  void init();

 protected:
  void obtainCoveredFragsRec(int &count, shared_ptr<TreeNode> tree, const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const;
 public:
  static bool satisfyNode(shared_ptr<TreeNode> node, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares);
  shared_ptr<TreeNode> parsePolicy(); // takes the policy description and returns an equivalent parse tree
  shared_ptr<TreeNode> parseTreeFromExpression(std::string expr);
  shared_ptr<TreeNode> getPolicy();
  ShTreeAccessPolicy();
  ShTreeAccessPolicy(const string &description, const int n); // constructor with participants numbered from 1 to n, each participant holding one share
  ShTreeAccessPolicy(const string &description, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  ShTreeAccessPolicy(const ShTreeAccessPolicy& other);
  ShTreeAccessPolicy& operator=(const ShTreeAccessPolicy& other);
  std::string getDescription() const;
  unsigned int getNumShares();
  //  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
  bool evaluateIDs(const vector<std::string> shareIDs, vector<int> &witnessSharesIndices) const;
  Big findCoefficient(const std::string id,const vector<std::string> shareIDs) const;
  void obtainCoveredFrags(const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const;
};
