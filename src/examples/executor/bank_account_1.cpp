#include <experimental/thread_pool>
#include <iostream>

using namespace std::experimental;

// Traditional active object pattern.

class bank_account
{
public:
  explicit bank_account(int i)
    : id_(i)
  {
  }

  void deposit(int amount)
  {
    ex_.dispatch([=]
      {
        balance_ += amount;
      });
  }

  void withdraw(int amount)
  {
    ex_.dispatch([=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      });
  }

  void print()
  {
    ex_.dispatch([=]
      {
        std::cout << "Account " << id_ << " balance is " << balance_ << "\n";
      });
  }

private:
  int id_;
  int balance_ = 0;
  thread_pool pool_{1};
  thread_pool::executor ex_ = get_executor(pool_);
};

int main()
{
  bank_account a(123);
  a.deposit(20);
  a.withdraw(10);
  a.print();
}
