#include <experimental/thread_pool>
#include <experimental/executor>

int main()
{
  std::experimental::thread_pool pool;
  typedef std::experimental::thread_pool::executor_type executor;

  executor e1 = std::experimental::get_associated_executor(std::experimental::wrap(pool, []{}));
  executor e2 = std::experimental::get_associated_executor([]{}, pool.get_executor());
  executor e3 = std::experimental::get_associated_executor([]{}, pool);

  (void)e1;
  (void)e2;
  (void)e3;
}
