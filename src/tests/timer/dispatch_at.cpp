#include <experimental/timer>
#include <experimental/loop_scheduler>
#include <experimental/future>

std::atomic<int> handler_count;

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
  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  std::experimental::loop_scheduler scheduler;
  auto ex = get_executor(scheduler);
  std::experimental::executor::work w = ex.make_work();
  std::thread t([&](){ scheduler.run(); });

  auto time = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
  std::experimental::dispatch_at(time, ex.wrap(handler1));
  std::experimental::dispatch_at(time, ex.wrap(&handler1));
  std::experimental::dispatch_at(time, ex.wrap(handler2()));
  std::experimental::dispatch_at(time, ex.wrap(h2));
  std::experimental::dispatch_at(time, ex.wrap(ch2));
  std::experimental::dispatch_at(time, ex.wrap(handler3()));
  std::experimental::dispatch_at(time, ex.wrap(std::move(h3)));
  std::future<void> fut1 = std::experimental::dispatch_at(time, ex.wrap(std::experimental::use_future));
  fut1.get();

  w = nullptr;
  assert(!w);
  assert(w == nullptr);
  assert(nullptr == w);
  assert(!(w != nullptr));
  assert(!(nullptr != w));

  t.join();

  assert(handler_count == 7);
}
