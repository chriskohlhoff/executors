#include <experimental/type_traits>
#include <experimental/future>
#include <cassert>
#include <system_error>

template <class CompletionToken>
auto async_foo(bool fail, CompletionToken&& tok)
{
  std::experimental::async_completion<CompletionToken, void(std::error_code, int)> completion(tok);

  if (fail)
    std::move(completion.handler)(make_error_code(std::errc::invalid_argument), 1);
  else
    std::move(completion.handler)(std::error_code(), 1);

  return completion.result.get();
}

int success_count = 0;
int fail_count = 0;

void handler1(const std::error_code& e, int)
{
  e ? ++fail_count : ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(const std::error_code& e, int)
  {
    e ? ++fail_count : ++success_count;
  }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(const std::error_code& e, int) &&
  {
    e ? ++fail_count : ++success_count;
  }
};

int main()
{
  async_foo(false, handler1);
  async_foo(true, handler1);

  async_foo(false, &handler1);
  async_foo(true, &handler1);

  async_foo(false, handler2());
  async_foo(true, handler2());

  handler2 h1;
  async_foo(false, h1);
  async_foo(true, h1);

  const handler2 h2;
  async_foo(false, h2);
  async_foo(true, h2);

  async_foo(false, handler3());
  async_foo(true, handler3());

  handler3 h3, h4;
  async_foo(false, std::move(h3));
  async_foo(true, std::move(h4));

  async_foo(false, [](std::error_code e, int){ e ? ++fail_count : ++success_count; });
  async_foo(true, [](std::error_code e, int){ e ? ++fail_count : ++success_count; });

  std::future<int> f1 = async_foo(false, std::experimental::use_future);
  try
  {
    int v = f1.get();
    assert(v == 1);
    ++success_count;
  }
  catch (...)
  {
    ++fail_count;
  }

  std::future<int> f2 = async_foo(true, std::experimental::use_future);
  try
  {
    f2.get();
    ++success_count;
  }
  catch (...)
  {
    ++fail_count;
  }

  assert(success_count == 9);
  assert(fail_count == 9);
}
