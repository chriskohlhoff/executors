#include <experimental/await>
#include <experimental/future>
#include <experimental/strand>
#include <experimental/thread_pool>
#include <iostream>
#include <vector>

using std::experimental::await_context;
using std::experimental::dispatch;
using std::experimental::executor;
using std::experimental::strand;
using std::experimental::thread_pool;
using std::experimental::use_future;
using std::experimental::wrap;

// Caller specifies an executor type at compile time.
// The caller chooses how to wait for the operation to finish.
// Lightweight, immediate execution using dispatch.
// Composition using resumable functions / stackless coroutines.

template <class Executor>
class bank_account
{
  int balance_ = 0;
  mutable strand<Executor> ex_;

public:
  explicit bank_account(const Executor& ex)
    : ex_(ex)
  {
  }

  template <class CompletionToken>
  auto deposit(int amount, CompletionToken&& token)
  {
    return dispatch(ex_, [=]
      {
        balance_ += amount;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto withdraw(int amount, CompletionToken&& token)
  {
    return dispatch(ex_, [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto balance(CompletionToken&& token) const
  {
    return dispatch(ex_, [=]
      {
        return balance_;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto transfer(int amount, bank_account& to_acct, CompletionToken&& token)
  {
    return dispatch(
      wrap(ex_, [=]
        {
          if (balance_ >= amount)
          {
            balance_ -= amount;
            return amount;
          }

          return 0;
        }),
      wrap(to_acct.ex_,
        [&to_acct](int deducted)
        {
          to_acct.balance_ += deducted;
        }),
      std::forward<CompletionToken>(token));
  }
};

template <class Iterator, class CompletionToken>
auto find_largest_account(Iterator begin, Iterator end, CompletionToken&& token)
{
  return dispatch(
    [i = begin, end, largest_acct = end, balance = int(), largest_balance = int()]
    (await_context ctx) mutable
    {
      reenter (ctx)
      {
        for (; i != end; ++i)
        {
          await balance = i->balance(ctx);
          if (largest_acct == end || balance > largest_balance)
          {
            largest_acct = i;
            largest_balance = balance;
          }
        }
      }
      return largest_acct;
    },
    std::forward<CompletionToken>(token));
}

int main()
{
  thread_pool pool;
  auto ex = pool.get_executor();
  std::vector<bank_account<thread_pool::executor_type>> accts(3, bank_account<thread_pool::executor_type>(ex));
  accts[0].deposit(20, use_future).get();
  accts[1].deposit(30, use_future).get();
  accts[2].deposit(40, use_future).get();
  accts[0].withdraw(10, use_future).get();
  accts[1].transfer(5, accts[0], use_future).get();
  accts[2].transfer(15, accts[1], use_future).get();
  std::cout << "Account 0 balance = " << accts[0].balance(use_future).get() << "\n";
  std::cout << "Account 1 balance = " << accts[1].balance(use_future).get() << "\n";
  std::cout << "Account 2 balance = " << accts[2].balance(use_future).get() << "\n";
  auto largest = find_largest_account(accts.begin(), accts.end(), use_future).get();
  std::cout << "Largest balance = " << largest->balance(use_future).get() << "\n";
}
