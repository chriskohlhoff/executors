#include <experimental/executor>
#include <experimental/future>
#include <experimental/loop_scheduler>
#include <cassert>
#include <stdexcept>

std::atomic<int> function_count(0);
std::atomic<int> handler_count(0);

void function1()
{
  ++function_count;
}

struct function2
{
  function2() {}
  void operator()() { ++function_count; }
};

struct function3
{
  function3() {}
  function3(const function3&) = delete;
  function3(function3&&) {}
  void operator()() { ++function_count; }
};

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

int main()
{
  function2 f2;
  const function2 cf2;
  function3 f3;

  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  std::experimental::loop_scheduler scheduler;
  std::experimental::executor ex = get_executor(scheduler);

  ex = std::experimental::system_executor();
  assert(&ex.context() == &std::experimental::system_executor().context());

  std::experimental::post(function1, ex, handler1);
  std::experimental::post(function1, ex, &handler1);
  std::experimental::post(function1, ex, handler2());
  std::experimental::post(function1, ex, h2);
  std::experimental::post(function1, ex, ch2);
  std::experimental::post(function1, ex, handler3());
  std::experimental::post(function1, ex, std::move(h3));
  std::future<void> fut1 = std::experimental::post(function1, ex, std::experimental::use_future);
  fut1.get();

  std::experimental::post(&function1, ex, handler1);
  std::experimental::post(&function1, ex, &handler1);
  std::experimental::post(&function1, ex, handler2());
  std::experimental::post(&function1, ex, h2);
  std::experimental::post(&function1, ex, ch2);
  std::experimental::post(&function1, ex, handler3());
  std::experimental::post(&function1, ex, std::move(h3));
  std::future<void> fut2 = std::experimental::post(&function1, ex, std::experimental::use_future);
  fut2.get();

  std::experimental::post(function2(), ex, handler1);
  std::experimental::post(function2(), ex, &handler1);
  std::experimental::post(function2(), ex, handler2());
  std::experimental::post(function2(), ex, h2);
  std::experimental::post(function2(), ex, ch2);
  std::experimental::post(function2(), ex, handler3());
  std::experimental::post(function2(), ex, std::move(h3));
  std::future<void> fut3 = std::experimental::post(function2(), ex, std::experimental::use_future);
  fut3.get();

  std::experimental::post(f2, ex, handler1);
  std::experimental::post(f2, ex, &handler1);
  std::experimental::post(f2, ex, handler2());
  std::experimental::post(f2, ex, h2);
  std::experimental::post(f2, ex, ch2);
  std::experimental::post(f2, ex, handler3());
  std::experimental::post(f2, ex, std::move(h3));
  std::future<void> fut4 = std::experimental::post(f2, ex, std::experimental::use_future);
  fut4.get();

  std::experimental::post(cf2, ex, handler1);
  std::experimental::post(cf2, ex, &handler1);
  std::experimental::post(cf2, ex, handler2());
  std::experimental::post(cf2, ex, h2);
  std::experimental::post(cf2, ex, ch2);
  std::experimental::post(cf2, ex, handler3());
  std::experimental::post(cf2, ex, std::move(h3));
  std::future<void> fut5 = std::experimental::post(cf2, ex, std::experimental::use_future);
  fut5.get();

  std::experimental::post(function3(), ex, handler1);
  std::experimental::post(function3(), ex, &handler1);
  std::experimental::post(function3(), ex, handler2());
  std::experimental::post(function3(), ex, h2);
  std::experimental::post(function3(), ex, ch2);
  std::experimental::post(function3(), ex, handler3());
  std::experimental::post(function3(), ex, std::move(h3));
  std::future<void> fut6 = std::experimental::post(function3(), ex, std::experimental::use_future);
  fut6.get();

  std::experimental::post(std::move(f3), ex, handler1);
  std::experimental::post(std::move(f3), ex, &handler1);
  std::experimental::post(std::move(f3), ex, handler2());
  std::experimental::post(std::move(f3), ex, h2);
  std::experimental::post(std::move(f3), ex, ch2);
  std::experimental::post(std::move(f3), ex, handler3());
  std::experimental::post(std::move(f3), ex, std::move(h3));
  std::future<void> fut7 = std::experimental::post(std::move(f3), ex, std::experimental::use_future);
  fut7.get();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  assert(function_count == 56);
  assert(handler_count == 49);
}
