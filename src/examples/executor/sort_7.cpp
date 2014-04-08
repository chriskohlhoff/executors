#include <experimental/continuation>
#include <experimental/executor>
#include <experimental/future>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using std::experimental::continuation;
using std::experimental::copost;
using std::experimental::use_future;

template <class Iterator>
struct sorter
{
  Iterator begin_, end_;
  bool do_merge_ = false;
  continuation<void()> continuation_;

  sorter(Iterator b, Iterator e) : begin_(b), end_(e) {}

  template <class C> sorter(Iterator b, Iterator e, C c)
    : begin_(b), end_(e), continuation_(std::move(c)) {}

  void operator()() &&;
};

namespace std { namespace experimental {

template <class Iterator>
struct continuation_of<sorter<Iterator>>
{
  typedef void signature();

  template <class C> static sorter<Iterator> chain(sorter<Iterator> f, C c)
  {
    return sorter<Iterator>(f.begin_, f.end_, std::move(c));
  }
};

}} // namespace std::experimental

template <class Iterator>
void sorter<Iterator>::operator()() &&
{
  const std::size_t n = end_ - begin_;
  if (do_merge_)
  {
    std::inplace_merge(begin_, begin_ + (n / 2), end_);
    dispatch(std::move(continuation_));
  }
  else if (n <= 32768)
  {
    std::sort(begin_, end_);
    dispatch(std::move(continuation_));
  }
  else
  {
    do_merge_ = true;
    copost(
      sorter<Iterator>{begin_, begin_ + (n / 2)},
      sorter<Iterator>{begin_ + (n / 2), end_},
      std::move(*this));
  }
}

template <class Iterator, class CompletionToken>
auto parallel_sort(Iterator begin, Iterator end, CompletionToken&& token)
{
  return dispatch(
    sorter<Iterator>(begin, end),
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
