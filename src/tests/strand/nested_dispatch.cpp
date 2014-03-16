#include <experimental/strand>
#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>

int main()
{
  std::experimental::loop_scheduler scheduler;
  auto ex = make_strand(get_executor(scheduler));

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
