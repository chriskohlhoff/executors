#include <experimental/executor>
#include <experimental/future>
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

  std::experimental::loop_scheduler scheduler;
  std::experimental::executor ex = scheduler.get_executor();

  ex = std::experimental::system_executor();
  assert(&ex.context() == &std::experimental::system_executor().context());

  std::experimental::defer(ex, handler1);
  std::experimental::defer(ex, &handler1);
  std::experimental::defer(ex, handler2());
  std::experimental::defer(ex, h2);
  std::experimental::defer(ex, ch2);
  std::experimental::defer(ex, handler3());
  std::experimental::defer(ex, std::move(h3));
  std::future<void> fut1 = std::experimental::defer(ex, std::experimental::use_future);
  fut1.get();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  assert(handler_count == 7);

  std::future<int> fut2 = std::experimental::defer(ex, std::experimental::package(handler4));
  try
  {
    fut2.get();
    assert(0);
  }
  catch (std::exception& e)
  {
    assert(e.what() == std::string("oops"));
  }
}
