#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <iostream>
#include <condition_variable>
#include <unordered_set>
#include <map>
#include <queue>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */

class waitingTask {
    public:
        int task_id;
        int num_total_tasks;
        IRunnable* runnable;
        std::vector<TaskID> deps;

        bool operator==(const waitingTask& other) const {
            return task_id == other.task_id;
        }
};

class workingTask {
    public:
        int task_id;
        int task_now;
        int tasks_left;
        int num_total_tasks;
        IRunnable* runnable;
};

namespace std {
    template <>
    struct hash<waitingTask> {
        std::size_t operator()(const waitingTask& k) const {
            return std::hash<int>()(k.task_id);
        }
    };
}

class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
  private:
      std::vector<std::thread> threads;
      int num_threads;
      std::mutex* runningQueueMutex;
      std::mutex* waitingForDepsQueueMutex;
      std::mutex* tasksDoneMutex;
      std::mutex* task_id_mutex;
      std::mutex* threads_sleeping_mutex;
      std::condition_variable*  threads_sleeping_cv;
      std::condition_variable*  waitingForDeps_cv;
      std::condition_variable*  sync_cv;



      std::map<TaskID, bool> finishedTasks;
      std::map<TaskID, int> tasksDone;

      TaskID task_id;
      bool syncing;
      bool finished;
      std::queue<workingTask> runningQueue;
      std::unordered_set<waitingTask> waitingForDepsQueue;
  public:
      TaskSystemParallelThreadPoolSleeping(int num_threads);
      ~TaskSystemParallelThreadPoolSleeping();
      const char* name();
      void threadRun(int thread_id);
      void run(IRunnable* runnable, int num_total_tasks);
      TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                              const std::vector<TaskID>& deps);
      void sync();
};

#endif
