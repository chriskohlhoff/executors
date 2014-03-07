#include <experimental/thread_pool>
#include <experimental/executor>
#include <cassert>

int main()
{
  std::unique_ptr<std::experimental::thread_pool> scheduler(new std::experimental::thread_pool(1));
  auto ex = get_executor(*scheduler);

  int count = 0;
  ex.post(
    [&]()
    {
      int count_before_dispatch = count;
      ex.dispatch([&](){ ++count; });
      assert(count == count_before_dispatch + 1);
    });

  scheduler.reset();
  assert(count == 1);
}
