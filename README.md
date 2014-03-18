A C++14 library for executors
=============================

This is a potential standard library proposal that covers:

* Executors and schedulers
* A replacement for `std::async()`
* Resumable functions / coroutines
* A model for asynchronous operations

Basic concepts
--------------

To use this library, there are some key design concepts that you should know...

**Function objects** are the parcels of work that are executed by the library:

    auto f1 = []{ cout << "Hello, world!\n"; };

An **execution context** represents a place where function objects will be executed. A thread pool is an example of an execution context.

    thread_pool tp;

An **executor** is the interface used to schedule function objects for execution. You can create an executor for a given execution context:

    auto ex1 = make_executor(tp);

or adapt an existing executor into another:

    auto ex2 = make_strand(ex1);
    
Ultimately, an executor represents a set of rules about when it is okay to invoke a function object. While `ex1` can use any thread in the pool, `ex2` ensures that none of its function objects run concurrently.

The executor interface is defined by a collection of type requirements, but programs that want runtime selection of executors can use the library's polymorphic wrapper `executor`.

    executor ex3 = ex2;
    
Functions with the name **post** are used to schedule a function object for later execution.

    ex1.post(f1);
    
For performance, you may want to allow nested execution of the function. Functions named **dispatch** are used for this.

    ex1.dispatch(f1);

Whether or not the function is executed immediately depends on the executor's rules. If the executor decides it is not okay to execute `f1` immediately, it behaves the same as `post()` and schedules the function object for later execution.

If you want to wait for a function object to finish, use a **completion token** to tell the library how you want to be notified. A function object can be used as a completion token.

    auto f2 = []{ return "Hello, world!\n"s; };
    dispatch(f2, [](string s){ cout << s; });

The library also provides other types of completion token. One example is `use_future`, which causes the function result to be returned in a `future<>` object.

    future<string> fut = dispatch(f2, use_future);
    cout << fut.get();

