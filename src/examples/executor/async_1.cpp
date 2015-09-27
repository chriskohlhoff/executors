#include <experimental/executor>
#include <functional>
#include <iostream>
#include <string>

using std::experimental::post;
using std::experimental::use_future;

// Emulate std::async() when used with launch::async.
template <class F, class... Args>
auto async(F&& f, Args&&... args)
{
  return post(
    use_future(
      std::bind(std::forward<F>(f),
        std::forward<Args>(args)...)));
}

int main()
{
  std::future<std::string> fut = async(
      []{
        std::cout << "Enter a line: ";
        std::string s;
        std::getline(std::cin, s);
        return s;
      });

  // Do something else here...

  std::cout << fut.get() << std::endl;
}
