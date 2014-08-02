#include <experimental/future>
#include <experimental/thread_pool>
#include <iostream>

using std::experimental::post;
using std::experimental::thread_pool;
using std::experimental::use_future;

// Traditional active object pattern.
// Member functions block until operation is finished.

class bank_account
{
  int balance_ = 0;
  mutable thread_pool pool_{1};

public:
  void deposit(int amount)
  {
    post(pool_, [=]
      {
        balance_ += amount;
      },
      use_future).get();
  }

  void withdraw(int amount)
  {
    post(pool_, [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      },
      use_future).get();
  }

  int balance() const
  {
    return post(pool_, [=]
      {
        return balance_;
      },
      use_future).get();
  }
};

int main()
{
  bank_account acct;
  acct.deposit(20);
  acct.withdraw(10);
  std::cout << "balance = " << acct.balance() << "\n";
}
