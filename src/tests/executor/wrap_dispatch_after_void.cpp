#include <experimental/executor>
#include <experimental/future>
#include <experimental/loop_scheduler>
#include <experimental/timer>
#include <cassert>

int function_count = 0;
int handler_count = 0;

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

  auto rel_time = std::chrono::milliseconds(100);

  std::experimental::dispatch_after(rel_time, function1, ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, function1, ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, function1, ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, function1, ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, function1, ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, function1, ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, function1, ex.wrap(std::move(h3)));
  std::future<void> fut1 = std::experimental::dispatch_after(rel_time, function1, ex.wrap(std::experimental::use_future));
  fut1.get();

  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, &function1, ex.wrap(std::move(h3)));
  std::future<void> fut2 = std::experimental::dispatch_after(rel_time, &function1, ex.wrap(std::experimental::use_future));
  fut2.get();

  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, function2(), ex.wrap(std::move(h3)));
  std::future<void> fut3 = std::experimental::dispatch_after(rel_time, function2(), ex.wrap(std::experimental::use_future));
  fut3.get();

  std::experimental::dispatch_after(rel_time, f2, ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, f2, ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, f2, ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, f2, ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, f2, ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, f2, ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, f2, ex.wrap(std::move(h3)));
  std::future<void> fut4 = std::experimental::dispatch_after(rel_time, f2, ex.wrap(std::experimental::use_future));
  fut4.get();

  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, cf2, ex.wrap(std::move(h3)));
  std::future<void> fut5 = std::experimental::dispatch_after(rel_time, cf2, ex.wrap(std::experimental::use_future));
  fut5.get();

  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, function3(), ex.wrap(std::move(h3)));
  std::future<void> fut6 = std::experimental::dispatch_after(rel_time, function3(), ex.wrap(std::experimental::use_future));
  fut6.get();

  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(handler1));
  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(&handler1));
  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(handler2()));
  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(h2));
  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(ch2));
  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(handler3()));
  std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(std::move(h3)));
  std::future<void> fut7 = std::experimental::dispatch_after(rel_time, std::move(f3), ex.wrap(std::experimental::use_future));
  fut7.get();

  assert(function_count == 56);
  assert(handler_count == 49);
}
