#include "tasksys.h"
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include <unordered_set>
#include <queue>
#include <map>


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

TaskSystemParallelThreadPoolSleeping::
    TaskSystemParallelThreadPoolSleeping
    (int num_of_threads): ITaskSystem(num_of_threads)
                         , threads(num_of_threads)
                         , num_threads(num_of_threads)
                         , tasksDoneMutex(new std::mutex())
                         , threads_sleeping_mutex(new std::mutex())
                         , threads_sleeping_cv(new std::condition_variable())
{
    task_id = 0;
    task_id_mutex = new std::mutex();
    waitingForDeps_cv = new std::condition_variable();
    waitingForDepsQueueMutex = new std::mutex();
    runningQueueMutex = new std::mutex();
    runningQueue = std::queue<workingTask>();
    waitingForDepsQueue = std::unordered_set<waitingTask>();
    finishedTasks = std::map<TaskID, bool>();
    tasksDone = std::map<TaskID, int>();
    finished = false;
    for (int i = 0; i < num_threads; i++) {
      threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::threadRun, this, i);
    }

}

void TaskSystemParallelThreadPoolSleeping::threadRun(int thread_id) {
  while (true) {
    if (thread_id == 0) {
      std::unique_lock<std::mutex> lock(*waitingForDepsQueueMutex);
      if (waitingForDepsQueue.empty()) {
        waitingForDeps_cv->wait(lock);
      } else {
        for (auto& it: waitingForDepsQueue) {
          bool allDepsFinished = true;
          for (auto& dep: it.deps) {
            if (!finishedTasks[dep]) {
              allDepsFinished = false;
              break;
            }
          }
          if (allDepsFinished) {
            workingTask task;
            task.task_id = it.task_id;
            task.num_total_tasks = it.num_total_tasks;
            task.runnable = it.runnable;
            task.tasks_left = it.num_total_tasks;
            task.task_now = 0;
            std::unique_lock<std::mutex> lock(*runningQueueMutex);
            runningQueue.push(task);

            /// Notify the threads to start working
            threads_sleeping_cv->notify_all();///// TODO: check again if this is correct
            runningQueueMutex->unlock();
            waitingForDepsQueue.erase(it);
            break;
          }
        }
      }

    } else {

    if (finished) {
      return;
    }

    workingTask taskToRun;
    taskToRun.task_now = -1;

    {
      std::lock_guard<std::mutex> lock(*runningQueueMutex);
      if (!runningQueue.empty()) {
        if (runningQueue.front().task_now < runningQueue.front().num_total_tasks) {
          taskToRun = runningQueue.front();
          runningQueue.front().task_now++;
        } else {
          runningQueue.pop();
          if (runningQueue.empty()) {
            taskToRun.task_now = -10;
          }
        }
      } else {
        taskToRun.task_now = -10;
      }
    }
    
    if (taskToRun.task_now >= 0 && taskToRun.task_now < taskToRun.num_total_tasks) {
      taskToRun.runnable->runTask(taskToRun.task_now, taskToRun.num_total_tasks);

      {
        std::lock_guard<std::mutex> lock(*tasksDoneMutex);
        tasksDone[taskToRun.task_id]++;
        if (tasksDone[taskToRun.task_id] == taskToRun.num_total_tasks) {
          waitingForDepsQueueMutex->lock();
          waitingForDepsQueueMutex->unlock();
          waitingForDeps_cv->notify_all();
        }
      }
    } else if (taskToRun.task_now == -10) {
      // TODO: maybe lock should be on threads_sleeping_mutex
      std::unique_lock<std::mutex> lock(*runningQueueMutex);
      while (runningQueue.empty() && !finished) {
        threads_sleeping_cv->wait(lock);
      }
    }

    }
  }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {

{
  std::lock_guard<std::mutex> lock(*runningQueueMutex);
  finished = true;
  threads_sleeping_cv->notify_all();
}
  for (int i = 0; i < num_threads; i++) {
    TaskSystemParallelThreadPoolSleeping::threads[i].join();
  }
  delete task_id_mutex;
  delete waitingForDeps_cv;
  delete waitingForDepsQueueMutex;
  delete runningQueueMutex;
  delete tasksDoneMutex;
  delete threads_sleeping_cv;
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


  //
  // TODO: CS149 students will modify the implementation of this
  // method in Parts A and B.  The implementation provided below runs all
  // tasks sequentially on the calling thread.
  //

  /// dynamic + sleep when no work

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

  // std::unique_lock<std::mutex> lk(*runMutex);
  // mutex_->lock();
  // taskCount = 0;
  // this->runnable = runnable;
  // tasks_left = num_total_tasks;
  // total_tasks = num_total_tasks;
  // tasksDone = 0;
  // threads_sleeping_cv->notify_all();
  // mutex_->unlock();
  // parent_cv->wait(lk);
  // lk.unlock();
}


TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    /// Put the task in the waiting queue
    waitingTask task;
    task_id_mutex->lock();
    task.task_id = task_id++;
    task_id_mutex->unlock();
    task.num_total_tasks = num_total_tasks;
    task.runnable = runnable;
    task.deps = deps;

    std::unique_lock<std::mutex> lock(*waitingForDepsQueueMutex);
    waitingForDepsQueue.insert(task);
    waitingForDeps_cv->notify_all();
    lock.unlock();

    return task.task_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
