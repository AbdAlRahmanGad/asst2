#include "tasksys.h"
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char *TaskSystemSerial::name() { return "Serial"; }

TaskSystemSerial::TaskSystemSerial(int num_threads)
    : ITaskSystem(num_threads) {}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable *runnable, int num_total_tasks) {
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable *runnable,
                                          int num_total_tasks,
                                          const std::vector<TaskID> &deps) {
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }

  return 0;
}

void TaskSystemSerial::sync() { return; }

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelSpawn::name() {
  return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads)
    : ITaskSystem(num_threads) {
  // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn
  // in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable *runnable, int num_total_tasks) {
  // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn
  // in Part B.
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(
    IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps) {
  // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn
  // in Part B.
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }

  return 0;
}

void TaskSystemParallelSpawn::sync() {
  // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn
  // in Part B.
  return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelThreadPoolSpinning::name() {
  return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(
    int num_threads)
    : ITaskSystem(num_threads) {
  // NOTE: CS149 students are not expected to implement
  // TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable *runnable,
                                               int num_total_tasks) {
  // NOTE: CS149 students are not expected to implement
  // TaskSystemParallelThreadPoolSpinning in Part B.
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(
    IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps) {
  // NOTE: CS149 students are not expected to implement
  // TaskSystemParallelThreadPoolSpinning in Part B.
  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }

  return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
  // NOTE: CS149 students are not expected to implement
  // TaskSystemParallelThreadPoolSpinning in Part B.
  return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelThreadPoolSleeping::name() {
  return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(
    int num_of_threads)
    : ITaskSystem(num_of_threads), threads(num_of_threads),
      num_threads(num_of_threads), tasksDoneMutex(new std::mutex()),
      threads_sleeping_mutex(new std::mutex()),
      threads_sleeping_cv(new std::condition_variable()) {

  task_id = 0;
  task_id_mutex = new std::mutex();
  waitingForDeps_cv = new std::condition_variable();
  sync_cv = new std::condition_variable();
  syncing = false;
  waitingForDepsQueueMutex = new std::mutex();
  runningQueueMutex = new std::mutex();
  runningQueue = std::queue<workingTask>();
  waitingForDepsQueue = std::unordered_set<waitingTask>();
  finishedTasks = std::map<TaskID, bool>();
  tasksDone = std::map<TaskID, int>();
  finished = false;
  for (int i = 0; i < num_threads; i++) {
    threads[i] =
        std::thread(&TaskSystemParallelThreadPoolSleeping::threadRun, this, i);
  }
}

void TaskSystemParallelThreadPoolSleeping::threadRun(int thread_id) {
  while (true) {
    if (thread_id == 0) {
      std::unique_lock<std::mutex> lock(*waitingForDepsQueueMutex);
      if (syncing) {
        waitingForDeps_cv->notify_all();
        std::unique_lock<std::mutex> lock(*runningQueueMutex);
        if (runningQueue.empty() && waitingForDepsQueue.empty()) {
          sync_cv->notify_all();
        }
        lock.unlock();
      }
      if (waitingForDepsQueue.empty() && !finished) {
        waitingForDeps_cv->wait(lock);
      } else if (finished) {
        return;
      } else {
        for (auto &it : waitingForDepsQueue) {
          bool allDepsFinished = true;
          for (auto &dep : it.deps) {
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
            threads_sleeping_cv->notify_all();
            lock.unlock();
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
          if (runningQueue.front().task_now <
              runningQueue.front().num_total_tasks) {
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

      if (taskToRun.task_now >= 0 &&
          taskToRun.task_now < taskToRun.num_total_tasks) {
        taskToRun.runnable->runTask(taskToRun.task_now,
                                    taskToRun.num_total_tasks);

        {
          std::unique_lock<std::mutex> lock(*tasksDoneMutex);
          tasksDone[taskToRun.task_id]++;
          if (tasksDone[taskToRun.task_id] == taskToRun.num_total_tasks) {
            lock.unlock();
            waitingForDepsQueueMutex->lock();
            lock.lock();
            finishedTasks[taskToRun.task_id] = true;
            lock.unlock();
            waitingForDeps_cv->notify_all();
            waitingForDepsQueueMutex->unlock();
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
    std::lock_guard<std::mutex> lock2(*waitingForDepsQueueMutex);
    std::lock_guard<std::mutex> lock(*runningQueueMutex);

    finished = true;
    threads_sleeping_cv->notify_all();
    waitingForDeps_cv->notify_all();
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

void TaskSystemParallelThreadPoolSleeping::run(IRunnable *runnable,
                                               int num_total_tasks) {

  for (int i = 0; i < num_total_tasks; i++) {
    runnable->runTask(i, num_total_tasks);
  }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(
    IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps) {

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

  std::unique_lock<std::mutex> lock(*waitingForDepsQueueMutex);
  waitingForDeps_cv->notify_all();
  syncing = true;
  sync_cv->wait(lock);
  syncing = false;
  lock.unlock();
  return;
}
