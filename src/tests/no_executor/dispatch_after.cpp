#include <experimental/executor>
#include <experimental/future>
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

  std::experimental::dispatch_after(rel_time, handler1);
  std::experimental::dispatch_after(rel_time, &handler1);
  std::experimental::dispatch_after(rel_time, handler2());
  std::experimental::dispatch_after(rel_time, h2);
  std::experimental::dispatch_after(rel_time, ch2);
  std::experimental::dispatch_after(rel_time, handler3());
  std::experimental::dispatch_after(rel_time, std::move(h3));
  std::future<void> fut1 = std::experimental::dispatch_after(rel_time, std::experimental::use_future);
  fut1.get();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  assert(handler_count == 7);

  std::future<int> fut2 = std::experimental::dispatch_after(rel_time, std::experimental::package(handler4));
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
