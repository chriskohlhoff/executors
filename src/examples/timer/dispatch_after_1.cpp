#include <experimental/future>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>

using namespace std::experimental;

int main()
{
  dispatch(
    [](yield_context yield)
    {
      for (int i = 0; i < 10; ++i)
      {
        dispatch_after(std::chrono::seconds(1), yield);
        std::cout << i << std::endl;
      }
    }, use_future).get();
}
