#include <experimental/timer>
#include <experimental/future>
#include <atomic>

std::atomic<int> success_count;

void handler1(std::error_code ec)
{
  if (!ec) ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(std::error_code ec) { if (!ec) ++success_count; }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(std::error_code ec) && { if (!ec) ++success_count; }
};

int main()
{
  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  std::experimental::steady_timer t1(std::chrono::milliseconds(100));
  t1.wait(handler1);

  std::experimental::steady_timer t2(std::chrono::milliseconds(100));
  t2.wait(&handler1);

  std::experimental::steady_timer t3(std::chrono::milliseconds(100));
  t3.wait(handler2());

  std::experimental::steady_timer t4(std::chrono::milliseconds(100));
  t4.wait(h2);

  std::experimental::steady_timer t5(std::chrono::milliseconds(100));
  t5.wait(ch2);

  std::experimental::steady_timer t6(std::chrono::milliseconds(100));
  t6.wait(handler3());

  std::experimental::steady_timer t7(std::chrono::milliseconds(100));
  t7.wait(std::move(h3));

  std::experimental::steady_timer t8(std::chrono::milliseconds(100));
  std::future<void> fut1 = t8.wait(std::experimental::use_future);
  fut1.get();

  std::experimental::steady_timer t9(std::chrono::milliseconds(100));
  t9.wait(handler1);
  std::experimental::steady_timer t10(std::move(t9));

  std::experimental::steady_timer t11(std::chrono::milliseconds(100));
  t11.wait(&handler1);
  std::experimental::steady_timer t12(std::move(t10));

  std::experimental::steady_timer t13(std::chrono::milliseconds(100));
  t13.wait(handler2());
  std::experimental::steady_timer t14(std::move(t13));

  std::experimental::steady_timer t15(std::chrono::milliseconds(100));
  t15.wait(h2);
  std::experimental::steady_timer t16(std::move(t15));

  std::experimental::steady_timer t17(std::chrono::milliseconds(100));
  t17.wait(ch2);
  std::experimental::steady_timer t18(std::move(t17));

  std::experimental::steady_timer t19(std::chrono::milliseconds(100));
  t19.wait(handler3());
  std::experimental::steady_timer t20(std::move(t19));

  std::experimental::steady_timer t21(std::chrono::milliseconds(100));
  t21.wait(std::move(h3));
  std::experimental::steady_timer t22(std::move(t21));

  std::experimental::steady_timer t23(std::chrono::milliseconds(100));
  std::future<void> fut2 = t23.wait(std::experimental::use_future);
  std::experimental::steady_timer t24(std::move(t23));
  fut2.get();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  assert(success_count == 14);
}
