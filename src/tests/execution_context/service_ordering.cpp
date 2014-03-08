#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>

int constructor_count = 0;
int shutdown_count = 0;
int destructor_count = 0;

class service1
  : public std::experimental::execution_context::service
{
public:
  static std::experimental::execution_context::id id;

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
};

std::experimental::execution_context::id service1::id;

class service2
  : public std::experimental::execution_context::service
{
public:
  static std::experimental::execution_context::id id;

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
};

std::experimental::execution_context::id service2::id;

class service3
  : public std::experimental::execution_context::service
{
public:
  static std::experimental::execution_context::id id;

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
};

std::experimental::execution_context::id service3::id;

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
  }

  assert(shutdown_count == 3);
  assert(destructor_count == 3);
}
