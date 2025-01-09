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
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
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

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_of_threads): ITaskSystem(num_of_threads)
                                                                       , threads(num_of_threads)
                                                                       , num_threads(num_of_threads)
                                                                       , mutex_(new std::mutex()) {
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    delete mutex_;
}

void TaskSystemParallelSpawn::threadRun(IRunnable* runnable, int* task_num, int num_total_tasks) {
  int task = -1;
  while (task < num_total_tasks) {
    mutex_->lock();
    task = (*task_num)++;
    mutex_->unlock();

    if (task >= num_total_tasks) break;

    runnable->runTask(task, num_total_tasks);
  }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    /// dynamic assignment of threads because workloads are not known
    int *task_num = new int;
    *task_num = 0;
    for (int i = 0; i < num_threads; i++) {
        threads[i] = std::thread(&TaskSystemParallelSpawn::threadRun, this, runnable, task_num, num_total_tasks);
    }

    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
    }

    delete task_num;
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
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



TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning
                                                (int num_of_threads): ITaskSystem(num_of_threads)
                                                                       , threads(num_of_threads)
                                                                       , num_threads(num_of_threads)
                                                                       , mutex_(new std::mutex())
                                                                       , runMutex(new std::mutex())
                                                                       , cv(new std::condition_variable()) {
  total_tasks = -1;
  taskCount = -1;
  tasksDone = 0;
  finished = false;
  for (int i = 0; i < num_threads; i++) {
        threads[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::threadRun, this);
  }

}


void TaskSystemParallelThreadPoolSpinning::threadRun() {
    while (true) {
        // Check for shutdown signal
        {
            std::lock_guard<std::mutex> lock(*mutex_);
            if (finished) {
                return;
            }
        }

        int task = -1;
        // Try to get a task
        {
            std::lock_guard<std::mutex> lock(*mutex_);
            if (taskCount < total_tasks) {
                task = taskCount++;
            }
        }

        // Process task if valid
        if (task >= 0 && task < total_tasks) {
            runnable->runTask(task, total_tasks);

            // Update completion status
            {
                std::lock_guard<std::mutex> lock(*mutex_);
                tasksDone++;
                if (tasksDone == total_tasks) {
                    runMutex->lock();
                    runMutex->unlock();
                    cv->notify_one();  // Signal completion
                }
            }
        }
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    finished = true;
    for (int i = 0; i < num_threads; i++) {
      TaskSystemParallelThreadPoolSpinning::threads[i].join();
    }
    delete mutex_;
    delete runMutex;
    runnable = nullptr;
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    /// static or dynamic assignment of threads?
    /// dynamic assignment of threads because workloads are not known

    /// dynamic + busy waiting

    std::unique_lock<std::mutex> lk(*runMutex);
    mutex_->lock();
    taskCount = 0;
    this->runnable = runnable;
    total_tasks = num_total_tasks;
    tasksDone = 0;
    mutex_->unlock();
    cv->wait(lk);
    lk.unlock();
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
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

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_of_threads): ITaskSystem(num_of_threads)
                                                                                              , threads(num_of_threads)
                                                                                              , num_threads(num_of_threads){

    mutex_ = new std::mutex();
    cv = new std::condition_variable();
    taskCount = 0;
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    /// dynamic + busy waiting + atomic
//    for (int i = 0; i < TaskSystemParallelThreadPoolSleeping::num_threads; i++) {
//
//        TaskSystemParallelThreadPoolSleeping::threads[i] = std::thread([runnable, i, num_total_tasks, this]() {
//
//            std::unique_lock<std::mutex> lk(*mutex_);
//
//            while(taskCount == 0) {
//                cv->wait(lk);
//            }
//                int task = q.front();
//                q.pop();
//                taskCount--;
//
//                runnable->runTask(task, num_total_tasks);
//                lk.unlock();
//
//         });
//
//    }
//
//    for (int i =0; i < num_total_tasks; i++){
//        q.push(i);
//        taskCount++;
//    }
//    cv->notify_all();


    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
