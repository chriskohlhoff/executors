#include <experimental/executor>
#include <experimental/future>
#include <experimental/thread_pool>
#include <experimental/timer>
#include <cassert>
#include <stdexcept>
#include <string>

std::atomic<int> handler_count(0);

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
  void operator()() && { ++handler_count; }
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

  auto rel_time = std::chrono::milliseconds(100);

  std::experimental::thread_pool pool;

  std::experimental::dispatch_after(rel_time, pool, handler1);
  std::experimental::dispatch_after(rel_time, pool, &handler1);
  std::experimental::dispatch_after(rel_time, pool, handler2());
  std::experimental::dispatch_after(rel_time, pool, h2);
  std::experimental::dispatch_after(rel_time, pool, ch2);
  std::experimental::dispatch_after(rel_time, pool, handler3());
  std::experimental::dispatch_after(rel_time, pool, std::move(h3));
  std::future<void> fut1 = std::experimental::dispatch_after(rel_time, pool, std::experimental::use_future);
  fut1.get();

  pool.join();

  assert(handler_count == 7);
}
