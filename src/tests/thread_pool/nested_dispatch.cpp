#include <experimental/thread_pool>
#include <experimental/executor>
#include <cassert>

int main()
{
  std::experimental::thread_pool pool(1);
  auto ex = make_executor(pool);

  int count = 0;
  ex.post(
    [&]()
    {
      int count_before_dispatch = count;
      ex.dispatch([&](){ ++count; });
      assert(count == count_before_dispatch + 1);
    });

  pool.join();
  assert(count == 1);
}
