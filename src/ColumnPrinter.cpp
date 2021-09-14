/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 Bjarke Hammersholt Roune (www.broune.com)

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
#include "ColumnPrinter.h"

#include "FrobbyStringStream.h"

namespace {
  size_t getLineWidth(const string& str, size_t pos) {
	size_t startPos = pos;
	while (pos < str.size() && str[pos] != '\n')
	  ++pos;
	return pos - startPos;
  }

  void printSpaces(ostream& out, size_t howMany) {
	while (howMany > 0) {
	  out << ' ';
	  --howMany;
	}
  }
}

ColumnPrinter::ColumnPrinter(size_t columnCount):
  _cols(),
  _colsDeleter(_cols) {
  while (columnCount > 0) {
	addColumn();
    --columnCount;
  }
}

void ColumnPrinter::setPrefix(const string& prefix) {
  _prefix = prefix;
}

void ColumnPrinter::addColumn(bool flushLeft,
							  const string& prefix, const string& suffix) {
  auto_ptr<Col> col(new Col());
  col->prefix = prefix;
  col->suffix = suffix;
  col->flushLeft = flushLeft;

  exceptionSafePushBack(_cols, std::move(col));
}

size_t ColumnPrinter::getColumnCount() const {
  return _cols.size();
}

ostream& ColumnPrinter::operator[](size_t col) {
  ASSERT(col < getColumnCount());
  return _cols[col]->text;
}

void ColumnPrinter::print(ostream& out) const {
  // Calculate the width of each column.
  vector<size_t> widths(getColumnCount());
  for (size_t col = 0; col < getColumnCount(); ++col) {
	const string& text = _cols[col]->text.str();
	size_t maxWidth = 0;

	size_t pos = 0;
	while (pos < text.size()) {
	  size_t width = getLineWidth(text, pos);
	  if (width > maxWidth)
		maxWidth = width;

	  // We can't just increment pos unconditionally by width + 1, as
	  // that could result in an overflow.
	  pos += width;
	  if (text[pos] == '\n')
		++pos;
	}
	widths[col] = maxWidth;
  }

  // Print each row
  vector<size_t> poses(getColumnCount());
  while (true) {
	bool done = true;
	for (size_t col = 0; col < getColumnCount(); ++col) {
	  if (poses[col] < _cols[col]->text.str().size()) {
		done = false;
		break;
	  }
	}
	if (done)
	  break;

	out << _prefix;
	for (size_t col = 0; col < getColumnCount(); ++col) {
	  out << _cols[col]->prefix;

	  const string& text = _cols[col]->text.str();
	  size_t& pos = poses[col];
	  size_t width = getLineWidth(text, pos);

	  if (!_cols[col]->flushLeft)
		printSpaces(out, widths[col] - width);

      while (pos < text.size()) {
        if (text[pos] == '\n') {
          ++pos;
          break;
        }
		out << text[pos];
		++pos;
	  }

	  if (_cols[col]->flushLeft)
		printSpaces(out, widths[col] - width);

	  out << _cols[col]->suffix;
	}
	out << '\n';
  }
}

void ColumnPrinter::print(FrobbyStringStream& out) const {
  ostringstream tmp;
  print(tmp);
  out << tmp.str().c_str();
}

ostream& operator<<(ostream& out, const ColumnPrinter& printer) {
  printer.print(out);
  return out;
}

void print(FILE* out, const ColumnPrinter& pr) {
  ostringstream str;
  str << pr;
  fputs(str.str().c_str(), out);
}
