#include <experimental/thread_pool>
#include <experimental/executor>

int main()
{
  std::experimental::thread_pool pool;
  typedef std::experimental::executor_work<std::experimental::thread_pool::executor_type> work;

  work w1 = std::experimental::make_work(pool.get_executor());
  work w2 = std::experimental::make_work(pool);
  work w3 = std::experimental::make_work(std::experimental::wrap(pool, []{}));
  work w4 = std::experimental::make_work([]{}, pool.get_executor());
  work w5 = std::experimental::make_work([]{}, pool);

  w1.reset();
  w2.reset();
  w3.reset();
  w4.reset();
  w5.reset();

  pool.join();
}
