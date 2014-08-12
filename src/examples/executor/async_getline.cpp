#include <experimental/executor>
#include <experimental/thread_pool>
#include <iostream>

using std::experimental::dispatch;
using std::experimental::make_work;
using std::experimental::post;
using std::experimental::thread_pool;
using std::experimental::wrap;

template <class Handler>
void async_getline(std::istream& is, Handler handler)
{
  auto work = make_work(handler);

  post([&is, work, handler=std::move(handler)]
      {
        std::string line;
        std::getline(is, line);

        dispatch(work.get_executor(),
            [line, handler=std::move(handler)]
            {
              handler(line);
            });
      });
}

int main()
{
  thread_pool pool;

  std::cout << "Enter a line: ";

  async_getline(std::cin,
      wrap(pool, [](std::string line)
        {
          std::cout << "Line: " << line << "\n";
        }));

  pool.join();
}
