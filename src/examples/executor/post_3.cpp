#include <algorithm>
#include <cassert>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
#include <experimental/executor>
#include <experimental/strand>
#include <experimental/thread_pool>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using std::experimental::executor;
using std::experimental::executor_work;
using std::experimental::make_work;
using std::experimental::post;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::thread_pool;
using std::experimental::wrap;

class latch
{
public:
  explicit latch(std::size_t initial_count)
    : count_(initial_count)
  {
  }

  void arrive()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    assert(count_ > 0);
    if (--count_ == 0)
      condition_.notify_all();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this]{ return count_ == 0; });
  }

private:
  std::mutex mutex_;
  std::condition_variable condition_;
  std::size_t count_;
};

class notifying_latch
{
public:
  template <class Handler>
  explicit notifying_latch(std::size_t initial_count, Handler handler)
    : count_(initial_count),
      handler_([work=make_work(handler), handler]{
          post(work.get_executor(), handler);
        })
  {
  }

  void arrive()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    assert(count_ > 0);
    if (--count_ == 0)
    {
      handler_();
      handler_ = nullptr;
    }
  }

private:
  std::mutex mutex_;
  std::size_t count_;
  std::function<void()> handler_;
};

std::vector<char> read_file(const std::string& filename)
{
  std::ifstream file(filename.c_str());
  std::istreambuf_iterator<char> iter(file.rdbuf()), end;
  return std::vector<char>(iter, end);
}

std::vector<char> process_content(std::vector<char> content)
{
  for (char& c: content)
    c = std::toupper(c);
  return std::move(content);
}

void append_content(std::vector<char>& dest, std::vector<char> source)
{
  dest.insert(dest.end(), source.begin(), source.end());
}

void render_content(const std::vector<char>& content)
{
  std::ostreambuf_iterator<char> iter(std::cout.rdbuf());
  std::copy(content.begin(), content.end(), iter);
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: post_3 <files...>\n";
    return 1;
  }

  std::vector<std::string> src_files;
  for (int i = 1; i < argc; ++i)
    src_files.push_back(argv[i]);

  thread_pool file_pool(8);
  std::vector<std::vector<char>> content_results;
  latch done(src_files.size());
  for (auto src: src_files)
  {
    // Post file read on file_pool and content processing on system_executor.
    post(file_pool,
      [&, src]
      {
        post(
          [&, content=read_file(src)]() mutable
          {
            auto result = process_content(std::move(content));
            content_results.push_back(std::move(result));
            done.arrive();
          });
      });
  }
  done.wait();

  std::vector<char> out;
  strand<system_executor> merge_executor;
  thread_pool ui_executor(1);
  notifying_latch nl(content_results.size(),
      wrap(ui_executor, [&]{ render_content(out); }));
  for (auto& result: content_results)
  {
    post(merge_executor,
      [&]
      {
        append_content(out, result);
        nl.arrive();
      });
  }
  ui_executor.join();
}
