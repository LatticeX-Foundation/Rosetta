#include "cc/modules/common/tests/test.h"
#include "cc/modules/common/include/utils/check_type.h"

#include <iostream>
template <typename T>
auto func(T &&) -> T;

class Foo {};
class Bar {
 public:
  Bar() {
    std::cout << check_type<Bar>() << std::endl << std::endl;
  }
};

TEST_CASE("utils check type", "[common][utils]") {
  std::cout << check_type<const volatile void*>() << std::endl;
  std::cout << check_type<const volatile void*(&)[10]>() << std::endl;
  std::cout << check_type<int[1][2][3]>() << std::endl;
  std::cout << check_type<const int[3][10]>() << std::endl << std::endl;

  std::cout << check_type<int(unsigned)>() << std::endl;
  std::cout << check_type<int (*)(const int*(*)[3][10], Foo&&, int, unsigned)>() << std::endl;
  std::cout << check_type<char(*(*const)(const int(&)[10]))[10]>() << std::endl << std::endl;

  std::cout << check_type<Foo>() << std::endl << std::endl;
  std::cout << check_type<int Foo::*const(&)[]>() << std::endl << std::endl;

  std::cout << check_type<void (Foo::*)(void)>() << std::endl;
  std::cout << check_type<void (Foo::*)(void) volatile const>() << std::endl;
  std::cout << check_type<void (Foo::*(*)[])(int) const>() << std::endl;
  std::cout << check_type<int (Foo::*const)(int, Foo&&, int) volatile>() << std::endl << std::endl;

  std::cout << check_type<decltype(func<Foo>)>() << std::endl;
  std::cout << check_type<decltype(func<Foo&>)>() << std::endl;
  std::cout << check_type<decltype(func<Foo&&>)>() << std::endl;
}
