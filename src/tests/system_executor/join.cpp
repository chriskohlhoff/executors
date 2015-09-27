#include <experimental/executor>
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

  std::experimental::system_executor ex;

  std::experimental::post(ex, handler1);
  std::experimental::post(ex, &handler1);
  std::experimental::post(ex, handler2());
  std::experimental::post(ex, h2);
  std::experimental::post(ex, ch2);
  std::experimental::post(ex, handler3());
  std::experimental::post(ex, std::move(h3));

  ex.context().join();

  assert(handler_count == 7);
}
