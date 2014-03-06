#include <experimental/loop_scheduler>
#include <experimental/executor>
#include <cassert>

int main()
{
  std::experimental::loop_scheduler scheduler;
  auto ex = scheduler.get_executor();

  int count = 0;

  ex.post(
    [&]()
    {
      int count_before_dispatch = count;
      ex.dispatch([&](){ ++count; });
      assert(count == count_before_dispatch + 1);
    });

  int count_before_dispatch = count;
  ex.dispatch([&](){ ++count; });
  assert(count == count_before_dispatch);

  scheduler.run();
  assert(count == 2);
}
