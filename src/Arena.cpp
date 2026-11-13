/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2011 University of Aarhus
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
#include "Arena.h"

#include <new>
#include <limits>

Arena Arena::_scratchArena;

Arena::Arena() {
}

Arena::~Arena() {
  while (_block.hasPreviousBlock())
	discardPreviousBlock();
  delete[] _block._blockBegin;
}

Arena::Block::Block():
  _blockBegin(0),
  _freeBegin(0),
  _blockEnd(0),
  _previousBlock(0) {
}

void Arena::growCapacity(const size_t needed) {
  // ** Calcuate size of block (doubles capacity)
  size_t size = std::max(needed, _block.getSize());
  if (size > std::numeric_limits<size_t>::max() / 2)
	throw bad_alloc(); // size * 2 overflows
  size *= 2;
  const size_t minimumAlloc = 16 * 1024 - sizeof(Block) - 16;
  size = std::max(size, minimumAlloc); // avoid many small blocks
  // align size by rounding down
  size = size & ~(MemoryAlignment - 1); // works because m.a. is a power of 2
  ASSERT(size >= needed); // is satisfied because we multiplied by 2
  ASSERT(size % MemoryAlignment == 0);
  if (size > std::numeric_limits<size_t>::max() - sizeof(Block))
	throw bad_alloc(); // size + sizeof(block) overflows

  // ** Save current block information at end of memory area
  if (!_block.isNull()) {
	Block* previousBlock = reinterpret_cast<Block*>(_block._blockEnd);
	*previousBlock = _block;
	_block._previousBlock = previousBlock;
  }

  // ** Allocate buffer and update _block
  char* buffer = FROBBY_NEW char[size + sizeof(Block)];
  _block._blockBegin = buffer;
  _block._freeBegin = buffer;
  _block._blockEnd = buffer + size;
}

void Arena::freeTopFromOldBlock(void* ptr) {
  ASSERT(ptr != 0);
  ASSERT(_block.isEmpty());
  ASSERT(_block._previousBlock != 0);

  ASSERT(_block._previousBlock->debugIsValid(ptr));
  _block._previousBlock->_freeBegin = static_cast<char*>(ptr);
  if (_block._previousBlock->isEmpty())
	discardPreviousBlock();
}

void Arena::freeAndAllAfterFromOldBlock(void* ptr) {
  ASSERT(!_block.isInBlock(ptr));
  ASSERT(_block._previousBlock != 0);

  _block._freeBegin = _block._blockBegin;
  while (!(_block._previousBlock->isInBlock(ptr))) {
	discardPreviousBlock();
	ASSERT(_block._previousBlock != 0);
  }

  ASSERT(_block._previousBlock->debugIsValid(ptr));
  _block._previousBlock->_freeBegin = static_cast<char*>(ptr);
  if (_block._previousBlock->isEmpty())
	discardPreviousBlock();
}

void Arena::discardPreviousBlock() {
  ASSERT(_block._previousBlock != 0);
  Block* before = _block._previousBlock->_previousBlock;
  delete[] _block._previousBlock->_blockBegin;
  _block._previousBlock = before;
}

#ifdef DEBUG
bool Arena::Block::debugIsValid(const void* ptr) const {
  return _blockBegin <= ptr && ptr < _freeBegin;
}
#endif
