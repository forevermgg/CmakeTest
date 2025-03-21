#pragma once
/**
 * Overview
 * ========
 *
 * A simple implementation of a scheduler (thread pool). Allows to schedule
 * tasks and futures.
 */

#include <functional>
#include <memory>

#include "move_to_lambda.h"

/**
 * A Worker allows to schedule tasks which are executed sequentially.
 * Workers are created from a Scheduler.
 *
 * Lifetime and destruction:
 *
 *   - The scheduler from which a worker is created must not be destructed
 *     before the worker.
 *
 *   - The worker must not be destructed before all its tasks are finished.
 */
class Worker {
 public:
  virtual ~Worker() = default;
  Worker() = default;

  Worker(Worker const&) = delete;
  Worker& operator=(Worker const&) = delete;

  /**
   * Schedules a task on this worker. Tasks are executed strictly sequentially
   * in the order they are scheduled.
   */
  virtual void Schedule(std::function<void()> task) = 0;
};

/**
 * A Scheduler which allows to schedule 'tasks'.
 *
 * Lifetime and destruction:
 *
 *   - A Scheduler *must* be idle (no active or pending work) at destruction
 *     time. See WaitUntilIdle.
 *
 *   - Implies: A Scheduler *must not* be destructed by one of its own tasks
 *
 *   - Implies: Task closures may safely hold raw pointers to their thread pool.
 *     They should *not* have ownership (via a smart-pointer or similar).
 */
class Scheduler {
 public:
  virtual ~Scheduler() = default;
  Scheduler() = default;

  Scheduler(Scheduler const&) = delete;
  Scheduler& operator=(Scheduler const&) = delete;

  /**
   * Creates a new Worker based on this scheduler.
   */
  virtual std::unique_ptr<Worker> CreateWorker();

  /**
   * Schedules a task that will execute on the scheduler.
   */
  virtual void Schedule(std::function<void()> task) = 0;

  /**
   * Waits until there are no tasks running or pending.
   *
   * In this state, the thread pool will not restart working until some
   * external entity is scheduling new tasks, as work caused by tasks spawning
   * other tasks has ceased.
   */
  virtual void WaitUntilIdle() = 0;
};

/**
 * Creates a scheduler using a fixed-size pool of threads to run tasks.
 */
std::unique_ptr<Scheduler> CreateThreadPoolScheduler(std::size_t thread_count);
