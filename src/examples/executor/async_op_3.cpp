#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <experimental/thread_pool>
#include <iostream>
#include <string>

using std::experimental::async_completion;
using std::experimental::bind_executor;
using std::experimental::dispatch;
using std::experimental::loop_scheduler;
using std::experimental::make_work_guard;
using std::experimental::post;
using std::experimental::thread_pool;
using std::experimental::use_future;

// A function to asynchronously read a single line from an input stream.
template <class CompletionToken>
auto async_getline(std::istream& is, CompletionToken&& token)
{
  async_completion<CompletionToken, void(std::string)> completion(token);

  // Create executor_work for the handler's associated executor.
  auto work = make_work_guard(completion.completion_handler);

  // Post a function object to do the work asynchronously.
  post([&is, work, handler=std::move(completion.completion_handler)]() mutable
      {
        std::string line;
        std::getline(is, line);

        // Pass the result to the handler, via the associated executor.
        dispatch(work.get_executor(),
            [line=std::move(line), handler=std::move(handler)]() mutable
            {
              handler(std::move(line));
            });
      });

  return completion.result.get();
}

class line_printer
{
public:
  typedef loop_scheduler::executor_type executor_type;

  explicit line_printer(loop_scheduler& s)
    : executor_(s.get_executor())
  {
  }

  executor_type get_executor() const noexcept
  {
    return executor_;
  }

  void operator()(std::string line)
  {
    std::cout << "Line: " << line << "\n";
  }

private:
  loop_scheduler::executor_type executor_;
};

int main()
{
  thread_pool pool;

  std::cout << "Enter a line: ";

  async_getline(std::cin,
      bind_executor(pool, [](std::string line)
        {
          std::cout << "Line: " << line << "\n";
        }));

  pool.join();

  loop_scheduler scheduler;

  std::cout << "Enter another line: ";

  async_getline(std::cin, line_printer(scheduler));

  scheduler.run();

  std::cout << "Enter yet another line: ";

  std::future<std::string> line = async_getline(std::cin, use_future);
  std::cout << line.get() << "\n";
}
