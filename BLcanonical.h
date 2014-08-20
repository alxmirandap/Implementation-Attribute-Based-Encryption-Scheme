#ifndef DEF_UTILS
#include "utils.h"
#endif

#ifndef DEF_SECRET_SHARING
#include "secretsharing.h"
#endif

#define DEF_BL_CANON


class BLAccessPolicy : public AccessPolicy{
  std::string m_description; // to facilitate parsing, the description should be input in infix, that is functional, notation.
  vector<vector<int>> m_minimal_sets;
  void init();

 protected:
  bool satisfyMinimalSet(int setID, vector<int> set, vector<ShareTuple> shares, vector<ShareTuple> &satisfyingShares) const;

 public:
  static std::vector<std::vector<int>> parseFromExpression(int level, std::string expr);
  BLAccessPolicy();
  BLAccessPolicy(const string &description, const int n); // constructor with participants numbered from 1 to n, each participant holding one share
  BLAccessPolicy(const string &description, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  BLAccessPolicy(const BLAccessPolicy& other);
  BLAccessPolicy& operator=(const BLAccessPolicy& other);
  std::string getDescription() const;
  unsigned int getNumShares();
  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
};
