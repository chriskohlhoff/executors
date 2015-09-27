#include <experimental/executor>
#include <experimental/thread_pool>
#include <cassert>
#include <string>

int handler_count = 0;

void handler1()
{
  ++handler_count;
}

struct handler2
{
  handler2() {}
  void operator()() { ++handler_count; }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()() { ++handler_count; }
};

int handler4()
{
  throw std::runtime_error("oops");
}

int main()
{
  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  std::experimental::thread_pool pool;

  std::experimental::post(pool, handler1);
  std::experimental::post(pool, &handler1);
  std::experimental::post(pool, handler2());
  std::experimental::post(pool, h2);
  std::experimental::post(pool, ch2);
  std::experimental::post(pool, handler3());
  std::experimental::post(pool, std::move(h3));
  std::future<void> fut1 = std::experimental::post(pool, std::experimental::use_future);
  fut1.get();

  pool.join();

  assert(handler_count == 7);
}
