A C++14 library for executors
=============================

A two minute introduction to the library
----------------------------------------

Run a function asynchronously:

    post([]{
        // ...
      });

Run a function asynchronously, on your own thread pool:

    thread_pool pool;

    post(pool, []{
        // ...
      });

    pool.join();

Run a function asynchronously and wait for the result:

    std::future<int> f =
      post(use_future([]{
        // ...
        return 42;
      }));

    std::cout << f.get() << std::endl;

Run a function asynchronously, on your own thread pool, and wait for the result:

    thread_pool pool;

    std::future<int> f =
      post(pool, use_future([]{
        // ...
        return 42;
      }));

    std::cout << f.get() << std::endl;

[endsect]
