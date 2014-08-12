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

void putElemPtr(int i) {
  std::shared_ptr<int> pi = std::make_shared<int>(i);
  vecptr.push_back(pi);
}
 
int main()
{
  int a = 10;
  int b = 15;
  int c = 20;

  vec.push_back(a);
  vec.push_back(b);
  vec.push_back(c);
  vec.push_back(a);


  putElemPtr(a);
  putElemPtr(b);
  putElemPtr(c);
  putElemPtr(a);
  
  int d = getElem(3);
  d -=5;

  std::shared_ptr<int> pd = getElemPtr(3);
  *pd = 33;

  int e = *pd;

  std::cout << "e: " << e << std::endl;
  std::cout << "*pd: " << *pd << std::endl;

  *pd = 27;

  std::cout << "e: " << e << std::endl;
  std::cout << "*pd: " << *pd << std::endl;


  int x = 12;
  int* y = &x;

  *y = 21;

  int z = *y;

  std::cout << "x: " << x << std::endl;
  std::cout << "y: " << *y << std::endl;
  std::cout << "z: " << z << std::endl;

  *y = 32;

  std::cout << "x: " << x << std::endl;
  std::cout << "y: " << *y << std::endl;
  std::cout << "z: " << z << std::endl;


  // for (int i = 0; i < vec.size(); i++) {
  //   std::cout << "Elem[" << i << "]: " << vec[i] << std::endl;
  // }

  // for (int i = 0; i < vecptr.size(); i++) {
  //   std::cout << "ElemPtr[" << i << "]: " << *vecptr[i] << std::endl;
  // }
}
