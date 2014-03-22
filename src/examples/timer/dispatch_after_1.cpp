#include <experimental/future>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>

using std::experimental::dispatch;
using std::experimental::dispatch_after;
using std::experimental::use_future;
using std::experimental::yield_context;

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
