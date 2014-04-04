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

struct sorter_continuation_base
{
  virtual ~sorter_continuation_base() {}
  virtual void operator()() = 0;
};

template <class Continuation>
struct sorter_continuation : sorter_continuation_base
{
  Continuation continuation_;
  explicit sorter_continuation(Continuation c) : continuation_(std::move(c)) {}
  virtual void operator()() { continuation_(); }
};

template <class Iterator>
struct sorter
{
  Iterator begin_, end_;
  std::unique_ptr<sorter_continuation_base> continuation_;
  bool do_merge_ = false;

  sorter(Iterator b, Iterator e) : begin_(b), end_(e) {}
  void operator()();
};

namespace std { namespace experimental {

template <class Iterator>
struct continuation_traits<sorter<Iterator>>
{
  typedef void signature();

  template <class F, class C> static sorter<Iterator> chain(F&& f, C c)
  {
    sorter<Iterator> s(f.begin_, f.end_);
    s.continuation_.reset(new sorter_continuation<C>(std::move(c)));
    return s;
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
    (*continuation_)();
  }
  else if (n <= 32768)
  {
    std::sort(begin_, end_);
    (*continuation_)();
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
