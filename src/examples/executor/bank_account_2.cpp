#include <experimental/future>
#include <experimental/strand>
#include <iostream>

using namespace std::experimental;

// Active object sharing a system-wide pool of threads.
// The caller gets to wait for the operation to finish.

class bank_account
{
public:
  explicit bank_account(int i)
    : id_(i)
  {
  }

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
  auto print(CompletionToken&& token)
  {
    return dispatch(ex_, [=]
      {
        std::cout << "Account " << id_ << " balance is " << balance_ << "\n";
      },
      std::forward<CompletionToken>(token));
  }

private:
  int id_;
  int balance_ = 0;
  strand<system_executor> ex_ = make_strand(system_executor());
};

int main()
{
  bank_account a(123);
  a.deposit(20, use_future).get();
  a.withdraw(10, use_future).get();
  a.print(use_future).get();
}
