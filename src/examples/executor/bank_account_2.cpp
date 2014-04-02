#include <experimental/future>
#include <experimental/thread_pool>
#include <iostream>

using std::experimental::make_executor;
using std::experimental::post;
using std::experimental::thread_pool;
using std::experimental::use_future;

// Traditional active object pattern.
// Member functions block until operation is finished.

class bank_account
{
  int balance_ = 0;
  thread_pool pool_{1};
  mutable thread_pool::executor ex_ = make_executor(pool_);

public:
  void deposit(int amount)
  {
    post(ex_, [=]
      {
        balance_ += amount;
      },
      use_future).get();
  }

  void withdraw(int amount)
  {
    post(ex_, [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      },
      use_future).get();
  }

  int balance() const
  {
    return post(ex_, [=]
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
