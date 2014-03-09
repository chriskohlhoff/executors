#include <experimental/timer>

int main()
{
  {
    std::experimental::steady_timer t1;
    t1.expires_after(std::chrono::milliseconds(100));
    t1.wait();
  }

  {
    std::experimental::steady_timer t1;
    t1.expires_at(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
    t1.wait();
  }

  {
    std::experimental::steady_timer t1(std::chrono::milliseconds(100));
    t1.wait();
  }

  {
    std::experimental::steady_timer t1(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
    t1.wait();
  }

  {
    std::experimental::steady_timer t1;
    t1.expires_after(std::chrono::milliseconds(100));
    std::experimental::steady_timer t2(std::move(t1));
    t2.wait();
  }

  {
    std::experimental::steady_timer t1;
    t1.expires_at(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
    std::experimental::steady_timer t2(std::move(t1));
    t2.wait();
  }

  {
    std::experimental::steady_timer t1(std::chrono::milliseconds(100));
    std::experimental::steady_timer t2(std::move(t1));
    t2.wait();
  }

  {
    std::experimental::steady_timer t1(std::chrono::steady_clock::now() + std::chrono::milliseconds(100));
    std::experimental::steady_timer t2(std::move(t1));
    t2.wait();
  }
}
