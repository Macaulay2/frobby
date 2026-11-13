/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2007 Bjarke Hammersholt Roune (www.broune.com)

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
#include "Partition.h"

Partition::Partition():
  _partitions(0),
  _size(0),
  _capacity(0) {
}

Partition::Partition(const Partition& partition):
  _size(partition._size),
  _capacity(partition._size),
  _setCount(partition._setCount) {
  _partitions = FROBBY_NEW int[_size];
  std::copy(partition._partitions,
            partition._partitions + _size, _partitions);
}

Partition::~Partition() {
  delete[] _partitions;
}

void Partition::reset(size_t size) {
  if (size > _capacity) {
    delete[] _partitions;
    _partitions = FROBBY_NEW int[size];
    _capacity = size;
  }

  _size = size;
  _setCount = size;
  fill_n(_partitions, _size, -1);
}

bool Partition::join(size_t i, size_t j) {
  ASSERT(i < _size);
  ASSERT(j < _size);

  size_t rootI = getRoot(i);
  size_t rootJ = getRoot(j);

  if (rootI == rootJ)
    return false;

  ASSERT(_partitions[rootJ] < 0);
  ASSERT(_partitions[rootI] < 0);

  // +1 to subtract the initial -1
  _partitions[rootI] += _partitions[rootJ] + 1;
  _partitions[rootJ] = rootI;
  --_setCount;

  return true;
}

size_t Partition::getSetCount() const {
#ifdef DEBUG
  size_t setCount = 0;
  for (size_t i = 0; i < _size; ++i)
    if (i == getRoot(i))
      ++setCount;
  ASSERT(_setCount == setCount);
#endif

  return _setCount;
}

size_t Partition::getSizeOfClassOf(size_t i) const {
  return -_partitions[getRoot(i)];
}

size_t Partition::getSetSize(size_t set) const {
  for (size_t i = 0; i < _size; ++i) {
    if (i == getRoot(i)) {
      if (set == 0) {
        ASSERT(_partitions[i] < 0);
        return -(_partitions[i] + 1); // +1 to offset the initial -1
      }
      --set;
    }
  }
  ASSERT(false);
  return 0;
}

size_t Partition::getRoot(size_t i) const {
  ASSERT(i < _size);
  if (_partitions[i] >= 0) {
    _partitions[i] = getRoot(_partitions[i]);
    return _partitions[i];
  } else
    return i;
}

void Partition::addToSet(size_t i) {
  ASSERT(i < _size);
  ASSERT(_partitions[getRoot(i)] < 0);
  _partitions[getRoot(i)] -= 1;
}

size_t Partition::getSize() const {
  return _size;
}

void Partition::print(FILE* file) const {
  fprintf(file, "Partition(size=%lu sets:", (unsigned long)_size);
  for (size_t i = 0; i < _size; ++i)
    fprintf(file, " %li", (long)_partitions[i]);
  fputc('\n', file);
}
