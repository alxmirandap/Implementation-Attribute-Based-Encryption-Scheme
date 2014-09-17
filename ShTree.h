/*
  Testbed for empirical evaluation of KP-ABE schemes, according to Crampton, Pinto (CSF2014).
  Code by: Alexandre Miranda Pinto

  This file implements a specific Secret Sharing scheme: a tree of Shamir threshold schemes.
  There are two classes implemented here: 
  - ShTreeAccessPolicy is a subclass of the abstract AccessPolicy
  - ShTreeSS is a subclass of the abstract SecretSharing
*/

#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#ifndef DEF_TREE
#include "tree.h"
#endif

#define DEF_SH_TREE


class ShTreeAccessPolicy : public AccessPolicy
{
  std::string m_description; // to facilitate parsing, the description should be input in prefix, that is functional, notation.
  shared_ptr<TreeNode> m_treePolicy;
  void init();

 protected:
  void obtainCoveredFragsRec(int &count, shared_ptr<TreeNode> tree, const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const;
 public:
  static bool satisfyNodeID(shared_ptr<TreeNode> treeNode, vector<std::string> shareIDs, vector<int> &satisfyingSharesIndices);
  static bool satisfyNode(shared_ptr<TreeNode> node, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares);
  shared_ptr<TreeNode> parsePolicy(); // takes the policy description and returns an equivalent parse tree
  shared_ptr<TreeNode> parseTreeFromExpression(std::string expr);
  shared_ptr<TreeNode>& getPolicy();
  ShTreeAccessPolicy();
  ShTreeAccessPolicy(const string &description, const int n); // constructor with participants numbered from 1 to n, each participant holding one share
  ShTreeAccessPolicy(const string &description, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  ShTreeAccessPolicy(const ShTreeAccessPolicy& other);
  ShTreeAccessPolicy& operator=(const ShTreeAccessPolicy& other);
  std::string getDescription() const;
  unsigned int getNumShares();
  //  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
  bool evaluateIDs(const vector<std::string> shareIDs, vector<int> &witnessSharesIndices) const;
  vector<Big> findCoefficients(const vector<std::string> shareIDs, const Big& order) const;
  void obtainCoveredFrags(const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const;

  static void storeSharePrefixes(std::map<std::string, vector<int> >& setChildNos, std::string& shareID);
  static bool extractPrefixAndNoFromID(std::string& shareID, std::string& prefix, int& childNo);
  static Big findFinalCoefficient(const std::string& shareID,  std::map<std::string,  vector<int> >& setChildNos, 
			   std::map<std::string, vector<Big> >& setCoeffs, const Big& order);
  static Big computeLagrangeCoefficient(unsigned int shareIndex, vector<ShareTuple>& witnessShares, const Big& order);
  static Big computeLagrangeCoefficientChildNos(unsigned int shareIndex, vector<int>& witnessChildNos, const Big& order);
  static int extractChildNoFromID(std::string& shareID);

  inline static int extractPublicInfoFromChildNo(int childNo) {
    return childNo + 1;
  }

  inline static int extractPublicInfoFromID(std::string ID) {
    int childNo = extractChildNoFromID(ID);
    return extractPublicInfoFromChildNo(childNo);
  }

};

class ShTreeSS : public SecretSharing
{
 private:
  shared_ptr<ShTreeAccessPolicy>  i_policy; 
  vector<Big> m_randomness;
  
 protected:
  void init();
  void initPolicy();
  void manageRandomness(RandomnessActions action, shared_ptr<TreeNode> root, int &count);
  void manageRandomness(RandomnessActions action);

 public:
  ShTreeSS(shared_ptr<ShTreeAccessPolicy> policy, PFC &pfc);
  ShTreeSS(shared_ptr<ShTreeAccessPolicy> policy, const Big &order, PFC &pfc);  

  // virtual inherited methods:
  vector<Big> getDistribRandomness();  
  std::vector<ShareTuple> distribute_random(const Big& s);
  std::vector<ShareTuple> distribute_determ(const Big& s, const vector<Big>& randomness);
  std::vector<ShareTuple> distribute_determ(shared_ptr<TreeNode> root, const Big& s, const vector<Big>& randomness, int &count);
  Big reconstruct (const vector<ShareTuple> shares);
  Big reconstruct_old(const vector<ShareTuple> shares);

  // util-type functions
  static void updateSet(std::map<std::string, vector<ShareTuple> >& setShares, const std::string& prefix, ShareTuple& share);
  static void addNewSet(std::map<std::string, vector<ShareTuple> >& setShares, const std::string& prefix, ShareTuple& share);
  static void putShareInSet(std::map<std::string, vector<ShareTuple> >& setShares, const std::string& prefix, ShareTuple& share);
  static std::string getSetPrefix(std::string &shareID);

  static ShareTuple detailedReconstruction(vector<ShareTuple>& minimalShares, std::string& prefix, const Big& order);
  static ShareTuple solveSet(std::map<std::string, vector<ShareTuple> >& setShares, std::string& prefix, const Big& order);
  static ShareTuple reduceLowestShares(const vector<ShareTuple>& shares, Big order);
  static ShareTuple reduceMapShares(std::string& prefix, const std::string& target, 
				    std::map<std::string, vector<ShareTuple> >& setShares, const Big& order);


};
