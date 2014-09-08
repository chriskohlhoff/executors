#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <experimental/executor>
#include <experimental/strand>
#include <experimental/thread_pool>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

using std::experimental::make_work;
using std::experimental::post;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::thread_pool;

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
    std::cerr << "Usage: post_4 <files...>\n";
    return 1;
  }

  std::vector<std::string> src_files;
  for (int i = 1; i < argc; ++i)
    src_files.push_back(argv[i]);

  thread_pool file_pool(8);
  strand<system_executor> merge_executor;
  std::vector<char> out;
  std::size_t pending_results = src_files.size();
  thread_pool ui_executor(1);

  for (auto src: src_files)
  {
    // Post file read on file_pool and content processing on system_executor.
    post(file_pool,
      [&, work=make_work(ui_executor), src]
      {
        post(
          [&, work, content=read_file(src)]() mutable
          {
            post(merge_executor,
              [&, work, result=process_content(std::move(content))]
              {
                append_content(out, result);
                if (--pending_results == 0)
                {
                  post(ui_executor,
                    [&]
                    {
                      render_content(out);
                    });
                }
              });
          });
      });
  }

  ui_executor.join();
}
