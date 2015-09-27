#include <experimental/executor>
#include <experimental/loop_scheduler>
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

  std::experimental::loop_scheduler scheduler;
  std::experimental::executor_work<std::experimental::loop_scheduler::executor_type> w(scheduler.get_executor());
  std::thread t([&](){ scheduler.run(); });

  std::experimental::defer(std::experimental::bind_executor(scheduler, handler1));
  std::experimental::defer(std::experimental::bind_executor(scheduler, &handler1));
  std::experimental::defer(std::experimental::bind_executor(scheduler, handler2()));
  std::experimental::defer(std::experimental::bind_executor(scheduler, h2));
  std::experimental::defer(std::experimental::bind_executor(scheduler, ch2));
  std::experimental::defer(std::experimental::bind_executor(scheduler, handler3()));
  std::experimental::defer(std::experimental::bind_executor(scheduler, std::move(h3)));
  std::future<void> fut1 = std::experimental::defer(std::experimental::bind_executor(scheduler, std::experimental::use_future));
  fut1.get();

  w.reset();
  t.join();

  assert(handler_count == 7);
}
