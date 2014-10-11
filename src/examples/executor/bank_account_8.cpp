#include <experimental/future>
#include <experimental/strand>
#include <experimental/yield>
#include <iostream>
#include <vector>

using std::experimental::dispatch;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::use_future;
using std::experimental::wrap;
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
    [=](yield_context yield)
    {
      auto largest_acct = end;
      int largest_balance = 0;

      for (auto i = begin; i != end; ++i)
      {
        int balance = i->balance(yield);
        if (largest_acct == end || balance > largest_balance)
        {
          largest_acct = i;
          largest_balance = balance;
        }
      }

      return largest_acct;
    },
    std::forward<CompletionToken>(token));
}

int main()
{
  std::vector<bank_account> accts(3);
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
