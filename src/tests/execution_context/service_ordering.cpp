#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>

int constructor_count = 0;
int fork_prepare_count = 0;
int fork_parent_count = 0;
int shutdown_count = 0;
int destructor_count = 0;

class service1
  : public std::experimental::execution_context::service
{
public:
  service1(std::experimental::execution_context& ctx)
    : std::experimental::execution_context::service(ctx)
  {
    assert(constructor_count == 0);
    ++constructor_count;
  }

  ~service1()
  {
    assert(destructor_count == 2);
    ++destructor_count;
  }

private:
  void shutdown_service()
  {
    assert(shutdown_count == 2);
    ++shutdown_count;
  }

  void notify_fork(std::experimental::fork_event e)
  {
    if (e == std::experimental::fork_event::prepare)
    {
      assert(fork_prepare_count == 2);
      ++fork_prepare_count;
    }
    else
    {
      assert(fork_parent_count == 0);
      ++fork_parent_count;
    }
  }
};

class service2
  : public std::experimental::execution_context::service
{
public:
  explicit service2(std::experimental::execution_context& ctx)
    : std::experimental::execution_context::service(ctx)
  {
    assert(constructor_count == 1);
    ++constructor_count;
  }

  ~service2()
  {
    assert(destructor_count == 1);
    ++destructor_count;
  }

private:
  void shutdown_service()
  {
    assert(shutdown_count == 1);
    ++shutdown_count;
  }

  void notify_fork(std::experimental::fork_event e)
  {
    if (e == std::experimental::fork_event::prepare)
    {
      assert(fork_prepare_count == 1);
      ++fork_prepare_count;
    }
    else
    {
      assert(fork_parent_count == 1);
      ++fork_parent_count;
    }
  }
};

class service3
  : public std::experimental::execution_context::service
{
public:
  explicit service3(std::experimental::execution_context& ctx)
    : std::experimental::execution_context::service(ctx)
  {
    assert(constructor_count == 2);
    ++constructor_count;
  }

  ~service3()
  {
    assert(destructor_count == 0);
    ++destructor_count;
  }

private:
  void shutdown_service()
  {
    assert(shutdown_count == 0);
    ++shutdown_count;
  }

  void notify_fork(std::experimental::fork_event e)
  {
    if (e == std::experimental::fork_event::prepare)
    {
      assert(fork_prepare_count == 0);
      ++fork_prepare_count;
    }
    else
    {
      assert(fork_parent_count == 2);
      ++fork_parent_count;
    }
  }
};

int main()
{
  {
    std::experimental::loop_scheduler scheduler;

    assert(!std::experimental::has_service<service1>(scheduler));
    assert(!std::experimental::has_service<service2>(scheduler));
    assert(!std::experimental::has_service<service3>(scheduler));

    std::experimental::use_service<service1>(scheduler);

    assert(std::experimental::has_service<service1>(scheduler));
    assert(!std::experimental::has_service<service2>(scheduler));
    assert(!std::experimental::has_service<service3>(scheduler));

    std::experimental::use_service<service2>(scheduler);

    assert(std::experimental::has_service<service1>(scheduler));
    assert(std::experimental::has_service<service2>(scheduler));
    assert(!std::experimental::has_service<service3>(scheduler));

    std::experimental::use_service<service3>(scheduler);

    assert(std::experimental::has_service<service1>(scheduler));
    assert(std::experimental::has_service<service2>(scheduler));
    assert(std::experimental::has_service<service3>(scheduler));

    assert(constructor_count == 3);

    scheduler.notify_fork(std::experimental::fork_event::prepare);

    assert(fork_prepare_count == 3);

    scheduler.notify_fork(std::experimental::fork_event::parent);

    assert(fork_parent_count == 3);
  }

  assert(shutdown_count == 3);
  assert(destructor_count == 3);
}
