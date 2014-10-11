#include <experimental/future>
#include <experimental/strand>
#include <iostream>

using std::experimental::dispatch;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::use_future;
using std::experimental::wrap;

// Active object sharing a system-wide pool of threads.
// The caller chooses how to wait for the operation to finish.
// Lightweight, immediate execution using dispatch.
// Composition using variadic dispatch.

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

int main()
{
  bank_account acct1, acct2;
  acct1.deposit(20, use_future).get();
  acct2.deposit(30, use_future).get();
  acct1.withdraw(10, use_future).get();
  acct2.transfer(5, acct1, use_future).get();
  std::cout << "Account 1 balance = " << acct1.balance(use_future).get() << "\n";
  std::cout << "Account 2 balance = " << acct2.balance(use_future).get() << "\n";
}
