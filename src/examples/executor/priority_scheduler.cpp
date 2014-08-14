#include <experimental/executor>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>

using std::experimental::dispatch;
using std::experimental::execution_context;

class priority_scheduler : public execution_context
{
public:
  // A class that satisfies the Executor requirements.
  class executor_type
  {
  public:
    executor_type(priority_scheduler& ctx, int pri)
      : context_(ctx), priority_(pri)
    {
    }

    priority_scheduler& context() noexcept
    {
      return context_;
    }

    void on_work_started() noexcept
    {
      // This executor doesn't count work.
    }

    void on_work_finished() noexcept
    {
      // This executor doesn't count work.
    }

    template <class Func, class Alloc>
    void dispatch(Func&& f, const Alloc& a)
    {
      post(std::forward<Func>(f), a);
    }

    template <class Func, class Alloc>
    void post(Func f, const Alloc& a)
    {
      auto p(std::allocate_shared<item<Func>>(a, priority_, std::move(f)));
      std::lock_guard<std::mutex> lock(context_.mutex_);
      context_.queue_.push(p);
    }

    template <class Func, class Alloc>
    void defer(Func&& f, const Alloc& a)
    {
      post(std::forward<Func>(f), a);
    }

  private:
    priority_scheduler& context_;
    int priority_;
  };

  executor_type get_executor(int pri = 0) const noexcept
  {
    return executor_type(*const_cast<priority_scheduler*>(this), pri);
  }

  void run()
  {
    for (std::unique_lock<std::mutex> lock(mutex_);! queue_.empty(); lock.lock())
    {
      auto p(queue_.top());
      queue_.pop();
      lock.unlock();
      p->execute_(p);
    }
  }

private:
  struct item_base
  {
    int priority_;
    std::size_t seq_num_;
    void (*execute_)(std::shared_ptr<item_base>&);
  };

  template <class Func>
  struct item : item_base
  {
    item(int pri, Func f) : function(std::move(f))
    {
      priority_ = pri;
      execute_ = [](auto p)
      {
        Func f(std::move(static_cast<item*>(p.get())->function));
        p.reset();
        std::move(f)();
      };
    }

    Func function;
  };

  struct item_comp
  {
    bool operator()(
        const std::shared_ptr<item_base>& a,
        const std::shared_ptr<item_base>& b)
    {
      return a->priority_ < b->priority_;
    }
  };

  std::mutex mutex_;
  std::priority_queue<
    std::shared_ptr<item_base>,
    std::vector<std::shared_ptr<item_base>>,
    item_comp> queue_;
};

namespace std { namespace experimental { inline namespace concurrency_v1 {
  template <> struct is_executor<priority_scheduler::executor_type> : std::true_type {};
}}}

int main()
{
  priority_scheduler sched;
  auto low = sched.get_executor(0);
  auto med = sched.get_executor(1);
  auto high = sched.get_executor(2);
  dispatch(low, []{ std::cout << "1\n"; });
  dispatch(low, []{ std::cout << "11\n"; });
  dispatch(med, []{ std::cout << "2\n"; });
  dispatch(med, []{ std::cout << "22\n"; });
  dispatch(high, []{ std::cout << "3\n"; });
  dispatch(high, []{ std::cout << "33\n"; });
  dispatch(high, []{ std::cout << "333\n"; });
  sched.run();
}
