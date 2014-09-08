#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <experimental/strand>
#include <experimental/thread_pool>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

using std::experimental::loop_scheduler;
using std::experimental::make_work;
using std::experimental::post;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::thread_pool;

class content_pipeline
{
public:
  void run(const std::vector<std::string>& src_files)
  {
    // Reset internal state.
    out_.clear();
    pending_results_ = src_files.size();
    ui_executor_.restart();

    // Kick of the first stage of the pipeline.
    for (auto src: src_files)
      read_file(src);

    // Run the ui executor until the pipeline is complete.
    ui_executor_.run();
  }

private:
  void read_file(const std::string& filename)
  {
    post(file_pool_,
      [this, work=make_work(ui_executor_), filename]
      {
        std::ifstream file(filename.c_str());
        std::istreambuf_iterator<char> iter(file.rdbuf()), end;
        process_content(std::vector<char>(iter, end));
      });
  }

  void process_content(std::vector<char> content)
  {
    post(
      [this, work=make_work(ui_executor_), content=std::move(content)]() mutable
      {
        for (char& c: content)
          c = std::toupper(c);
        merge_content(std::move(content));
      });
  }

  void merge_content(std::vector<char> result)
  {
    post(merge_executor_,
      [this, work=make_work(ui_executor_), result=std::move(result)]
      {
        out_.insert(out_.end(), result.begin(), result.end());
        if (--pending_results_ == 0)
          render_content();
      });
  }

  void render_content()
  {
    post(ui_executor_,
      [this]
      {
        std::ostreambuf_iterator<char> iter(std::cout.rdbuf());
        std::copy(out_.begin(), out_.end(), iter);
      });
  }

  thread_pool file_pool_{8};
  strand<system_executor> merge_executor_;
  std::vector<char> out_;
  std::size_t pending_results_;
  loop_scheduler ui_executor_{1};
};

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: post_5 <files...>\n";
    return 1;
  }

  std::vector<std::string> src_files;
  for (int i = 1; i < argc; ++i)
    src_files.push_back(argv[i]);

  content_pipeline pipeline;
  pipeline.run(src_files);
}
