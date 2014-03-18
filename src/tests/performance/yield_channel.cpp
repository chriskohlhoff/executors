#include <experimental/channel>
#include <experimental/loop_scheduler>
#include <experimental/yield>
#include <chrono>
#include <iostream>

using namespace std::experimental;

const int iterations = 100000;

int main()
{
  loop_scheduler s(1);
  auto ex = make_executor(s);

  channel<int> ch1(s);
  channel<int> ch2(s);

  dispatch(
    ex.wrap(
      [&](basic_yield_context<loop_scheduler::executor> yield)
      {
        for (int i = 0; i < iterations; ++i)
        {
          ch1.put(i, yield);
          ch2.get(yield);
        }
      }));

  dispatch(
    ex.wrap(
      [&](basic_yield_context<loop_scheduler::executor> yield)
      {
        for (int i = 0; i < iterations; ++i)
        {
          ch1.get(yield);
          ch2.put(i, yield);
        }
      }));

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  s.run();
  std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - start;

  std::cout << "time per switch: ";
  std::chrono::steady_clock::duration per_iteration = elapsed / iterations;
  std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(per_iteration).count();
  std::cout << " nanoseconds\n";

  std::cout << "switches per second: ";
  std::cout << (std::chrono::seconds(1) / per_iteration) << "\n";
}
