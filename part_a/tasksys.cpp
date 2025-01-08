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

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::thread threads[num_total_tasks];

    for (int i = 0; i < num_total_tasks; i++) {
        threads[i] = std::thread([runnable, i, num_total_tasks]() {
             runnable->runTask(i, num_total_tasks);
         });
    }

    for (int i = 0; i < num_total_tasks; i++) {
        threads[i].join();
    }
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_of_threads): ITaskSystem(num_of_threads)
                                                                                              , threads(num_of_threads)
                                                                                              , num_threads(num_of_threads){

    mutex_ = new std::mutex();
    taskCount = 0;

    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
//    TaskSystemParallelThreadPoolSpinning::threads = std::vector<std::thread>(num_threads);

}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    delete mutex_;
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    /// static or dynamic assignment of threads?

    /// static + no busy waiting
//    for (int i = 0; i < TaskSystemParallelThreadPoolSpinning::num_threads; i++) {
//
//        TaskSystemParallelThreadPoolSpinning::threads[i] = std::thread([runnable, i, num_total_tasks, this]() {
//
//            for (int j = i; j < num_total_tasks; j += num_threads)
//                runnable->runTask(j, num_total_tasks);
//         });
//
//    }
//
//
//    for (int i = 0; i < num_threads; i++) {
//        TaskSystemParallelThreadPoolSpinning::threads[i].join();
//    }

    /// dynamic + busy waiting + atomic
    for (int i =0; i < num_total_tasks; i++){
//        q.push(i);
        taskCount++;
    }
//    std::cout << taskCount;
//    mutex
    for (int i = 0; i < TaskSystemParallelThreadPoolSpinning::num_threads; i++) {

        TaskSystemParallelThreadPoolSpinning::threads[i] = std::thread([runnable, i, num_total_tasks, this]() {

//            while (true) {
//                while(taskCount == 0) {}
//                while(taskCount > 0 ) {
                mutex_->lock();
//                int task = q.front();
//                q.pop();
                int task = --taskCount;
                mutex_->unlock();

                runnable->runTask(task, num_total_tasks);
//                }
//                taskCount = -1;
//                            }
         });

    }

        for (int i = 0; i < num_threads; i++) {
            TaskSystemParallelThreadPoolSpinning::threads[i].join();
        }

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
    for (int i = 0; i < TaskSystemParallelThreadPoolSleeping::num_threads; i++) {

        TaskSystemParallelThreadPoolSleeping::threads[i] = std::thread([runnable, i, num_total_tasks, this]() {

            std::unique_lock<std::mutex> lk(*mutex_);

            while(taskCount == 0) {
                cv->wait(lk);
            }
                int task = q.front();
                q.pop();
                taskCount--;

                runnable->runTask(task, num_total_tasks);
                lk.unlock();

         });

    }

    for (int i =0; i < num_total_tasks; i++){
        q.push(i);
        taskCount++;
    }
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
