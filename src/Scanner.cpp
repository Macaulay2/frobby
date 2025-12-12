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
#include "Scanner.h"

#include "VarNames.h"
#include "IOHandler.h"
#include "error.h"
#include "FrobbyStringStream.h"
#include <limits>

static const size_t BufferSize = 10024;

Scanner::Scanner(const string& formatName, FILE* in):
  _in(in),
  _lineNumber(1),
  _char(' '),
  _tmpString(0),
  _tmpStringCapacity(16),
  _formatName(formatName),
  _buffer(BufferSize),
  _bufferPos(_buffer.end()) {
  if (getFormat() == getFormatNameIndicatingToGuessTheInputFormat())
    setFormat(autoDetectFormat(*this));
  _tmpString = new char[16];
}

unique_ptr<IOHandler> Scanner::createIOHandler() const {
  return ::createIOHandler(getFormat());
}

void Scanner::expect(const char* str) {
  ASSERT(str != 0);

  eatWhite();

  const char* it = str;
  while (*it != '\0') {
    int character = getChar();
    if (*it == character) {
      ++it;
      continue;
    }

    // Read the rest of what is there to improve error message.
    // TODO: read at least one char in total even if not alnum.
    FrobbyStringStream got;
    if (character == EOF && it == str)
      got << "no more input";
    else {
      got << '\"' << string(str, it);
      if (isalnum(character))
        got << static_cast<char>(character);
      while (isalnum(peek()))
        got << static_cast<char>(getChar());
      got << '\"';
    }

    reportErrorUnexpectedToken(str, got);
  }
}

void Scanner::expectEOF() {
  // TODO: get this moved into the null format itself.
  if (_formatName == "null")
    return;

  eatWhite();
  if (getChar() != EOF)
    reportErrorUnexpectedToken("no more input", "");
}

void Scanner::errorExpectTwo(char a, char b, int got) {
  ASSERT(a != got && b != got);
  FrobbyStringStream err;
  err << a << " or " << b;
  reportErrorUnexpectedToken(err, got);
}

void Scanner::errorExpectOne(char expected, int got) {
  ASSERT(expected != got);
  string expectedStr;
  expectedStr += expected;
  reportErrorUnexpectedToken(expectedStr, got);
}

size_t Scanner::readIntegerString() {
  eatWhite();

  ASSERT(_tmpStringCapacity > 1);

  if (peek() == '-' || peek() == '+')
    _tmpString[0] = static_cast<char>(getChar());
  else
    _tmpString[0] = '+';

  size_t size = 1;

  while (isdigit(peek())) {
    _tmpString[size] = static_cast<char>(getChar());
    ++size;
    if (size == _tmpStringCapacity)
      growTmpString();
  }
  _tmpString[size] = '\0';

  if (size == 1)
    reportErrorUnexpectedToken("an integer", "");

  return size;
}

size_t Scanner::readIntegerStringNoSign() {
  eatWhite();
  if (peek() == '-' || peek() == '+')
    reportErrorUnexpectedToken("integer without preceding sign", peek());
  // todo: remove code duplication with readIntegerString.

  ASSERT(_tmpStringCapacity > 1);

  size_t size = 0;
  while (isdigit(peek())) {
    _tmpString[size] = static_cast<char>(getChar());
    ++size;
    if (size == _tmpStringCapacity)
      growTmpString();
  }
  _tmpString[size] = '\0';

  if (size == 0)
    reportErrorUnexpectedToken("an integer", "");

  return size;
}

void Scanner::parseInteger(mpz_class& integer, size_t size) {
  // This code has a fast path for small integers and a slower path
  // for longer integers. The largest number representable in 32 bits
  // has 10 digits in base 10 and 1 char for the sign. If the number
  // we are reading has less than 10 digits, then we calculate it
  // directly without consulting GMP, which is faster.

  if (size < 10) {
    signed long l = 0;
    for (size_t i = 1; i < size; ++i)
      l = 10 * l + (_tmpString[i] - '0');
    if (_tmpString[0] == '-')
      l = -l;
    integer = l;
  } else {
    // For whatever reason mp_set_str does not support a + as the
    // first character.
    mpz_set_str(integer.get_mpz_t(), _tmpString + (_tmpString[0] != '-'), 10);
  }
}

void Scanner::readIntegerAndNegativeAsZero(mpz_class& integer) {
  // Fast path for common case of reading a zero.
  if (peek() == '0') {
    getChar();
    if (!isdigit(peek())) {
      integer = 0;
      return;
    }
  }

  size_t size = readIntegerString();
  if (_tmpString[0] == '-')
    integer = 0;
  else
    parseInteger(integer, size);
}

