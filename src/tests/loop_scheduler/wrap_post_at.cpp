#include <experimental/executor>
#include <experimental/future>
#include <experimental/loop_scheduler>
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

  auto abs_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);

  std::experimental::loop_scheduler scheduler;
  std::experimental::executor_work<std::experimental::loop_scheduler::executor_type> w(scheduler.get_executor());
  std::thread t([&](){ scheduler.run(); });

  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, handler1));
  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, &handler1));
  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, handler2()));
  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, h2));
  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, ch2));
  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, handler3()));
  std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, std::move(h3)));
  std::future<void> fut1 = std::experimental::post_at(abs_time, std::experimental::wrap(scheduler, std::experimental::use_future));
  fut1.get();

  w.reset();
  t.join();

  assert(handler_count == 7);
}
