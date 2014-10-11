#include <experimental/executor>
#include <experimental/future>
#include <experimental/strand>
#include <iostream>

using std::experimental::package;
using std::experimental::post;
using std::experimental::strand;
using std::experimental::system_executor;

// Active object sharing a system-wide pool of threads.
// Member functions block until operation is finished.

class bank_account
{
  int balance_ = 0;
  mutable strand<system_executor> strand_;

public:
  void deposit(int amount)
  {
    post(strand_,
      package([=]
        {
          balance_ += amount;
        })).get();
  }

  void withdraw(int amount)
  {
    post(strand_,
      package([=]
        {
          if (balance_ >= amount)
            balance_ -= amount;
        })).get();
  }

  int balance() const
  {
    return post(strand_,
      package([=]
        {
          return balance_;
        })).get();
  }
};

int main()
{
  bank_account acct;
  acct.deposit(20);
  acct.withdraw(10);
  std::cout << "balance = " << acct.balance() << "\n";
}
