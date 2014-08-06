#include "utils.h"
#include "tree.h"
#define BL

#ifndef DEF_SECRET_SHARING
#include "secret_sharing.h"
#endif


class BLAccessPolicy : public AccessPolicy{
  string m_description;

  

 protected:
  

 public:


  ShamirAccessPolicy();
  ShamirAccessPolicy(const string &description, const int n); // constructor with participants numbered from 1 to n, each participant holding one share
  ShamirAccessPolicy(const string &description, const vector<int> &parts); // constructor with participants specified freely, each participant holding one share
  ShamirAccessPolicy(const ShamirAccessPolicy& other);
  ShamirAccessPolicy& operator=(const ShamirAccessPolicy& other);
  unsigned int getDescription() const;
  unsigned int getNumShares() const;
  bool evaluate(const vector<ShareTuple> shares, vector<ShareTuple> &witnessShares) const;
};
