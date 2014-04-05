#include <experimental/executor>
#include <experimental/future>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

using std::experimental::copost;
using std::experimental::use_future;

int main(int argc, char* argv[])
{
  const std::string parallel("parallel");
  const std::string serial("serial");

  if (!(argc == 3 && (argv[1] != parallel || argv[1] != serial)))
  {
    std::cerr << "Usage: `sort parallel <size>' or `sort serial <size>'" << std::endl;
    return 1;
  }

  std::vector<double> vec(std::atoll(argv[2]));
  std::iota(vec.begin(), vec.end(), 0);

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(vec.begin(), vec.end(), g);

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  if (argv[1] == parallel)
  {
    copost(
      [&]{ std::sort(vec.begin(), vec.begin() + (vec.size() / 2)); },
      [&]{ std::sort(vec.begin() + (vec.size() / 2), vec.end()); },
      use_future).get();

    std::inplace_merge(vec.begin(), vec.begin() + (vec.size() / 2), vec.end());
  }
  else
  {
    std::sort(vec.begin(), vec.end());
  }

  std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - start;

  std::cout << "sort took ";
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  std::cout << " microseconds" << std::endl;
}
