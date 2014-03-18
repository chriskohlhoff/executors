#include <experimental/timer>
#include <thread>

std::atomic<int> success_count;
std::atomic<int> failure_count;

void handler1(std::error_code ec)
{
  if (!ec) ++success_count; else ++failure_count;
}

int main()
{
  std::experimental::steady_timer t1(std::chrono::seconds(1));

  t1.wait(handler1);
  t1.wait(handler1);
  t1.wait(handler1);
  t1.wait(handler1);
  t1.wait(handler1);
  t1.wait(handler1);
  t1.wait(handler1);
  t1.wait(handler1);

  assert(success_count == 0);
  assert(failure_count == 0);

  t1.cancel_one();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  assert(success_count == 0);
  assert(failure_count == 1);

  t1.cancel_one();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  assert(success_count == 0);
  assert(failure_count == 2);

  t1.cancel();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  assert(success_count == 0);
  assert(failure_count == 8);
}
