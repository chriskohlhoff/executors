#include <experimental/future>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>

using std::experimental::dispatch;
using std::experimental::dispatch_at;
using std::experimental::use_future;
using std::experimental::yield_context;

int main()
{
  dispatch(
    [](yield_context yield)
    {
      auto start_time = std::chrono::steady_clock::now();
      for (int i = 0; i < 10; ++i)
      {
        dispatch_at(start_time + std::chrono::seconds(i + 1), yield);
        std::cout << i << std::endl;
      }
    }, use_future).get();
}
