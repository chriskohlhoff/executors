#include <experimental/strand>
#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>
#include <thread>
#include <vector>

int count = 0;

int main()
{
  std::experimental::loop_scheduler scheduler;
  auto ex = make_strand(scheduler.get_executor());

  for (int i = 0; i < 10; ++i)
  {
    std::experimental::post(ex, []
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ++count;
      });
  }

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  std::thread thread([&]{ scheduler.run(); });
  scheduler.run();
  thread.join();

  std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - start;

  assert(count == 10);
  assert(elapsed >= std::chrono::seconds(1));
}
