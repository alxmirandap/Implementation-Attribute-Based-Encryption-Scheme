#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#define DEF_BL_CANON


class BLAccessPolicy : public AccessPolicy{
  std::string m_description; // to facilitate parsing, the description should be input in prefix, that is functional, notation.
  vector<vector<int>> m_minimal_sets;
  void init();

 protected:
  bool satisfyMinimalSet(int setID, vector<int> set, vector<std::string> shareIDs, vector<int> &satisfyingSharesIndices) const;
 public:
  static std::vector<std::vector<int>> parseFromExpression(int level, std::string expr);
  vector<vector<int> >& getMinimalSets();
  BLAccessPolicy();
  BLAccessPolicy(const string &description, const int n); // constructor with participants numbered from 1 to n, each participant holding one share
  BLAccessPolicy(const string &description, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  BLAccessPolicy(const BLAccessPolicy& other);
  BLAccessPolicy& operator=(const BLAccessPolicy& other);
  std::string getDescription() const;
  unsigned int getNumShares();
  //  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
  static std::string createShareID(std::string setID, std::string partID);
  bool evaluateIDs(const vector<std::string> shareIDs, vector<int> &witnessSharesIndices) const;
  Big findCoefficient(const std::string id,const vector<std::string> shareIDs) const;
  void obtainCoveredFrags(const vector<int> &atts, vector<int> &attFragIndices, vector<int> &keyFragIndices, vector<std::string> &coveredShareIDs) const;
};

class BLSS : public SecretSharing
{
 private:
  shared_ptr<BLAccessPolicy>  i_policy; 
  vector<Big> m_randomness;
  
 protected:
  void init();
  void initPolicy();
  void manageRandomness(RandomnessActions action);

 public:
  BLSS(shared_ptr<BLAccessPolicy> policy, PFC &pfc);
  BLSS(shared_ptr<BLAccessPolicy> policy, const Big &order, PFC &pfc);  

  // virtual inherited methods:
  vector<Big> getDistribRandomness();  
  std::vector<ShareTuple> distribute_random(const Big& s);
  std::vector<ShareTuple> distribute_determ(const Big& s, const vector<Big>& randomness);
  Big reconstruct (const vector<ShareTuple> shares);
};
