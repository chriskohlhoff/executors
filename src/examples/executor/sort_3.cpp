#include <experimental/executor>
#include <experimental/future>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using std::experimental::copost;
using std::experimental::use_future;

template <class Iterator>
struct sorter
{
  Iterator begin_, end_;
  bool do_merge_ = false;

  // As we are doing recursive copost() calls, some type erasure is required.
  // Would really prefer to use std::unique_function here, but it doesn't exist.
  template <class T> static void destroy(void* p) { delete static_cast<T*>(p); }
  template <class T> static void call(void* p) { (*static_cast<T*>(p))(); }
  std::unique_ptr<void, void(*)(void*)> continuation_{nullptr, &sorter::destroy<int>};
  void (*call_continuation_)(void* p) = nullptr;

  sorter(Iterator b, Iterator e) : begin_(b), end_(e) {}

  template <class C> sorter(Iterator b, Iterator e, C c)
    : begin_(b), end_(e),
      continuation_(new C(std::move(c)), &sorter::destroy<C>),
      call_continuation_(&sorter::call<C>) {}

  void operator()();
};

namespace std { namespace experimental {

template <class Iterator>
struct continuation_traits<sorter<Iterator>>
{
  typedef void signature();

  template <class C> static sorter<Iterator> chain(sorter<Iterator> f, C c)
  {
    return sorter<Iterator>(f.begin_, f.end_, std::move(c));
  }
};

}} // namespace std::experimental

template <class Iterator>
void sorter<Iterator>::operator()()
{
  const std::size_t n = end_ - begin_;
  if (do_merge_)
  {
    std::inplace_merge(begin_, begin_ + (n / 2), end_);
    call_continuation_(continuation_.get());
  }
  else if (n <= 32768)
  {
    std::sort(begin_, end_);
    call_continuation_(continuation_.get());
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
    sorter<std::vector<double>::iterator> s(vec.begin(), vec.end());
    dispatch(std::move(s), use_future).get();
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
