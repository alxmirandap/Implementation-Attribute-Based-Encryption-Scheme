#include <iostream>
#include <memory>
#include <vector>

//vector<std::shared_ptr<int> vec;
 
std::vector<int> vec;
std::vector<std::shared_ptr<int>> vecptr;

//std::shared_ptr<int> 
std::shared_ptr<int> getElemPtr(int i)
{
  return vecptr[i];
}

int getElem(int i)
{
  return vec[i];
}
 
int main()
{
  int a = 10;
  int b = 15;
  int c = 20;

  std::shared_ptr<int> pa = std::make_shared<int>(a);
  std::shared_ptr<int> pb = std::make_shared<int>(b);
  std::shared_ptr<int> pc = std::make_shared<int>(c);

  vec.push_back(a);
  vec.push_back(b);
  vec.push_back(c);
  vec.push_back(a);

  vecptr.push_back(pa);
  vecptr.push_back(pb);
  vecptr.push_back(pc);
  vecptr.push_back(pa);

  int d = getElem(3);
  d -=5;

  std::shared_ptr<int> pd = getElemPtr(3);
  *pd = 33;

  for (int i = 0; i < vec.size(); i++) {
    std::cout << "Elem[" << i << "]: " << *vecptr[i] << std::endl;
  }

}
