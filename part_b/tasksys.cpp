#include "tasksys.h"
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <iostream>
#include <condition_variable>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping
                                      (int num_of_threads): ITaskSystem(num_of_threads)
                                                             , threads(num_of_threads)
                                                             , num_threads(num_of_threads)
                                                             , mutex_(new std::mutex())
                                                             , runMutex(new std::mutex())
                                                             , threads_sleeping_mutex(new std::mutex())
                                                             , parent_cv(new std::condition_variable())
                                                             , threads_sleeping_cv(new std::condition_variable()) {
    total_tasks = -1;
    tasks_left = 0;
    taskCount = -1;
    tasksDone = 0;
    finished = false;
    for (int i = 0; i < num_threads; i++) {
      threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::threadRun, this);
    }

}

void TaskSystemParallelThreadPoolSleeping::threadRun() {
  while (true) {

    if (finished) {
      return;
    }

    int task = -1;
    {
      std::lock_guard<std::mutex> lock(*mutex_);
      if (taskCount < total_tasks) {
        task = taskCount++;
      }
    }

    if (task >= 0 && task < total_tasks) {
      runnable->runTask(task, total_tasks);

      {
        std::lock_guard<std::mutex> lock(*mutex_);
        tasksDone++;
        tasks_left--;
        if (tasksDone == total_tasks) {
          runMutex->lock();
          runMutex->unlock();
          parent_cv->notify_all();
        }
      }
    } else {
      std::unique_lock<std::mutex> lock(*threads_sleeping_mutex);
      while (tasks_left == 0 && !finished) {
        threads_sleeping_cv->wait(lock);
      }
    }
  }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {

  finished = true;
  threads_sleeping_cv->notify_all();

  for (int i = 0; i < num_threads; i++) {
    TaskSystemParallelThreadPoolSleeping::threads[i].join();
  }
  delete mutex_;
  delete runMutex;
  delete parent_cv;
  delete threads_sleeping_cv;
  runnable = nullptr;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


  //
  // TODO: CS149 students will modify the implementation of this
  // method in Parts A and B.  The implementation provided below runs all
  // tasks sequentially on the calling thread.
  //

  /// dynamic + sleep when no work

  std::unique_lock<std::mutex> lk(*runMutex);
  mutex_->lock();
  taskCount = 0;
  this->runnable = runnable;
  tasks_left = num_total_tasks;
  total_tasks = num_total_tasks;
  tasksDone = 0;
  threads_sleeping_cv->notify_all();
  mutex_->unlock();
  parent_cv->wait(lk);
  lk.unlock();
}


TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
