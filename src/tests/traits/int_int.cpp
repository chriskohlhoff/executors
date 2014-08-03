#include <experimental/type_traits>
#include <experimental/future>
#include <cassert>

template <class CompletionToken>
auto async_foo(CompletionToken&& tok)
{
  std::experimental::async_completion<CompletionToken, void(int, int)> completion(tok);

  std::move(completion.handler)(1, 2);

  return completion.result.get();
}

int success_count = 0;

void handler1(int, int)
{
  ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(int, int) { ++success_count; }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(int, int) && { ++success_count; }
};

int main()
{
  async_foo(handler1);

  async_foo(&handler1);

  async_foo(handler2());

  handler2 h1;
  async_foo(h1);

  const handler2 h2;
  async_foo(h2);

  async_foo(handler3());

  handler3 h3;
  async_foo(std::move(h3));

  async_foo([](int, int){ ++success_count; });

  std::future<std::tuple<int, int>> f = async_foo(std::experimental::use_future);
  f.get();
  ++success_count;

  assert(success_count == 9);
}
