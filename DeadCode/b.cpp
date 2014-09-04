#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>

class Test {
private:
  int a;

  Test(int b) {
    a = b;
  }

public:
  Test() {
    a = 0;
  }
  static Test makeTest(int a) {
    return Test(a);
  }
  
  std::string toString(){
    std::stringstream ss;
    ss << "My value: " << a;
    return ss.str();
  }

  int getValue() {
    return a;
  }

};


class TestContainer {
private:
  Test t;

public:
  TestContainer(Test w) {
    t = Test::makeTest(w.getValue());
  }

  std::string toString(){
    return "Container: " + t.toString();
  }

  static std::shared_ptr<TestContainer> makeTestContainer(int x){
    Test t = Test::makeTest(x);
    std::shared_ptr<TestContainer> ptr = std::make_shared<TestContainer>(t);
    std::cout << "ptr value: " << ptr << std::endl;
    std::cout << "Inside " << ptr->toString() << std::endl;
    return ptr;
  } 
};



int main() {
  int x = 10;
  std::shared_ptr<TestContainer> ptr = TestContainer::makeTestContainer(x);
  std::cout << "ptr value: " << ptr << std::endl;

  std::cout << "outside 1" << ptr->toString() << std::endl;
  x = 20;
  std::cout << "outside 2" << ptr->toString() << std::endl;
}
