#include <experimental/executor>
#include <experimental/future>
#include <experimental/strand>
#include <iostream>

using std::experimental::dispatch;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::use_future;

// Active object sharing a system-wide pool of threads.
// The caller chooses how to wait for the operation to finish.
// Lightweight, immediate execution using dispatch.

class bank_account
{
  int balance_ = 0;
  mutable strand<system_executor> strand_;

public:
  template <class CompletionToken>
  auto deposit(int amount, CompletionToken&& token)
  {
    return dispatch(strand_, [=]
      {
        balance_ += amount;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto withdraw(int amount, CompletionToken&& token)
  {
    return dispatch(strand_, [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto balance(CompletionToken&& token) const
  {
    return dispatch(strand_, [=]
      {
        return balance_;
      },
      std::forward<CompletionToken>(token));
  }
};

int main()
{
  bank_account acct1;
  acct1.deposit(20, []{ std::cout << "deposit complete\n"; });
  acct1.withdraw(10, []{ std::cout << "withdraw complete\n"; });
  acct1.balance([](int b){ std::cout << "balance = " << b << "\n"; });

  bank_account acct2;
  acct2.deposit(40, use_future).get();
  acct2.withdraw(15, use_future).get();
  std::cout << "balance = " << acct2.balance(use_future).get() << "\n";
}
