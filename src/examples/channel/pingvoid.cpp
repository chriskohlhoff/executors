#include <experimental/channel>
#include <experimental/strand>
#include <experimental/timer>
#include <experimental/yield>
#include <iostream>
#include <string>

using namespace std::experimental;

void pinger(channel<void>& c, yield_context yield)
{
  for (int i = 0; ; ++i)
  {
    c.put(yield);
  }
}

void printer(channel<void>& c, yield_context yield)
{
  for (;;)
  {
    c.get(yield);
    std::cout << "void" << std::endl;
    dispatch_after(std::chrono::seconds(1), yield);
  }
}

int main()
{
  channel<void> c;
  auto s = make_strand(system_executor());

  dispatch(s.wrap([&](yield_context yield){ pinger(c, yield); }));
  dispatch(s.wrap([&](yield_context yield){ printer(c, yield); }));

  std::string input;
  std::getline(std::cin, input);
}
