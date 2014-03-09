#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>

class service1
  : public std::experimental::execution_context::service
{
public:
  explicit service1(std::experimental::execution_context& ctx, int = 42)
    : std::experimental::execution_context::service(ctx)
  {
  }

private:
  void shutdown_service() {}
};

class service2
  : public std::experimental::execution_context::service
{
public:
  explicit service2(std::experimental::execution_context& ctx, int)
    : std::experimental::execution_context::service(ctx)
  {
  }

private:
  void shutdown_service() {}
};

int main()
{
  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<service1>(scheduler));
    std::experimental::use_service<service1>(scheduler);
    assert(std::experimental::has_service<service1>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<service1>(scheduler));
    std::experimental::make_service<service1>(scheduler);
    assert(std::experimental::has_service<service1>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<service1>(scheduler));
    std::experimental::make_service<service1>(scheduler, 123);
    assert(std::experimental::has_service<service1>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<service2>(scheduler));
    try
    {
      std::experimental::use_service<service2>(scheduler);
      assert(0);
    }
    catch (std::bad_cast&)
    {
    }
    assert(!std::experimental::has_service<service2>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<service2>(scheduler));
    std::experimental::make_service<service2>(scheduler, 123);
    assert(std::experimental::has_service<service2>(scheduler));
    std::experimental::use_service<service2>(scheduler);
  }
}
