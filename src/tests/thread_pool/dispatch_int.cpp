#include <experimental/executor>
#include <experimental/future>
#include <experimental/thread_pool>
#include <atomic>
#include <cassert>
#include <memory>

std::atomic<int> function_count(0);
std::atomic<int> handler_count(0);

int function1()
{
  return ++function_count;
}

struct function2
{
  function2() {}
  int operator()() { return ++function_count; }
};

struct function3
{
  function3() {}
  function3(const function3&) = delete;
  function3(function3&&) {}
  int operator()() { return ++function_count; }
};

void handler1(int)
{
  ++handler_count;
}

struct handler2
{
  handler2() {}
  void operator()(int) { ++handler_count; }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(int) { ++handler_count; }
};

int main()
{
  function2 f2;
  const function2 cf2;
  function3 f3;

  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  std::unique_ptr<std::experimental::thread_pool> scheduler(new std::experimental::thread_pool);
  auto ex = scheduler->get_executor();

  std::experimental::dispatch(function1, ex, handler1);
  std::experimental::dispatch(function1, ex, &handler1);
  std::experimental::dispatch(function1, ex, handler2());
  std::experimental::dispatch(function1, ex, h2);
  std::experimental::dispatch(function1, ex, ch2);
  std::experimental::dispatch(function1, ex, handler3());
  std::experimental::dispatch(function1, ex, std::move(h3));
  std::future<int> fut1 = std::experimental::dispatch(function1, ex, std::experimental::use_future);
  fut1.get();

  std::experimental::dispatch(&function1, ex, handler1);
  std::experimental::dispatch(&function1, ex, &handler1);
  std::experimental::dispatch(&function1, ex, handler2());
  std::experimental::dispatch(&function1, ex, h2);
  std::experimental::dispatch(&function1, ex, ch2);
  std::experimental::dispatch(&function1, ex, handler3());
  std::experimental::dispatch(&function1, ex, std::move(h3));
  std::future<int> fut2 = std::experimental::dispatch(&function1, ex, std::experimental::use_future);
  fut2.get();

  std::experimental::dispatch(function2(), ex, handler1);
  std::experimental::dispatch(function2(), ex, &handler1);
  std::experimental::dispatch(function2(), ex, handler2());
  std::experimental::dispatch(function2(), ex, h2);
  std::experimental::dispatch(function2(), ex, ch2);
  std::experimental::dispatch(function2(), ex, handler3());
  std::experimental::dispatch(function2(), ex, std::move(h3));
  std::future<int> fut3 = std::experimental::dispatch(function2(), ex, std::experimental::use_future);
  fut3.get();

  std::experimental::dispatch(f2, ex, handler1);
  std::experimental::dispatch(f2, ex, &handler1);
  std::experimental::dispatch(f2, ex, handler2());
  std::experimental::dispatch(f2, ex, h2);
  std::experimental::dispatch(f2, ex, ch2);
  std::experimental::dispatch(f2, ex, handler3());
  std::experimental::dispatch(f2, ex, std::move(h3));
  std::future<int> fut4 = std::experimental::dispatch(f2, ex, std::experimental::use_future);
  fut4.get();

  std::experimental::dispatch(cf2, ex, handler1);
  std::experimental::dispatch(cf2, ex, &handler1);
  std::experimental::dispatch(cf2, ex, handler2());
  std::experimental::dispatch(cf2, ex, h2);
  std::experimental::dispatch(cf2, ex, ch2);
  std::experimental::dispatch(cf2, ex, handler3());
  std::experimental::dispatch(cf2, ex, std::move(h3));
  std::future<int> fut5 = std::experimental::dispatch(cf2, ex, std::experimental::use_future);
  fut5.get();

  std::experimental::dispatch(function3(), ex, handler1);
  std::experimental::dispatch(function3(), ex, &handler1);
  std::experimental::dispatch(function3(), ex, handler2());
  std::experimental::dispatch(function3(), ex, h2);
  std::experimental::dispatch(function3(), ex, ch2);
  std::experimental::dispatch(function3(), ex, handler3());
  std::experimental::dispatch(function3(), ex, std::move(h3));
  std::future<int> fut6 = std::experimental::dispatch(function3(), ex, std::experimental::use_future);
  fut6.get();

  std::experimental::dispatch(std::move(f3), ex, handler1);
  std::experimental::dispatch(std::move(f3), ex, &handler1);
  std::experimental::dispatch(std::move(f3), ex, handler2());
  std::experimental::dispatch(std::move(f3), ex, h2);
  std::experimental::dispatch(std::move(f3), ex, ch2);
  std::experimental::dispatch(std::move(f3), ex, handler3());
  std::experimental::dispatch(std::move(f3), ex, std::move(h3));
  std::future<int> fut7 = std::experimental::dispatch(std::move(f3), ex, std::experimental::use_future);
  fut7.get();

  scheduler.reset();

  assert(function_count == 56);
  assert(handler_count == 49);
}
