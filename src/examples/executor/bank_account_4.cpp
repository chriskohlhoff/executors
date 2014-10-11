#include <experimental/executor>
#include <experimental/future>
#include <experimental/thread_pool>
#include <iostream>

using std::experimental::post;
using std::experimental::thread_pool;
using std::experimental::use_future;

// Traditional active object pattern.
// The caller chooses how to wait for the operation to finish.

class bank_account
{
  int balance_ = 0;
  mutable thread_pool pool_{1};

public:
  template <class CompletionToken>
  auto deposit(int amount, CompletionToken&& token)
  {
    return post(pool_, [=]
      {
        balance_ += amount;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto withdraw(int amount, CompletionToken&& token)
  {
    return post(pool_, [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      },
      std::forward<CompletionToken>(token));
  }

  template <class CompletionToken>
  auto balance(CompletionToken&& token) const
  {
    return post(pool_, [=]
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
