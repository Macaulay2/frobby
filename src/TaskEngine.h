/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 University of Aarhus
   Contact Bjarke Hammersholt Roune for license information (www.broune.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see http://www.gnu.org/licenses/.
*/
#ifndef TASK_ENGINE_GUARD
#define TASK_ENGINE_GUARD

#include <vector>

class Task;

/** TaskEngine handles a list of tasks that are to be carried out.

 Pending tasks are run in last-in-first-out order. TaskEngine passes
 itself to tasks that are run so that tasks can schedule more tasks.

 Using TaskEngine is an alternative to using recursion to schedule
 sub-computations. TaskEngine does what the stack would do in a
 recursive computation. An advantage of TaskEngine over recursion is
 that stack overflow does not occur. The maximum number of pending
 tasks is limited only by the size of the memory.

 TaskEngine is also the first step towards an implementation where
 sub-computations can be run in parallel. At that time TaskEngine will
 need support for specifying dependencies among tasks.
*/
class TaskEngine {
 public:
  TaskEngine();
  ~TaskEngine();

  /** Add a task at the head of the list of pending tasks.

   TaskEngine guarantees to call either run() or dispose() on the task
   at some point. It is allowed to add the same task several times,
   though then run() or dispose() will be called as many times as the
   task has been added.
  */
  void addTask(Task* task);

  /** Run the most recently added task that has not been run yet.

   Returns true if a task has been run. Returns false if there are no
   pending tasks.
  */
  bool runNextTask();

  /** Runs all pending tasks.

   If a task adds new tasks, those are run as well.
  */
  void runTasks();

  /** Returns the number of times addTask has been successfully
   called. This value may overflow if it becomes too large to contain
   in a size_t.
  */
  size_t getTotalTasksEver();

  /** Dispose of all pending tasks without running them. */
  void disposeAll();

 private:
  void dispose(Task* task);

  /** This is used for statistics so that it is not a disaster if this
   overflows for very long-running computations. */
  size_t _totalTasksEver;

  vector<Task*> _tasks;
};

#endif
