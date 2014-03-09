#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>

class base_service1
  : public std::experimental::execution_context::service
{
public:
  typedef base_service1 key_type;

  virtual void do_something() {}

protected:
  explicit base_service1(std::experimental::execution_context& ctx)
    : std::experimental::execution_context::service(ctx)
  {
  }

private:
  void shutdown_service() {}
};

class service1
  : public base_service1
{
public:
  explicit service1(std::experimental::execution_context& ctx)
    : base_service1(ctx)
  {
  }

  virtual void do_something()
  {
  }
};

class base_service2
  : public std::experimental::execution_context::service
{
public:
  typedef base_service2 key_type;

  explicit base_service2(std::experimental::execution_context& ctx)
    : std::experimental::execution_context::service(ctx)
  {
  }

  virtual void do_something() = 0;

private:
  void shutdown_service() {}
};

class service2
  : public base_service2
{
public:
  explicit service2(std::experimental::execution_context& ctx)
    : base_service2(ctx)
  {
  }

  virtual void do_something()
  {
  }
};

class base_service3
  : public std::experimental::execution_context::service
{
public:
  typedef base_service3 key_type;

  explicit base_service3(std::experimental::execution_context& ctx)
    : std::experimental::execution_context::service(ctx)
  {
  }

  virtual void do_something()
  {
  }

private:
  void shutdown_service() {}
};

class service3
  : public base_service3
{
public:
  explicit service3(std::experimental::execution_context& ctx)
    : base_service3(ctx)
  {
  }

  virtual void do_something()
  {
  }
};

int main()
{
  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service1>(scheduler));
    assert(!std::experimental::has_service<service1>(scheduler));
    try
    {
      std::experimental::use_service<base_service1>(scheduler);
      assert(0);
    }
    catch (std::bad_cast&)
    {
    }
    assert(!std::experimental::has_service<base_service1>(scheduler));
    assert(!std::experimental::has_service<service1>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service1>(scheduler));
    assert(!std::experimental::has_service<service1>(scheduler));
    std::experimental::use_service<service1>(scheduler);
    assert(std::experimental::has_service<base_service1>(scheduler));
    assert(std::experimental::has_service<service1>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service1>(scheduler));
    assert(!std::experimental::has_service<service1>(scheduler));
    std::experimental::make_service<service1>(scheduler);
    assert(std::experimental::has_service<base_service1>(scheduler));
    assert(std::experimental::has_service<service1>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service2>(scheduler));
    assert(!std::experimental::has_service<service2>(scheduler));
    try
    {
      std::experimental::use_service<base_service2>(scheduler);
      assert(0);
    }
    catch (std::bad_cast&)
    {
    }
    assert(!std::experimental::has_service<base_service2>(scheduler));
    assert(!std::experimental::has_service<service2>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service2>(scheduler));
    assert(!std::experimental::has_service<service2>(scheduler));
    std::experimental::use_service<service2>(scheduler);
    assert(std::experimental::has_service<base_service2>(scheduler));
    assert(std::experimental::has_service<service2>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service2>(scheduler));
    assert(!std::experimental::has_service<service2>(scheduler));
    std::experimental::make_service<service2>(scheduler);
    assert(std::experimental::has_service<base_service2>(scheduler));
    assert(std::experimental::has_service<service2>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service3>(scheduler));
    assert(!std::experimental::has_service<service3>(scheduler));
    std::experimental::use_service<base_service3>(scheduler);
    assert(std::experimental::has_service<base_service3>(scheduler));
    assert(std::experimental::has_service<service3>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service3>(scheduler));
    assert(!std::experimental::has_service<service3>(scheduler));
    std::experimental::use_service<service3>(scheduler);
    assert(std::experimental::has_service<base_service3>(scheduler));
    assert(std::experimental::has_service<service3>(scheduler));
  }

  {
    std::experimental::loop_scheduler scheduler;
    assert(!std::experimental::has_service<base_service3>(scheduler));
    assert(!std::experimental::has_service<service3>(scheduler));
    std::experimental::make_service<service3>(scheduler);
    assert(std::experimental::has_service<base_service3>(scheduler));
    assert(std::experimental::has_service<service3>(scheduler));
  }
}
