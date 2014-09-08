#include <cassert>
#include <condition_variable>
#include <cstdlib>
#include <experimental/executor>
#include <iostream>
#include <mutex>
#include <thread>

using std::experimental::post;

class latch
{
public:
  explicit latch(std::size_t initial_count)
    : count_(initial_count)
  {
  }

  void arrive()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    assert(count_ > 0);
    if (--count_ == 0)
      condition_.notify_all();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]{ return count_ == 0; });
  }

private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::size_t count_;
};

struct work
{
};

void do_something(const work&)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: post_1 <count>\n";
    return 1;
  }

  std::vector<work> work_list(std::atoi(argv[1]));

  // Start a bunch of asynchronous tasks and wait for them to complete.
  latch l(work_list.size());
  for (auto work: work_list)
  {
    post(
      [work, &l]
      {
        do_something(work);
        l.arrive();
      });
  }
  l.wait();
}
