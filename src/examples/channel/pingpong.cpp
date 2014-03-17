#include <experimental/channel>
#include <experimental/strand>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>
#include <string>

using namespace std::experimental;

void pinger(channel<std::string>& c, yield_context yield)
{
  for (int i = 0; ; ++i)
  {
    c.put("ping", yield);
  }
}

void ponger(channel<std::string>& c, yield_context yield)
{
  for (int i = 0; ; ++i)
  {
    c.put("pong", yield);
  }
}

void printer(channel<std::string>& c, yield_context yield)
{
  for (;;)
  {
    std::string msg = c.get(yield);
    std::cout << msg << std::endl;
    dispatch_after(std::chrono::seconds(1), yield);
  }
}

int main()
{
  channel<std::string> c;
  auto s = make_strand(system_executor());

  dispatch(s.wrap([&](yield_context yield){ pinger(c, yield); }));
  dispatch(s.wrap([&](yield_context yield){ ponger(c, yield); }));
  dispatch(s.wrap([&](yield_context yield){ printer(c, yield); }));

  std::string input;
  std::getline(std::cin, input);
}
