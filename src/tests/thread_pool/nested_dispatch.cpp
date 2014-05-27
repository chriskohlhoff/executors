#include <experimental/thread_pool>
#include <experimental/executor>
#include <cassert>

int main()
{
  std::experimental::thread_pool pool(1);
  auto ex = pool.get_executor();

  int count = 0;
  std::experimental::post(ex,
    [&]()
    {
      int count_before_dispatch = count;
      std::experimental::dispatch(ex, [&](){ ++count; });
      assert(count == count_before_dispatch + 1);
    });

  pool.join();
  assert(count == 1);
}
