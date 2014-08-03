#include <experimental/executor>
#include <experimental/future>
#include <experimental/loop_scheduler>
#include <experimental/strand>
#include <cassert>
#include <stdexcept>

int main()
{
  std::experimental::loop_scheduler scheduler;

  std::experimental::executor ex1 = scheduler.get_executor();
  std::experimental::executor ex2 = scheduler.get_executor();
  assert(ex1 == ex2);

  ex2 = ex1;
  assert(ex1 == ex2);

  ex1 = std::experimental::system_executor();
  assert(ex1 != ex2);

  ex2 = std::experimental::system_executor();
  assert(ex1 == ex2);

  std::experimental::strand<std::experimental::system_executor> s1;
  ex1 = s1;
  ex2 = s1;
  assert(ex1 == ex2);

  std::experimental::strand<std::experimental::system_executor> s2;
  ex1 = s2;
  assert(s1 != s2);
  assert(ex1 != ex2);

  std::experimental::strand<std::experimental::system_executor> s3(s1);
  ex1 = s3;
  assert(s1 == s3);
  assert(s2 != s3);
  assert(ex1 == ex2);
}
