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

// Caller specifies an executor type at runtime.
// The caller chooses how to wait for the operation to finish.
// Lightweight, immediate execution using dispatch.
// Composition using resumable functions / stackless coroutines.

class bank_account
{
  int balance_ = 0;
  mutable strand<executor> ex_;

public:
  explicit bank_account(const executor& ex)
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
  auto transfer(int amount, std::vector<bank_account*> to_accts, CompletionToken&& token)
  {
    return dispatch(ex_,
      [=, i = std::size_t()](await_context ctx) mutable
      {
        reenter (ctx)
        {
          for (i = 0; i < to_accts.size(); ++i)
          {
            if (balance_ >= amount)
            {
              balance_ -= amount;
              await to_accts[i]->deposit(amount, ctx);
            }
          }
        }
      },
      std::forward<CompletionToken>(token));
  }
};

int main()
{
  thread_pool pool;
  auto ex = make_executor(pool);
  bank_account acct1(ex), acct2(ex), acct3(ex);
  acct1.deposit(20, use_future).get();
  acct2.deposit(30, use_future).get();
  acct3.deposit(40, use_future).get();
  acct1.withdraw(10, use_future).get();
  acct2.transfer(5, { &acct1, &acct3 }, use_future).get();
  std::cout << "Account 1 balance = " << acct1.balance(use_future).get() << "\n";
  std::cout << "Account 2 balance = " << acct2.balance(use_future).get() << "\n";
  std::cout << "Account 3 balance = " << acct3.balance(use_future).get() << "\n";
}
