#include <experimental/await>
#include <experimental/executor>
#include <experimental/future>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

using std::experimental::await_context;
using std::experimental::codispatch;
using std::experimental::use_future;

template <typename Iterator>
void sort_coroutine(Iterator begin, Iterator end, await_context ctx)
{
  std::size_t n = end - begin;
  reenter (ctx)
  {
    if (n <= 32768)
    {
      std::sort(begin, end);
    }
    else
    {
      await copost(
        [=](await_context ctx) { sort_coroutine(begin, begin + n / 2, ctx); },
        [=](await_context ctx) { sort_coroutine(begin + n / 2, end, ctx); },
        ctx);
      std::inplace_merge(begin, begin + n / 2, end);
    }
  }
}

template <class Iterator, class CompletionToken>
auto parallel_sort(Iterator begin, Iterator end, CompletionToken&& token)
{
  return dispatch(
    [=](await_context ctx){ sort_coroutine(begin, end, ctx); },
    std::forward<CompletionToken>(token));
}

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
  for (std::size_t i = 0; i < vec.size(); ++i)
    vec[i] = i;

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(vec.begin(), vec.end(), g);

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  if (argv[1] == parallel)
  {
    parallel_sort(vec.begin(), vec.end(), use_future).get();
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
