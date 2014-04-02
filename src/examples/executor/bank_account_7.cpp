#include <experimental/future>
#include <experimental/strand>
#include <experimental/yield>
#include <iostream>
#include <vector>

using std::experimental::dispatch;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::use_future;
using std::experimental::yield_context;

// Active object sharing a system-wide pool of threads.
// The caller chooses how to wait for the operation to finish.
// Lightweight, immediate execution using dispatch.
// Composition using resumable functions / stackful coroutines.

class bank_account
{
  int balance_ = 0;
  mutable strand<system_executor> ex_;

public:
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
      [=](yield_context yield)
      {
        for (auto to_acct : to_accts)
        {
          if (balance_ >= amount)
          {
            balance_ -= amount;
            to_acct->deposit(amount, yield);
          }
        }
      },
      std::forward<CompletionToken>(token));
  }
};

int main()
{
  bank_account acct1, acct2, acct3;
  acct1.deposit(20, use_future).get();
  acct2.deposit(30, use_future).get();
  acct3.deposit(40, use_future).get();
  acct1.withdraw(10, use_future).get();
  acct2.transfer(5, { &acct1, &acct3 }, use_future).get();
  std::cout << "Account 1 balance = " << acct1.balance(use_future).get() << "\n";
  std::cout << "Account 2 balance = " << acct2.balance(use_future).get() << "\n";
  std::cout << "Account 3 balance = " << acct3.balance(use_future).get() << "\n";
}
