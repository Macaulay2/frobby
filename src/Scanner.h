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
#ifndef SCANNER_GUARD
#define SCANNER_GUARD

#include "VarNames.h"
#include <string>

class IOHandler;

/** This class offers an input interface which is more convenient and
 for some purposes more efficient than dealing with a FILE*
 directly. It keeps track of the current line number to report better
 error messages. Only one Scanner should be reading from a given
 FILE*, since otherwise the line numbers will be inaccurate.

 All input methods whose documentation does not specifically say
 otherwise skip whitespace as defined by the standard isspace()
 method.

 There are four concepts for consuming input through a Scanner:

 Read X: Require an X to be in the input, and return what is read.

 Expect X: Require the exact value X to be in the input and skip past it.

 Match X: Return true if the exact value X is in the input, and in that case
 skip past it. Otherwise return false and do nothing else.

 Peek X: Return true if X is the next thing int he input. Do not skip
 past anything. May or may not skip whitespace depending on what X is.

 If a requirement is not met, Scanner reports a syntax error using the
 functions in the error.h header.
*/
class Scanner {
public:
  /** Construct a Scanner object.

   @param formatName The format being read. Is used for e.g. error
    messages.
   @param in The file to read input from.
   */
  Scanner(const string& formatName, FILE* in);
  ~Scanner() {delete[] _tmpString;}

  const string& getFormat() const {return _formatName;}
  void setFormat(const string& format) {_formatName = format;}
  unique_ptr<IOHandler> createIOHandler() const;

  /** Return true if the next character is c, and in that case skip
      past it. */
  bool match(char c);

  /** Return true if no more input. */
  bool matchEOF();

  /** Require the next character to be equal to expected. This
      character is skipped past. */
  void expect(char expected);

  /** Require the next character to be equal to a or b. This character
      is skipped past. */
  void expect(char a, char b);

  /** Require the following characters to be equal to str. These
      characters are skipped past. */
  void expect(const char* str);

  /** Require the following characters to be equal to str. These
      characters are skipped past. */
  void expect(const string& str) {expect(str.c_str());}

  /** Require that there is no more input. */
  void expectEOF();

  /** Read an arbitrary-precision integer. */
  void expectIntegerNoSign();

  /** Read an arbitrary-precision integer. */
  void readInteger(mpz_class& integer);

  /** Read an arbitrary-precision integer. */
  void readIntegerNoSign(string& str);

    /** Read an arbitrary-precision integer. */
  void readIntegerNoSign(mpz_class& str);

  /** Read an integer and set it to zero if it is negative. This is
      more efficient because the sign can be detected before the
      integer is read. */
  void readIntegerAndNegativeAsZero(mpz_class& integer);

    /** Read an integer and set it to zero if it is negative. This is
      more efficient because the sign can be detected before the
      integer is read. */
  void readIntegerAndNegativeAsZero(std::string& integer);

  /** Reads a size_t, where the representable range of that type
      determines when the number is too big. The number is required to
      be non-negative. */
  void readSizeT(size_t& size);

  /** The returned string is only valid until the next method on this
      object gets called. */
  const char* readIdentifier();

  /** Reads an identifier into str. str must be empty. */
  void readIdentifier(string& str);

  /** Reads an identifier and returns the index of that identifier as
      the index of a variable in names. Throws an exception if there
      is no identifier or the identifier is not a variable in
      names. */
  size_t readVariable(const VarNames& names);

  /** Skips whitespace and returns true if the next token is an
      identifier. */
  bool peekIdentifier();

  /** Returns true if the next character is whitespace. Does not,
      obviously, skip whitespace. */
  bool peekWhite() {return isspace(peek());}

  /** Skips whitespace and returns true if the next character is equal
      to the parameter(s). */
  bool peek(char character);

  /** Returns the number of newlines seen. Does not skip
      whitespace. */
  unsigned int getLineNumber() const {return _lineNumber;}

  /** Returns the next character or EOF. Does not skip whitespace. */
  int peek() {return _char;}

  /** Reads past any whitespace, where whitespace is defined by the
      standard function isspace(). */
  inline void eatWhite();

private:
  /** Returns the size of the string. */
  size_t readIntegerString();

  /** Returns the size of the string. */
  size_t readIntegerStringNoSign();

  void parseInteger(mpz_class& integer, size_t size);

  void errorExpectTwo(char a, char b, int got);
  void errorExpectOne(char expected, int got);
  void errorReadVariable(const char* name);
  void errorReadIdentifier();

  void reportErrorUnexpectedToken(const string& expected, int got);
  void reportErrorUnexpectedToken(const string& expected, const string& got);

  inline int getChar();
  void growTmpString();
  int readBuffer();

  mpz_class _integer;
  FILE* _in;
  unsigned long _lineNumber;
  int _char; // next character on stream

  char* _tmpString;
  size_t _tmpStringCapacity;

  string _formatName;

  vector<char> _buffer;
  vector<char>::iterator _bufferPos;
};



inline void Scanner::readIdentifier(string& str) {
  eatWhite();
  if (!isalpha(peek()))
    errorReadIdentifier();
  str.clear();
  do {
    str += static_cast<char>(getChar());
  } while (isalnum(peek()) || peek() == '_');
}

inline size_t Scanner::readVariable(const VarNames& names) {
  const char* name = readIdentifier();
  size_t var = names.getIndex(name);
  if (var == VarNames::invalidIndex)
    errorReadVariable(name);
  return var;
}


inline bool Scanner::matchEOF() {
  eatWhite();
  return peek() == EOF;
}

inline bool Scanner::match(char c) {
  eatWhite();
  if (c == peek()) {
    getChar();
    return true;
  } else
    return false;
}

inline void Scanner::expect(char a, char b) {
  eatWhite();
  int got = getChar();
  if (got != a && got != b)
    errorExpectTwo(a, b, got);
}

inline void Scanner::expect(char expected) {
  eatWhite();
  int got = getChar();
  if (got != expected)
    errorExpectOne(expected, got);
}

inline void Scanner::readInteger(mpz_class& integer) {
  size_t size = readIntegerString();
  parseInteger(integer, size);
}

inline void Scanner::expectIntegerNoSign() {
  readIntegerStringNoSign();
}

inline void Scanner::readIntegerNoSign(mpz_class& integer) {
  readIntegerStringNoSign();
  integer = _tmpString;
}

inline void Scanner::readIntegerNoSign(string& integer) {
  readIntegerStringNoSign();
  integer = _tmpString;
}

inline bool Scanner::peekIdentifier() {
  eatWhite();
  return isalpha(peek());
}

inline bool Scanner::peek(char character) {
  eatWhite();
  return peek() == character;
}

inline void Scanner::eatWhite() {
  while (isspace(peek()))
    getChar();
}

inline int Scanner::getChar() {
  if (_char == '\n')
    ++_lineNumber;
  int oldChar = _char;
  if (_bufferPos == _buffer.end())
    _char = readBuffer();
  else {
    _char = *_bufferPos;
    ++_bufferPos;
  }
  return oldChar;
}

#endif
