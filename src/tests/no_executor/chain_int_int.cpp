#include <experimental/executor>
#include <experimental/future>
#include <cassert>
#include <stdexcept>
#include <string>

int function_count = 0;
int handler_count = 0;

int function1(int)
{
  return ++function_count;
}

struct function2
{
  function2() {}
  int operator()(int) { return ++function_count; }
};

struct function3
{
  function3() {}
  function3(const function3&) = delete;
  function3(function3&&) {}
  int operator()(int) { return ++function_count; }
};

int function_throw(int)
{
  throw std::runtime_error("oops");
}

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
  void operator()(int) && { ++handler_count; }
};

template <class F>
void invoke(F f)
{
  std::move(f)(42);
}

int main()
{
  function2 f2;
  const function2 cf2;
  function3 f3;

  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  invoke(std::experimental::chain<void(int)>(function1));
  invoke(std::experimental::chain<void(int)>(function1, handler1));
  invoke(std::experimental::chain<void(int)>(function1, &handler1));
  invoke(std::experimental::chain<void(int)>(function1, handler2()));
  invoke(std::experimental::chain<void(int)>(function1, h2));
  invoke(std::experimental::chain<void(int)>(function1, ch2));
  invoke(std::experimental::chain<void(int)>(function1, handler3()));
  invoke(std::experimental::chain<void(int)>(function1, std::move(h3)));

  invoke(std::experimental::chain<void(int)>(&function1));
  invoke(std::experimental::chain<void(int)>(&function1, handler1));
  invoke(std::experimental::chain<void(int)>(&function1, &handler1));
  invoke(std::experimental::chain<void(int)>(&function1, handler2()));
  invoke(std::experimental::chain<void(int)>(&function1, h2));
  invoke(std::experimental::chain<void(int)>(&function1, ch2));
  invoke(std::experimental::chain<void(int)>(&function1, handler3()));
  invoke(std::experimental::chain<void(int)>(&function1, std::move(h3)));

  invoke(std::experimental::chain<void(int)>(function2()));
  invoke(std::experimental::chain<void(int)>(function2(), handler1));
  invoke(std::experimental::chain<void(int)>(function2(), &handler1));
  invoke(std::experimental::chain<void(int)>(function2(), handler2()));
  invoke(std::experimental::chain<void(int)>(function2(), h2));
  invoke(std::experimental::chain<void(int)>(function2(), ch2));
  invoke(std::experimental::chain<void(int)>(function2(), handler3()));
  invoke(std::experimental::chain<void(int)>(function2(), std::move(h3)));

  invoke(std::experimental::chain<void(int)>(f2));
  invoke(std::experimental::chain<void(int)>(f2, handler1));
  invoke(std::experimental::chain<void(int)>(f2, &handler1));
  invoke(std::experimental::chain<void(int)>(f2, handler2()));
  invoke(std::experimental::chain<void(int)>(f2, h2));
  invoke(std::experimental::chain<void(int)>(f2, ch2));
  invoke(std::experimental::chain<void(int)>(f2, handler3()));
  invoke(std::experimental::chain<void(int)>(f2, std::move(h3)));

  invoke(std::experimental::chain<void(int)>(cf2));
  invoke(std::experimental::chain<void(int)>(cf2, handler1));
  invoke(std::experimental::chain<void(int)>(cf2, &handler1));
  invoke(std::experimental::chain<void(int)>(cf2, handler2()));
  invoke(std::experimental::chain<void(int)>(cf2, h2));
  invoke(std::experimental::chain<void(int)>(cf2, ch2));
  invoke(std::experimental::chain<void(int)>(cf2, handler3()));
  invoke(std::experimental::chain<void(int)>(cf2, std::move(h3)));

  invoke(std::experimental::chain<void(int)>(function3()));
  invoke(std::experimental::chain<void(int)>(function3(), handler1));
  invoke(std::experimental::chain<void(int)>(function3(), &handler1));
  invoke(std::experimental::chain<void(int)>(function3(), handler2()));
  invoke(std::experimental::chain<void(int)>(function3(), h2));
  invoke(std::experimental::chain<void(int)>(function3(), ch2));
  invoke(std::experimental::chain<void(int)>(function3(), handler3()));
  invoke(std::experimental::chain<void(int)>(function3(), std::move(h3)));

  invoke(std::experimental::chain<void(int)>(std::move(f3)));
  invoke(std::experimental::chain<void(int)>(std::move(f3), handler1));
  invoke(std::experimental::chain<void(int)>(std::move(f3), &handler1));
  invoke(std::experimental::chain<void(int)>(std::move(f3), handler2()));
  invoke(std::experimental::chain<void(int)>(std::move(f3), h2));
  invoke(std::experimental::chain<void(int)>(std::move(f3), ch2));
  invoke(std::experimental::chain<void(int)>(std::move(f3), handler3()));
  invoke(std::experimental::chain<void(int)>(std::move(f3), std::move(h3)));

  assert(function_count == 56);
  assert(handler_count == 49);
}