void Scanner::readIntegerAndNegativeAsZero(string& integer) {
  // Fast path for common case of reading a zero.
  if (peek() == '0') {
    getChar();
    if (!isdigit(peek())) {
      integer = '0';
      return;
    }
  }

  readIntegerString();
  if (_tmpString[0] == '-')
    integer = '0';
  else
    integer = _tmpString + 1;
}

void Scanner::readSizeT(size_t& size) {
  readInteger(_integer);

  // Deal with different possibilities for how large size_t is.
  if (sizeof(size_t) == sizeof(unsigned int)) {
    if (!_integer.fits_uint_p()) {
      FrobbyStringStream  errorMsg;
      errorMsg << "expected non-negative integer of size at most "
               << numeric_limits<unsigned int>::max()
               << " but got " << _integer << '.';
      reportSyntaxError(*this, errorMsg);
    }
    size = (unsigned int)_integer.get_ui();
  } else if (sizeof(size_t) == sizeof(unsigned long)) {
    if (!_integer.fits_ulong_p()) {
      FrobbyStringStream errorMsg;
      errorMsg << "expected non-negative integer of size at most "
               << numeric_limits<unsigned long>::max()
               << " but got " << _integer << '.';
      reportSyntaxError(*this, errorMsg);
    }
    size = _integer.get_ui(); // returns an unsigned long despite the name.
  } else {
    FrobbyStringStream errorMsg;
    errorMsg <<
      "Frobby does not work on this machine due to an "
      "unexpected technical issue.\n"
      "Please contact the developers of Frobby about this.\n"
      "\n"
      "Details that will be useful to the developers:\n"
      " error location: Scanner::readSizeT\n"
      " sizeof(size_t) = " << sizeof(size_t) << "\n"
      " sizeof(unsigned int) = " << sizeof(unsigned int) << "\n"
      " sizeof(unsigned long) = " << sizeof(unsigned long) << "\n";
    reportInternalError(errorMsg);
  }
}

void Scanner::growTmpString() {
  ASSERT(_tmpStringCapacity > 0);
  size_t newCapacity = _tmpStringCapacity * 2;
  char* str = new char[newCapacity];
  for (size_t i = 0; i < _tmpStringCapacity; ++i)
    str[i] = _tmpString[i];
  delete[] _tmpString;

  _tmpString = str;
  _tmpStringCapacity = newCapacity;
}

const char* Scanner::readIdentifier() {
  eatWhite();
  if (!isalpha(peek()))
    errorReadIdentifier();

  ASSERT(_tmpStringCapacity > 0);

  size_t size = 0;
  while (isalnum(peek()) || peek() == '_') {
    _tmpString[size] = static_cast<char>(getChar());
    ++size;
    if (size == _tmpStringCapacity)
      growTmpString();
  }
  _tmpString[size] = '\0';

  return _tmpString;
}

void Scanner::errorReadIdentifier() {
  reportErrorUnexpectedToken("an identifier", "");
}

void Scanner::errorReadVariable(const char* name) {
  FrobbyStringStream errorMsg;
  errorMsg << "Unknown variable \"" << name << "\". Maybe you forgot a *.";
  reportSyntaxError(*this, errorMsg);
}

void Scanner::reportErrorUnexpectedToken
(const string& expected, int got) {
  FrobbyStringStream gotDescription;
  if (got == EOF)
    gotDescription << "no more input";
  else
    gotDescription << '\"' << static_cast<char>(got)<< '\"';
  reportErrorUnexpectedToken(expected, gotDescription);
}

void Scanner::reportErrorUnexpectedToken
(const string& expected, const string& got) {
  FrobbyStringStream errorMsg;
  errorMsg << "Expected " << expected;
  if (got != "")
    errorMsg << ", but got " << got;
  errorMsg << '.';
  reportSyntaxError(*this, errorMsg);
}

int Scanner::readBuffer() {
  if (_buffer.size() < _buffer.capacity() && (feof(_in) || ferror(_in)))
    return EOF;
  _buffer.resize(_buffer.capacity());
  size_t read = fread(&_buffer[0], 1, _buffer.capacity(), _in);
  _buffer.resize(read);
  _bufferPos = _buffer.begin();
  if (read == 0)
    return EOF;
  char c = *_bufferPos;
  ++_bufferPos;
  return c;
}
