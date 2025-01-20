
# A Multi-Core Task Execution Library 

## Project Overview

This project implements a C++ task execution library optimized for multi-core CPUs. It introduces multiple approaches for running data-parallel tasks (Part A) and more advanced task graphs with dependencies (Part B).

## Key Contributions

- **Serial Implementation**  
  • Introduced a baseline serial engine
  • Ensured correctness and provided a reference for future optimizations

- **Parallel Spawn (Part A - Step 1)**  
  • Spawned worker threads on each `run()` call
  • Demonstrated multi-core CPU usage for data-parallel tasks

- **Thread Pool - Spinning (Part A - Step 2)**  
  • Created a persistent thread pool
  • Used spinning to wait for tasks, reducing frequent thread creation overhead

- **Thread Pool - Sleeping (Part A - Step 3)**  
  • Leveraged condition variables to reduce CPU usage
  • Achieved synchronous task completion while sleeping idle threads

- **Async Task Graph Support (Part B)**  
  • Supported dependent task launches with asynchronous execution 
  • Implemented `runAsyncWithDeps()` and `sync()`, enabling DAG-based scheduling

## Project Highlights

- Used modern C++ threads, mutexes, and condition variables for concurrency control.  
- Optimized scheduling strategies for varying workloads and performance needs. 

## Usage & Testing

1. Compile using the provided `Makefile`.  
2. Run built-in tests with:  
   ```bash
   ./runtasks -n 4 simple_test_sync
   ```  
3. Examine added tests in main.cpp

By structuring and documenting each phase of development, these contributions showcase an end-to-end understanding of parallel programming, synchronization, and performance optimization.

## PROGRESS ✓


### Part A: Synchronous Bulk Task Launch
- [x] TaskSystemParallelSpawn implementation
    - [x] Create worker threads in run()
    - [x] Join threads before returning
    - [x] Test correctness with simple_test_sync
    
- [x] TaskSystemParallelThreadPoolSpinning
    - [x] Design thread pool architecture 
    - [x] Implement thread spinning logic
    - [x] Verify synchronous behavior
    
- [x] TaskSystemParallelThreadPoolSleeping  
    - [x] Add condition variables
    - [x] Implement sleep/wake functionality
    - [x] Optimize thread utilization

### Part B: Task Graph Support
- [x] Extend TaskSystemParallelThreadPoolSleeping
    - [x] Implement runAsyncWithDeps()
    - [x] Implement sync() 
    - [x] Add dependency tracking

### Running Tests [detailed]

The starter code contains a suite of test applications that use your task system. For a description of the test harness tests, see `tests/README.md`, and for the test definitions themselves, see `tests/tests.h`. To run a test, use the `runtasks` script. For example, to run the test called `mandelbrot_chunked`, which computes an image of a Mandelbrot fractal using a bulk launch of tasks that each process a continuous chunk of the image, type:

```bash
./runtasks -n 16 mandelbrot_chunked
```


The different tests have different performance characteristics -- some do little work per task, others perform significant amounts of processing.  Some tests create large numbers of tasks per launch, others very few.  Sometimes the tasks in a launch all have similar compute cost.  In others, the cost of tasks in a single bulk launch is variable. We have described most of the tests in `tests/README.md`, but we encourage you to inspect the code in `tests/tests.h` to understand the behavior of all tests in more detail.

One test that may be helpful to debug correctness while implementing your solution is `simple_test_sync`, which is a very small test that should not be used to measure performance but is small enough to be debuggable with print statements or debugger. See function `simpleTest` in `tests/tests.h`.

We encourage you to create your own tests. Take a look at the existing tests in `tests/tests.h` for inspiration. We have also included a skeleton test composed of `class YourTask` and function `yourTest()` for you to build on if you so choose. For the tests you do create, make sure to add them to the list of tests and test names in `tests/main.cpp`, and adjust the variable `n_tests` accordingly. Please note that while you will be able to run your own tests with your solution, you will not be able to compile the reference solution to run your tests.

The `-n` command-line option specifies the maximum number of threads the task system implementation can use.  In the example above, we chose `-n 16` because the CPU in the AWS instance features sixteen execution contexts.  The full list of tests available to run is available via command line help  (`-h` command line option).

The `-i` command-line options specifies the number of times to run the tests during performance measurement. To get an accurate measure of performance, `./runtasks` runs the test multiple times and records the _minimum_ runtime of several runs; In general, the default value is sufficient---Larger values might yield more accurate measurements, at the cost of greater test runtime.

In addition, we also provide you the test harness that we will use for grading performance:

> [!IMPORTANT] To test with async tests, modify the list of tests in `tests/run_test_harness.py` to include the async tests.

```bash
>>> python3 ../tests/run_test_harness.py
```

The harness has the following command line arguments,

```bash
>>> python3 run_test_harness.py -h
usage: run_test_harness.py [-h] [-n NUM_THREADS]
                           [-t TEST_NAMES [TEST_NAMES ...]] [-a]

Run task system performance tests

optional arguments:
  -h, --help            show this help message and exit
  -n NUM_THREADS, --num_threads NUM_THREADS
                        Max number of threads that the task system can use. (16
                        by default)
  -t TEST_NAMES [TEST_NAMES ...], --test_names TEST_NAMES [TEST_NAMES ...]
                        List of tests to run
  -a, --run_async       Run async tests
```

It produces a detailed performance report that looks like this:

```bash
>>> python3 ../tests/run_test_harness.py -t super_light super_super_light
python3 ../tests/run_test_harness.py -t super_light super_super_light
================================================================================
Running task system grading harness... (2 total tests)
  - Detected CPU with 16 execution contexts
  - Task system configured to use at most 16 threads
================================================================================
================================================================================
Executing test: super_super_light...
Reference binary: ./runtasks_ref_linux
Results for: super_super_light
                                        STUDENT   REFERENCE   PERF?
[Serial]                                9.053     9.022       1.00  (OK)
[Parallel + Always Spawn]               8.982     33.953      0.26  (OK)
[Parallel + Thread Pool + Spin]         8.942     12.095      0.74  (OK)
[Parallel + Thread Pool + Sleep]        8.97      8.849       1.01  (OK)
================================================================================
Executing test: super_light...
Reference binary: ./runtasks_ref_linux
Results for: super_light
                                        STUDENT   REFERENCE   PERF?
[Serial]                                68.525    68.03       1.01  (OK)
[Parallel + Always Spawn]               68.178    40.677      1.68  (NOT OK)
[Parallel + Thread Pool + Spin]         67.676    25.244      2.68  (NOT OK)
[Parallel + Thread Pool + Sleep]        68.464    20.588      3.33  (NOT OK)
================================================================================
Overall performance results
[Serial]                                : All passed Perf
[Parallel + Always Spawn]               : Perf did not pass all tests
[Parallel + Thread Pool + Spin]         : Perf did not pass all tests
[Parallel + Thread Pool + Sleep]        : Perf did not pass all tests
```

In the above output `PERF` is the ratio of your implementation's runtime to the reference solution's runtime. So values less than one indicate that your task system implementation is faster than the reference implementation.

> [!TIP]
> Mac users: While we provided reference solution binaries for both part a and part b, we will be testing your code using the linux binaries. Therefore, we recommend you check your implementation in the AWS instance before submitting. If you are using a newer Mac with an M1 chip, use the `runtasks_ref_osx_arm` binary when testing locally. Otherwise, use the `runtasks_ref_osx_x86` binary.

