#include <experimental/channel>
#include <experimental/strand>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>
#include <memory>
#include <string>

using std::experimental::channel;
using std::experimental::dispatch;
using std::experimental::dispatch_after;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::wrap;
using std::experimental::yield_context;

void pinger(std::shared_ptr<channel<std::string>> c, yield_context yield)
{
  for (;;)
  {
    c->put("ping", yield);
  }
}

void printer(std::shared_ptr<channel<std::string>> c, yield_context yield)
{
  for (;;)
  {
    std::string msg = c->get(yield);
    std::cout << msg << std::endl;
    dispatch_after(std::chrono::seconds(1), yield);
  }
}

int main()
{
  auto c = std::make_shared<channel<std::string>>();
  strand<system_executor> s;

  dispatch(wrap(s, [&](yield_context yield){ pinger(c, yield); }));
  dispatch(wrap(s, [&](yield_context yield){ printer(c, yield); }));

  std::string input;
  std::getline(std::cin, input);
}
