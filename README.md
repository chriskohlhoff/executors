A C++14 library for executors
=============================

> **NOTE:**
>
> The library described below is more extensive than that included in the latest standards proposal. To see what forms part of the proposal, visit [http://chriskohlhoff.github.io/executors/](http://chriskohlhoff.github.io/executors/).

This is a potential standard library proposal that covers:

* [Executors](#executors)
  * Executors and schedulers
  * Resumable functions / coroutines
  * A model for asynchronous operations
  * An alternative to `std::async()`
* [Timers](#timers)
* [Channels](#channels)

It has been tested with g++ 4.8.2, g++ 4.9 and clang 3.4, each using the `-std=c++1y` compiler option.

Executors
---------

The central concept of this library is the **executor**. An executor embodies a set of rules about where, when and how to run a function object. For example:

Type of executor | Where, when and how
---------------- | -------------------
System           | Any thread in the process.
Thread pool      | Any thread in the pool, and nowhere else.
Strand           | Not concurrent with any other function object sharing the strand, and in FIFO order.
Future / Promise | Any thread. Capture any exceptions thrown by the function object and store them in the promise.

Executors are ultimately defined by a set of type requirements, so the set of executors isn't limited to those listed here. Like allocators, library users can develop custom executor types to implement their own rules.

To submit a function object to an executor, we can choose from one of three fundamental operations: **dispatch**, **post** and **defer**. These operations differ in the eagerness with which they run the submitted function.

A dispatch operation is the most eager, and used when we want to run a function object according to an executor’s rules, but in the cheapest way available:

    void f1()
    {
      std::cout << "Hello, world!\n";
    }

    // ...

    dispatch(ex, f1);

By performing a dispatch operation, we are giving the executor `ex` the option of having `dispatch()` run the submitted function object before it returns. Whether an executor does this depends on its rules:

Type of executor | Behaviour of dispatch
---------------- | ---------------------
System           | Always runs the function object before returning from `dispatch()`.
Thread pool      | If we're inside the thread pool, runs the function object before returning from `dispatch()`. Otherwise, adds to the thread pool's work queue.
Strand           | If we're inside the strand, or if the strand queue is empty, runs the function object before returning from `dispatch()`. Otherwise, adds to the strand's work queue.
Future / Promise | Wraps the function object in a try/catch block, and runs it before returning from `dispatch()`.

The consequence of this is that, if the executor’s rules allow it, the compiler is able to inline the function call.

A post operation, on the other hand, is not permitted to run the function object itself.

    post(ex, f1);

A posted function is scheduled for execution as soon as possible, but according to the rules of the executor:

Type of executor | Behaviour of post
---------------- | -----------------
System           | Adds the function object to a system thread pool's work queue.
Thread pool      | Adds the function object to the thread pool's work queue.
Strand           | Adds the function object to the strand's work queue.
Future / Promise | Wraps the function object in a try/catch block, and adds it to the system work queue.

Finally, the defer operation is the least eager of the three.

    defer(ex, f1);

A defer operation is similar to a post operation, except that it implies a relationship between the caller and the function object being submitted. It is intended for use when submitting a function object that represents a continuation of the caller.

Type of executor | Behaviour of defer
---------------- | ------------------
System           | If the caller is executing within the system-wide thread pool, saves the function object to a thread-local queue. Once control returns to the system thread pool, the function object is scheduled for execution as soon as possible. If the caller is not inside the system thread pool, behaves as a post operation.
Thread pool      | If the caller is executing within the thread pool, saves the function object to a thread-local queue. Once control returns to the thread pool, the function object is scheduled for execution as soon as possible. If the caller is not inside the specified thread pool, behaves as a post operation.
Strand           | Adds the function object to the strand's work queue.
Future / Promise | Wraps the function object in a try/catch block, and delegates to the system executor for deferral.

### Posting functions to a thread pool

As a simple example, let us consider how to implement the Active Object design pattern using the executors library. In the Active Object pattern, all operations associated with an object are run on its own private thread.

    class bank_account
    {
      int balance_ = 0;
      std::experimental::thread_pool pool_{1};

    public:
      void deposit(int amount)
      {
        post(pool_, [=]
          {
            balance_ += amount;
          });
      }

      void withdraw(int amount)
      {
        post(pool_, [=]
          {
            if (balance_ >= amount)
              balance_ -= amount;
          });
      }
    };

> *Full example: [bank_account_1.cpp](src/examples/executor/bank_account_1.cpp)*

First, we create a private thread pool with a single thread:

    std::experimental::thread_pool pool_{1};

A thread pool is an example of an **execution context**. An execution context represents a place where function objects will be executed. This is distinct from an executor which, as an embodiment of a set of rules, is intended to be a lightweight object that is cheap to copy and wrap for further adaptation.

To add the function to the thread pool's queue, we use a post operation:

    post(pool_, [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      });

For convenience the post function is overloaded for execution contexts, such as `thread_pool`, to take care of obtaining the executor for you. The above call is equivalent to:

    post(pool_.get_executor(), [=]
      {
        if (balance_ >= amount)
          balance_ -= amount;
      });

### Waiting for function completion

When implementing the Active Object pattern, we will normally want to wait for the operation to complete. To do this we can reimplement our `bank_account` member functions to pass an additional **completion token** to the free function `post()`. A completion token specifies how we want to be notified when the function finishes. For example:

    void withdraw(int amount)
    {
      std::future<void> fut = std::experimental::post(pool_, [=]
        {
          if (balance_ >= amount)
            balance_ -= amount;
        },
        std::experimental::use_future);
      fut.get();
    }

> *Full example: [bank_account_2.cpp](src/examples/executor/bank_account_2.cpp)*

Here, the `use_future` completion token is specified. When passed the `use_future` token, the free function `post()` returns the result via a `std::future`.

Other types of completion token include plain function objects (used as callbacks), resumable functions or coroutines, and even user-defined types. If we want our active object to accept any type of completion token, we simply change the member functions to accept the token as a template parameter:

    template <class CompletionToken>
    auto withdraw(int amount, CompletionToken&& token)
    {
      return std::experimental::post(pool_, [=]
        {
          if (balance_ >= amount)
            balance_ -= amount;
        },
        std::forward<CompletionToken>(token));
    }

> *Full example: [bank_account_3.cpp](src/examples/executor/bank_account_3.cpp)*

The caller of this function can now choose how to receive the result of the operation, as opposed to having a single strategy hard-coded in the `bank_account` implementation. For example, the caller could choose to receive the result via a `std::future`:

    bank_account acct;
    // ...
    std::future<void> fut = acct.withdraw(10, std::experimental::use_future);
    fut.get();

or callback:

    acct.withdraw(10, []{ std::cout << "withdraw complete\n"; });

or any other type that meets the completion token requirements. This approach also works for functions that return a value:

    class bank_account
    {
      // ...

      template <class CompletionToken>
      auto balance(CompletionToken&& token) const
      {
        return std::experimental::post(pool_, [=]
          {
            return balance_;
          },
          std::forward<CompletionToken>(token));
      }
    };

When using `use_future`, the future's value type is determined automatically from the executed function's return type:

    std::future<int> fut = acct.balance(std::experimental::use_future);
    std::cout << "balance is " << fut.get() << "\n";

Similarly, when using a callback, the function's result is passed as an argument:

    acct.balance([](int bal){ std::cout << "balance is " << bal << "\n"; });

### Limiting concurrency using strands

Clearly, having a private thread for each `bank_account` is not going to scale well to thousands or millions of objects. We may instead want all bank accounts to share a thread pool. The `system_executor` object provides access to a system thread pool which we can use for this purpose:

    std::experimental::system_executor ex;
    post(ex, []{ std::cout << "Hello, world!\n"; });

However, the system thread pool uses an unspecified number of threads, and the posted function could run on any of them. The original reason for using the Active Object pattern was to limit the `bank_account` object's internal logic to run on a single thread. Fortunately, we can also limit concurrency by using the `strand<>` template.

The `strand<>` template is an executor that acts as an adapter for other executors. In addition to the rules of the underlying executor, a strand adds a guarantee of non-concurrency. That is, it guarantees that no two function objects submitted to the strand will run in parallel.

We can convert the `bank_account` class to use a strand very simply:

    class bank_account
    {
      int balance_ = 0;
      mutable std::experimental::strand<std::experimental::system_executor> ex_;

    public:
      // ...
    };

> *Full example: [bank_account_4.cpp](src/examples/executor/bank_account_4.cpp)*

### Lightweight, immediate execution using dispatch

As noted above, a post operation always submits a function object for later execution. This means that when we write:

    template <class CompletionToken>
    auto withdraw(int amount, CompletionToken&& token)
    {
      return std::experimental::post(ex_, [=]
        {
          if (balance_ >= amount)
            balance_ -= amount;
        },
        std::forward<CompletionToken>(token));
    }

we will always incur the cost of a context switch (plus an extra context switch if we wait for the result using a future). This cost can be avoided if we use a dispatch operation instead. The system executor's rules allow it to run a function object on any thread, so if we change the `withdraw` function to:

    template <class CompletionToken>
    auto withdraw(int amount, CompletionToken&& token)
    {
      return std::experimental::dispatch(ex_, [=]
        {
          if (balance_ >= amount)
            balance_ -= amount;
        },
        std::forward<CompletionToken>(token));
    }

> *Full example: [bank_account_4.cpp](src/examples/executor/bank_account_4.cpp)*

then the enclosed function object can be executed before `dispatch()` returns. The only condition where it will run later is when the strand is already busy on another thread. In this case, in order to meet the strand's non-concurrency guarantee, the function object must be added to the strand's work queue. In the common case there is no contention on the strand and the cost is minimised.

### Composition using variadic dispatch

Let us now add a function to transfer balance from one bank account to another. To implement this function we must coordinate code on two distinct executors: the strands that belong to each of the bank accounts.

A first attempt at solving this might use a `std::future`:

    class bank_account
    {
      // ...

      template <class CompletionToken>
      auto transfer(bank_account& to_acct, CompletionToken&& token)
      {
        return std::experimental::dispatch(ex_, [=, &to_acct]
          {
            if (balance_ >= amount)
            {
              balance_ -= amount;
              std::future<void> fut = to_acct.deposit(amount, std::experimental::use_future);
              fut.get();
            }
          },
          std::forward<CompletionToken>(token));
      }
    };

While correct, this approach has the side effect of blocking the thread until the future is ready. If the `to_acct` object's strand is busy running other function objects, this might take some time.

In the examples so far, you might have noticed that sometimes we call `post()` or `dispatch()` with just one function object, and sometimes we call them with both a function object and a completion token.

Both `post()` and `dispatch()` are variadic template functions that can accept a number of completion tokens. For example, the library defines `dispatch()` as:

    auto dispatch(t0, t1, ..., tN);
    auto dispatch(executor, t0, t1, ..., tN);

where *t0* to *tN* are completion tokens. When we call `dispatch()`, the library turns each token into a function object and calls these functions in sequence. The return value of any given function is passed as an argument to the next one. For example:

    std::future<std::string> fut = std::experimental::dispatch(ex_,
      []{ return 1; },
      [](int i) { return i + 1; },
      [](int i) { return i * 2; },
      [](int i) { return std::to_string(i); },
      [](std::string s) { return "value is " + s; },
      std::experimental::use_future);
    std::cout << fut.get() << std::endl;

will output the string `value is 4`.

For our bank account example, what is more important is that the variadic `post()` and `dispatch()` functions let you run each function object on a different executor. We can then write our `transfer()` function as follows:

    template <class CompletionToken>
    auto transfer(int amount, bank_account& to_acct, CompletionToken&& token)
    {
      return std::experimental::dispatch(
        std::experimental::wrap(ex_, [=]
          {
            if (balance_ >= amount)
            {
              balance_ -= amount;
              return amount;
            }

            return 0;
          }),
        std::experimental::wrap(to_acct.ex_,
          [&to_acct](int deducted)
          {
            to_acct.balance_ += deducted;
          }),
        std::forward<CompletionToken>(token));
    }

> *Full example: [bank_account_5.cpp](src/examples/executor/bank_account_5.cpp)*

Here, the first function object:

    std::experimental::wrap(ex_, [=]
      {
        if (balance_ >= amount)
        {
          balance_ -= amount;
          return amount;
        }

        return 0;
      }),

is run on the source account's strand `ex_`. We wrap the function object using `wrap(ex_, ...)` to tell `dispatch()` which executor to use.

The amount that is successfully deducted is then passed to the second function object:

    std::experimental::wrap(to_acct.ex_,
      [&to_acct](int deducted)
      {
        to_acct.balance_ += deducted;
      }),

which, again thanks to `wrap()`, is run on the `to_acct` object's strand. By running each function object on a specific executor, we ensure that both `bank_account` objects are updated in a thread-safe way.

### Composition using resumable functions

Variadic `post()` and `dispatch()` are useful for strictly sequential task flow, but for more complex control flow the executors library offers another approach: resumable functions, or coroutines. These coroutines come in two flavours, stackless and stackful, and to illustrate them we will now add a function to find the bank account with the largest balance.

Stackless coroutines are identified by having a last argument of type `await_context`:

    template <class Iterator, class CompletionToken>
    auto find_largest_account(Iterator begin, Iterator end, CompletionToken&& token)
    {
      return std::experimental::dispatch(
        [i = begin, end, largest_acct = end, balance = int(), largest_balance = int()]
        (std::experimental::await_context ctx) mutable
        {
          reenter (ctx)
          {
            for (; i != end; ++i)
            {
              await balance = i->balance(ctx);
              if (largest_acct == end || balance > largest_balance)
              {
                largest_acct = i;
                largest_balance = balance;
              }
            }
          }
          return largest_acct;
        },
        std::forward<CompletionToken>(token));
    }

> *Full example: [bank_account_6.cpp](src/examples/executor/bank_account_6.cpp)*

In this library, stackless coroutines are implemented using macros and a switch-based mechanism similar to Duff's Device. The `reenter` macro:

    reenter (ctx)

causes control to jump to the resume point that is saved in the `await_context` object named `ctx` (or the start of the block if this is the first time the coroutine has been entered). The `await` macro:

    await balance = i->balance(ctx);

Stores the resume point into `ctx` and suspends the coroutine. The `ctx` object is a completion token that causes the coroutine to automatically resume when the `balance()` operation completes. Once the coroutine resumes, the `balance` variable contains the result of the operation.

As the name suggests, stackless coroutines have no stack that persists across a resume point. As we cannot use any stack-based variables, we instead use a lambda capture to define our "locals":

    [i = begin, end, largest_acct = end, balance = int(), largest_balance = int()]

The lambda object is guaranteed to exist until the coroutine completes, and it is safe to take the address of these variables across a suspend/resume point.

Stackful coroutines are identified by having a last argument of type `yield_context`:

    template <class Iterator, class CompletionToken>
    auto find_largest_account(Iterator begin, Iterator end, CompletionToken&& token)
    {
      return std::experimental::dispatch(
        [=](std::experimental::yield_context yield)
        {
          auto largest_acct = end;
          int largest_balance;

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

> *Full example: [bank_account_7.cpp](src/examples/executor/bank_account_7.cpp)*

The `yield` object is a completion token that means that, when the call out to a bank account object is reached:

    int balance = i->balance(yield);

the library implementation automatically suspends the current function. The thread is not blocked and remains available to process other function objects. Once the `balance()` operation completes, the `find_largest_account` function resumes execution at the following statement.

These stackful resumable functions are implemented entirely as a library construct, and require no alteration to the language in the form of new keywords. Consequently, they can utilise artbitrarily complex control flow constructs, including stack-based variables, while still retaining concise, familiar C++ language use.

### Polymorphic executors

Up to this point, our `bank_account` class's executor has been a private implementation detail. However, rather than limit ourselves to the system executor, we will now alter the class to be able to specify the executor on a case-by-case basis.

Ultimately, executors are defined by a set of type requirements, and each of the executors we have used so far is a distinct type. For optimal performance we can use compile-time polymorphism, and specify the executor as a template parameter:

    template <class Executor>
    class bank_account
    {
      int balance_ = 0;
      mutable std::experimental::strand<Executor> ex_;

    public:
      explicit bank_account(const Executor& ex)
        : ex_(ex)
      {
      }

      // ...
    };

> *Full example: [bank_account_8.cpp](src/examples/executor/bank_account_8.cpp)*

On the other hand, in many situations runtime polymorphism will be preferred. To support this, the library provides the `executor` class, a polymorphic wrapper:

    class bank_account
    {
      int balance_ = 0;
      mutable std::experimental::strand<std::experimental::executor> ex_;

    public:
      explicit bank_account(const std::experimental::executor& ex = std::experimental::system_executor())
        : ex_(ex)
      {
      }

      // ...
    };

> *Full example: [bank_account_9.cpp](src/examples/executor/bank_account_9.cpp)*

The `bank_account` class can then be constructed using an explicitly-specified thread pool:

    std::experimental::thread_pool pool;
    auto ex = std::experimental::make_executor(pool);
    bank_account acct(ex);

or any other object that meets the executor type requirements.

### Coordinating parallel operations

To illustrate the tools that this library provides for managing parallelism and concurrency, let us now turn our attention to a different use case: sorting large datasets. Consider an example where we want to sort a very large vector of doubles:

    std::vector<double> vec(a_very_large_number);
    ...
    std::sort(vec.begin(), vec.end());

If we are running this code on a system with two at least CPUs then we can cut the running time by splitting the array into halves, sorting each half in parallel, and finally merging the two now-sorted halves into a sorted whole. The executors library lets us do this easily using the `copost()` function:

    std::experimental::copost(
      [&]{ std::sort(vec.begin(), vec.begin() + (vec.size() / 2)); },
      [&]{ std::sort(vec.begin() + (vec.size() / 2), vec.end()); },
      std::experimental::use_future).get();

    std::inplace_merge(vec.begin(), vec.begin() + (vec.size() / 2), vec.end());

> *Full example: [sort_1.cpp](src/examples/executor/sort_1.cpp)*

The function name `copost()` is short for <b>co</b>ncurrent **post**. In the above example, it posts the two lambda objects:

    [&]{ std::sort(vec.begin(), vec.begin() + (vec.size() / 2)); },
    [&]{ std::sort(vec.begin() + (vec.size() / 2), vec.end()); },

to the system thread pool, where they can run in parallel. When both have finished, the caller is notified via the final completion token (in this case, `use_future`).

Like `post()`, `copost()` (and its counterpart `codispatch()`) is a variadic template function that can accept a number of completion tokens. The library defines `copost()` as:

    auto copost(t0, t1, ..., tN-1, tN);
    auto copost(executor, t0, t1, ..., tN-1, tN);

where *t0* to *tN* are completion tokens. When we call `copost()`, the library turns each of the tokens into the function objects *f0* to *fN*. The functions *f0* to *fN-1* are then posted for parallel execution, and only when all are complete will *fN* be invoked. The return values of *f0* to *fN-1* are passed as arguments to *fN* as in the following example:

    std::experimental::copost(ex_,
      []{ return 1; },
      []{ return "hello"s; },
      []{ return 123.0; },
      [](int a, std::string b, double c) { ... });

### Composition using chain

To facilitate reusability, we will now wrap our parallel sort implementation in a function and let the user choose how to wait for completion. However, once the two halves have been sorted we now need to perform two separate actions: merge the halves, *and* deliver the completion notification according to the user-supplied token. We can combine these two actions into a single operation using `chain()`:

    template <class Iterator, class CompletionToken>
    auto parallel_sort(Iterator begin, Iterator end, CompletionToken&& token)
    {
      const std::size_t n = end - begin;
      return std::experimental::copost(
        [=]{ std::sort(begin, begin + (n / 2)); },
        [=]{ std::sort(begin + (n / 2), end); },
        std::experimental::chain(
          [=]{ std::inplace_merge(begin, begin + (n / 2), end); },
          std::forward<CompletionToken>(token)));
    }

> *Full example: [sort_2.cpp](src/examples/executor/sort_2.cpp)*

Here, the call to `chain()`:

    std::experimental::chain(

takes the two completion tokens:

      [=]{ std::inplace_merge(begin, begin + (n / 2), end); },
      std::forward<CompletionToken>(token)));

turns them into their corresponding function objects, and then combines these into a single function object. This single function object meets the requirements for a final argument to `copost()`.

The `chain()` function is a variadic function similar to `post()` and `dispatch()`:

    auto chain(t0, t1, ..., tN);
    template <class Signature> auto chain(t0, t1, ..., tN);

where *t0* to *tN* are completion tokens. When we call `chain()`, the library turns *t0* to *tN* into the corresponding function objects *f0* to *fN*. These are chained such that they execute serially, and the return value of any given function is passed as an argument to the next one. However, unlike `post()` and `dispatch()`, the chained functions are not run immediately, but are instead returned as a function object which we can save for later invocation.

The function objects generated by `chain()` are known as **continuations**. They differ from normal function objects in two ways: they always return `void`, and they may only be called using an rvalue reference. For example, the continuation created by:

    auto c = std::experimental::chain(
      []{ return 1; },
      [](int i) { return i + 1; },
      [](int i) { return i * 2; },
      [](int i) { return std::to_string(i); },
      [](std::string s) { std::cout << "value is " << s << "\n"; });

must be called like this:

    std::move(c)();

It is only safe to invoke the continuation once, as the call may have resulted in `c` being in a moved-from state.

A more typical way to invoke the continuation would be to pass it to an operation like `post()` or `dispatch()`:

    std::experimental::dispatch(std::move(c));

Unlike a direct call, this has the advantage of preserving completion token behaviour with respect to the deduced return type. This means that the following works as expected:

    auto c = std::experimental::chain(
      []{ return 1; },
      [](int i) { return i + 1; },
      [](int i) { return i * 2; },
      [](int i) { return std::to_string(i); },
      [](std::string s) { return "value is " + s; },
      std::experimental::use_future);
    std::future<std::string> fut = std::experimental::dispatch(c);
    std::cout << fut.get() << std::endl;

and outputs the string `value is 4`.

### Predictable return type deduction

For small arrays there is little benefit in parallelisation, as the CPU cost of coordinating parallel operations can outweigh any savings in elapsed time. We will now alter our parallel sort implementation to add a heuristic to detect small arrays and sort them serially:

    template <class Iterator, class CompletionToken>
    auto parallel_sort(Iterator begin, Iterator end, CompletionToken&& token)
    {
      const std::size_t n = end - begin;
      if (n <= 32768)
      {
        return std::experimental::dispatch(
          [=]{ std::sort(begin, end); },
          std::forward<CompletionToken>(token));
      }
      else
      {
        return std::experimental::copost(
          [=]{ std::sort(begin, begin + (n / 2)); },
          [=]{ std::sort(begin + (n / 2), end); },
          std::experimental::chain(
            [=]{ std::inplace_merge(begin, begin + (n / 2), end); },
            std::forward<CompletionToken>(token)));
      }
    }

> *Full example: [sort_3.cpp](src/examples/executor/sort_3.cpp)*

As our `parallel_sort()` function has an automatically deduced return type, we must ensure that all return statements use the same type. At first glance, it appears that we have two quite different return statements, with the `dispatch()` performing the serial sort:

    return std::experimental::dispatch(
      [=]{ std::sort(begin, end); },
      std::forward<CompletionToken>(token));

and the `copost()` performing the parallel sort:

    return std::experimental::copost(
      [=]{ std::sort(begin, begin + (n / 2)); },
      [=]{ std::sort(begin + (n / 2), end); },
      std::experimental::chain(
        [=]{ std::inplace_merge(begin, begin + (n / 2), end); },
        std::forward<CompletionToken>(token)));

The `dispatch()` and `copost()` return types are each deduced in two steps using the `handler_type<>` and `async_result<>` type traits:

1. The handler type `Handler` is determined by `handler_type<CompletionToken, Signature>::type`.
2. The return type is determined by `async_result<Handler>::type`.

You will recall that with both `dispatch()` and `chain()`, the function objects are called serially and the result of a function is passed as the argument to the next. If we consider the position of our completion tokens:

    std::forward<CompletionToken>(token)));

then for the `dispatch()` call, the preceding function is the lambda:

    [=]{ std::sort(begin, end); },

and for `chain()`, the preceding function is:

    [=]{ std::inplace_merge(begin, begin + (n / 2), end); },

Both of these have a `void` return type. Therefore, the `Signature` applied to the completion token is the same in both cases, namely `void()`. Consequently, both calls have the same return type.

### Polymorphic continuations

Now that we have a heuristic for deciding when an array is too small to warrant a parallel sort, we can use this test to make a complementary improvement to our algorithm: if, after dividing our array into halves, the halves themselves are above the threshold, they can be further divided and sorted in parallel. That is, we can make our `parallel_sort()` function recursive.

By default, operations like `dispatch()` and `copost()` preserve the type information of the tokens passed to them. This gives the compiler maximum opportunity to optimise the chain. However, recursion implies that we will need some form of type erasure. This is achieved using the polymorphic wrapper `continuation<>`:

    template <class Iterator, class CompletionToken>
    auto parallel_sort(Iterator begin, Iterator end, CompletionToken&& token)
    {
      const std::size_t n = end - begin;
      if (n <= 32768)
      {
        return dispatch(
          [=]{ std::sort(begin, end); },
          std::forward<CompletionToken>(token));
      }
      else
      {
        return copost(
          [=](continuation<> c)
          {
            return parallel_sort(begin, begin + (n / 2), std::move(c));
          },
          [=](continuation<> c)
          {
            return parallel_sort(begin + (n / 2), end, std::move(c));
          },
          chain(
            [=]{ std::inplace_merge(begin, begin + (n / 2), end); },
            std::forward<CompletionToken>(token)));
      }
    }

> *Full example: [sort_4.cpp](src/examples/executor/sort_4.cpp)*

When the library is passed a function with a last argument of type `continuation<>`:

    [=](continuation<> c)

it captures the chain of function objects that comes after it (that is, the continuation of the current function), and passes them in the `continuation<>` object. We must then pass this continuation object on to other operations, such as a recursive call to `parallel_sort()`:

    {
      return parallel_sort(begin, begin + (n / 2), std::move(c));
    },

In this example we let the library work out the correct signature of the continuation. The return type of the recursive call to `parallel_sort()` contains the type information needed by the library to make this deduction. If, on the other hand, we prefer to be explicit then we can specify the signature as a template parameter to `continuation<>`:

    [=](continuation<void()> c)
    {
      parallel_sort(begin, begin + (n / 2), std::move(c));
    },

These polymorphic continuations can also be used to capture and store other function objects:

    continuation<void()> c = std::experimental::chain(
      []{ return 1; },
      [](int i) { return i + 1; },
      [](int i) { return i * 2; },
      [](int i) { return std::to_string(i); },
      [](std::string s) { std::cout << "value is " << s << "\n"; });

The polymorphic wrapper has the same behaviour as the non-type-erased objects returned by `chain()`. That is, it has a `void` return type and must be called using an rvalue reference:

    std::move(c)();

or, as with `chain()`, more often passed to some other operation:

    std::experimental::post(std::move(c));

Timers
------

### Convenience functions for timer operations

When working with executors, we will often need to schedule functions to run at, or after, a time. This library provides several high-level operations we can use for this purpose.

First of these is the `post_after()` function, which we can use to schedule a function to run after a delay:

    std::experimental::post_after(std::chrono::seconds(1),
      []{ std::cout << "Hello, world!\n"; });

If we want to be notified when the function has finished, we can specify a completion token as well:

    std::future<void> fut = std::experimental::post_after(std::chrono::seconds(1),
      []{ std::cout << "Hello, world!\n"; }, std::experimental::use_future);
    fut.get();

Both of the above examples use the system executor. We can of course specify an executor of our own:

    std::experimental::thread_pool pool;
    std::experimental::post_after(pool, std::chrono::seconds(1),
      []{ std::cout << "Hello, world!\n"; });

The `post_at()` function can instead be used to run a function object at an absolute time:

    auto start_time = std::chrono::steady_clock::now();
    // ...
    std::experimental::post_after(start_time + std::chrono::seconds(1),
      []{ std::cout << "Hello, world!\n"; });

The library also provides `dispatch_after()` and `dispatch_at()` as counterparts to `post_after()` and `post_at()` respectively. As dispatch operations, they are permitted to run the function object before returning, according to the rules of the underlying executor.

### Timer operations in resumable functions

These high-level convenience functions can easily be used in resumable functions to provide the resumable equivalent of `std::thread_thread::sleep()`:

    dispatch(
      [](std::experimental::yield_context yield)
      {
        auto start_time = std::chrono::steady_clock::now();
        for (int i = 0; i < 10; ++i)
        {
          std::experimental::dispatch_at(start_time + std::chrono::seconds(i + 1), yield);
          std::cout << i << std::endl;
        }
      });

> *Full example: [dispatch_at_1.cpp](src/examples/timer/dispatch_at_1.cpp)*

Here, the `yield` object is passed as a completion token to `dispatch_at()`. The resumable function is automatically suspended and resumes once the absolute time is reached.

### Timer objects and cancellation of timer operations

The convenience functions do not provide a way to cancel a timer operation. For this level of control we want to use one of the timer classes provided by the library: `steady_timer`, `system_timer` or `high_resolution_timer`.

Timer objects can work with any execution context. When constructing a timer object, we can choose whether it uses the system context:

    std::experimental::steady_timer timer;

or a specific context, such as a thread pool:

    std::experimental::thread_pool pool;
    std::experimental::steady_timer timer(pool);

In the latter case, the timer cannot be used once its execution context ceases to exist.

A timer object has an associated expiry time, which can be set using either a relative value:

    timer.expires_after(std::chrono::seconds(60));

or an absolute one:

    timer.expires_at(start_time + std::chrono::seconds(60));

Once the expiry time is set, we then wait for the timer to expire:

    timer.wait([](std::error_code ec){ std::cout << "Hello, world!\n"; });

Finally, if we want to cancel the wait, we simply use the `cancel()` member function:

    timer.cancel();

If the cancellation was successful, the function object is called with a `error_code` equivalent to the condition `std::errc::operation_canceled`.

Channels
--------

Channels provide a lightweight mechanism for chains of asynchronous operations (and especially resumable functions) to communicate and synchronise their execution. Here is a simple example that will print "ping" once a second until stopped:

    void pinger(
      std::shared_ptr<std::experimental::channel<std::string>> c,
      std::experimental::yield_context yield)
    {
      for (;;)
      {
        c->put("ping", yield);
      }
    }

    void printer(
      std::shared_ptr<std::experimental::channel<std::string>> c,
      std::experimental::yield_context yield)
    {
      for (;;)
      {
        std::string msg = c->get(yield);
        std::cout << msg << std::endl;
        std::experimental::dispatch_after(std::chrono::seconds(1), yield);
      }
    }

    int main()
    {
      auto c = std::make_shared<std::experimental::channel<std::string>>();
      std::experimental::strand<std::experimental::system_executor> ex;

      std::experimental::dispatch(
        std::experimental::wrap(ex,
          [c](std::experimental::yield_context yield){ pinger(c, yield); }));

      std::experimental::dispatch(
        std::experimental::wrap(ex,
          [c](std::experimental::yield_context yield){ printer(c, yield); }));

      std::string input;
      std::getline(std::cin, input);
    }

> *Full example: [ping.cpp](src/examples/channel/ping.cpp)*

When `pinger` attempts to `put` a string into the channel it will wait until `printer` is ready to `get` it.
