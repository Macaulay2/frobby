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
#include "stdinc.h"
#include "TaskEngine.h"

#include "Task.h"
#include "display.h"

TaskEngine::TaskEngine():
  _totalTasksEver(0) {
}

TaskEngine::~TaskEngine() {
  disposeAll();
}

void TaskEngine::addTask(Task* task) {
  ASSERT(task != 0);

  try {
    _tasks.push_back(task);
  } catch (...) {
    // We should only get an exception if insertion failed.
    ASSERT(_tasks.empty() || _tasks.back() != task);
    dispose(task);
    throw;
  }

  ++_totalTasksEver;
}

bool TaskEngine::runNextTask() {
  if (_tasks.empty())
    return false;

  Task* task = _tasks.back();
  _tasks.pop_back();
  task->run(*this);

  return true;
}

void TaskEngine::runTasks() {
  while (runNextTask())
    ;
}

size_t TaskEngine::getTotalTasksEver() {
  return _totalTasksEver;
}

void TaskEngine::disposeAll() {
  while (!_tasks.empty()) {
    dispose(_tasks.back());
    _tasks.pop_back();
  }
}

void TaskEngine::dispose(Task* task) {
  ASSERT(task != 0);

  try {
    task->dispose();
  } catch (...) {
    displayInternalError("Task::dispose() threw an exception.");
    ASSERT(false);
    throw; // Lesser evil compared to ignoring the exception.
  }
}
