#include <cstdlib>
#include <experimental/executor>
#include <experimental/thread_pool>
#include <iostream>
#include <thread>

using std::experimental::post;
using std::experimental::thread_pool;

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
    std::cerr << "Usage: post_2 <count>\n";
    return 1;
  }

  std::vector<work> work_list(std::atoi(argv[1]));

  // Spawn a large amount of work and join on the pool to complete.
  thread_pool tp(16);
  for (auto work: work_list)
  {
    post(tp, [work]{
        do_something(work);
      });
  }
  tp.join();
}
