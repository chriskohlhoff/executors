#include <experimental/executor>
#include <iostream>

using std::experimental::dispatch;
using std::experimental::strand;
using std::experimental::system_executor;
using std::experimental::use_future;

// Active object sharing a system-wide pool of threads.
// Member functions block until operation is finished.

class bank_account
{
  int balance_ = 0;
  mutable strand<system_executor> strand_;

public:
  void deposit(int amount)
  {
    dispatch(strand_,
      use_future([=]
        {
          balance_ += amount;
        })).get();
  }

  void withdraw(int amount)
  {
    dispatch(strand_,
      use_future([=]
        {
          if (balance_ >= amount)
            balance_ -= amount;
        })).get();
  }

  int balance() const
  {
    return dispatch(strand_,
      use_future([=]
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
