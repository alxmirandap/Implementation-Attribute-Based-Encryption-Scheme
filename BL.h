#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secret_sharing.h"
#endif

#include "tree.h"
#define BL


class BLAccessPolicy : public AccessPolicy{
  std::string m_description; // to facilitate parsing, the description should be input in infix, that is functional, notation.
  shared_ptr<TreeNode> m_treePolicy;
  void init();

 protected:
  shared_ptr<TreeNode> parsePolicy(); // takes the policy description and returns an equivalent parse tree
  shared_ptr<TreeNode> parseTreeFromExpression(std::string expr);

 public:
  static bool satisfyNode(shared_ptr<TreeNode> node, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares);
  BLAccessPolicy();
  BLAccessPolicy(const string &description, const int n); // constructor with participants numbered from 1 to n, each participant holding one share
  BLAccessPolicy(const string &description, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  BLAccessPolicy(const BLAccessPolicy& other);
  BLAccessPolicy& operator=(const BLAccessPolicy& other);
  std::string getDescription() const;
  unsigned int getNumShares();
  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
};
