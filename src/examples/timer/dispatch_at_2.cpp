#include <experimental/loop_scheduler>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>

using namespace std::experimental;

int main()
{
  loop_scheduler scheduler;
  auto executor = make_executor(scheduler);

  dispatch(
    executor.wrap(
      [](yield_context yield)
      {
        auto start_time = std::chrono::steady_clock::now();
        for (int i = 0; i < 10; ++i)
        {
          dispatch_at(start_time + std::chrono::seconds(i + 1), yield);
          std::cout << i << std::endl;
        }
      }
    ));

  scheduler.run();
}
