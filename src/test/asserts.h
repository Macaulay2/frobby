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
#ifndef ASSERTS_GUARD
#define ASSERTS_GUARD

#include <stdexcept>
#include <sstream>
#include <vector>
#include <ostream>

class AssertException : public logic_error {
 public:
  AssertException(const string& str);
  AssertException(const AssertException& e);
};

namespace TestInternal {
  struct StdData {
  StdData(size_t lineParam, const char* fileParam,
    const char* testNameParam, bool printDotParam):
	line(lineParam), file(fileParam),
    testName(testNameParam), printDot(printDotParam) {}
	size_t line;
	const char* file;
	const char* testName;
	bool printDot;
  };
#define STD_DATA TestInternal::StdData(__LINE__, __FILE__, __nameOfTest, __printDots)
#define SILENT(X) \
  {bool ___printDots = __printDots; __printDots = false; X; __printDots = ___printDots;}

  template<class T>
	string toStr(const T& t) {stringstream out;out << t;return out.str();}

  // Forward-declaration due to new behaviour in GCC 12
  // see https://stackoverflow.com/a/77928633/5894824
  template<class T>
	ostream& operator<<(ostream& out, const vector<T>& v);

  void assertOK(const StdData&);
  void assertFail(const char* cond, const char* expected, const StdData&);
  void assertFail1(const char* cond, const char* expected, const StdData&,
				   const char* exp1, string exp1Value);
  void assertFail2(const char* cond, const char* expected, const StdData&,
				   const char* exp1, string exp1Value,
				   const char* exp2, string exp2Value);

  void assertSucceeded(bool printDot);

  void assertFailed(const char* errorMsg,
					const char* testName, const char* file, size_t line);

  void assertTrue(bool value, const char* condition,
				  const char* testName, const char* file, size_t line,
				  bool printDot);

#define ASSERT_EXCEPTION(CODE, EXCEPTION_TYPE) \
  {bool _frobby_sawException = false; \
   try {CODE;} catch (EXCEPTION_TYPE) {_frobby_sawException = true;}	\
   if (_frobby_sawException)assertOK(STD_DATA); \
   else assertFail(#CODE,"throw exception of type " #EXCEPTION_TYPE,STD_DATA);}

#define ASSERT_TRUE(VALUE) \
  {if(VALUE)assertOK(STD_DATA);else assertFail(#VALUE,"true",STD_DATA);}
#define ASSERT_TRUE_SILENT(VALUE) SILENT(ASSERT_TRUE(VALUE))

#define ASSERT_TRUE1(VALUE, X)											\
  {if(VALUE)assertOK(STD_DATA);else assertFail1(#VALUE,"true",STD_DATA,#X,TestInternal::toStr(X));}
#define ASSERT_FALSE1(VALUE, X)											\
  {if(!VALUE)assertOK(STD_DATA);else assertFail1(#VALUE,"false",STD_DATA,#X,TestInternal::toStr(X));}

#define ASSERT_TRUE2(VALUE, X, Y)											\
  {if(VALUE)assertOK(STD_DATA);else assertFail2(#VALUE,"true",STD_DATA,#X,TestInternal::toStr(X),#Y,TestInternal::toStr(Y));}
#define ASSERT_FALSE2(VALUE, X, Y)											\
  {if(!VALUE)assertOK(STD_DATA);else assertFail2(#VALUE,"false",STD_DATA,#X,TestInternal::toStr(X),#Y,TestInternal::toStr(Y));}

  void assertTrue2Failed(const char* valueString,
						 const char* testName, const char* file, size_t line,
						 const char* expression1, const char* expression1Value,
						 const char* expression2, const char* expression2Value);
  template<class A, class B>
	void assertTrue2(bool value, const char* valueString,
					 const char* testName, const char* file, size_t line,
					 const char* expression1, const A& a,
					 const char* expression2, const B& b,
					 bool printDot) {
	if (value) {
	  assertSucceeded(printDot);
	  return;
	}

	stringstream aValue;
	aValue << a;

	stringstream bValue;
	bValue << b;

	assertTrue2Failed(valueString, testName, file, line,
					  expression1, aValue.str().c_str(),
					  expression2, bValue.str().c_str());
  }
#define ASSERT_TRUE2_SILENT(VALUE, A, B)								\
  TestInternal::assertTrue2(VALUE, #VALUE, __nameOfTest, __FILE__, __LINE__, \
							#A,A,#B,B, false);


  void assertFalse(bool value, const char* condition,
				   const char* testName, const char* file, size_t line,
				   bool printDot);
#define ASSERT_FALSE(VALUE)												\
  TestInternal::assertFalse(VALUE, #VALUE, __nameOfTest, __FILE__, __LINE__, __printDots)
#define ASSERT_FALSE_SILENT(VALUE)										\
  TestInternal::assertFalse(VALUE, #VALUE, __nameOfTest, __FILE__, __LINE__, false)

  void assertEqualFailed(const char* a, const char* b,
						 const char* aString, const char* bString,
						 const char* testName, const char* file, size_t line);
  template<class T1, class T2>
	void assertEqual(const T1& a, const T2& b,
					 const char* aString, const char* bString,
					 const char* testName, const char* file, size_t line,
					 bool printDot) {
	if (a == b) {
	  assertSucceeded(printDot);
	  return;
	}

	stringstream aValue;
	aValue << a;

	stringstream bValue;
	bValue << b;

	assertEqualFailed(aValue.str().c_str(), bValue.str().c_str(),
					  aString, bString,
					  testName, file, line);
  }
#define ASSERT_EQ(A, B)													\
  TestInternal::assertEqual(A, B, #A, #B, __nameOfTest, __FILE__, __LINE__, __printDots)
#define ASSERT_EQ_SILENT(A, B)											\
  TestInternal::assertEqual(A, B, #A, #B, __nameOfTest, __FILE__, __LINE__, false)

  void assertNotEqualFailed(const char* a, const char* b,
							const char* aString, const char* bString,
							const char* testName, const char* file, size_t line);
  template<class T1, class T2>
	void assertNotEqual(const T1& a, const T2& b,
						const char* aString, const char* bString,
						const char* testName, const char* file, size_t line,
						bool printDot) {
	if (a != b) {
	  assertSucceeded(printDot);
	  return;
	}

	stringstream aValue;
	aValue << a;

	stringstream bValue;
	bValue << b;

	assertNotEqualFailed(aValue.str().c_str(), bValue.str().c_str(),
						 aString, bString,
						 testName, file, line);
  }
#define ASSERT_NEQ(A, B)												\
  TestInternal::assertNotEqual(A, B, #A, #B, __nameOfTest, __FILE__, __LINE__, __printDots)
#define ASSERT_NEQ_SILENT(A, B)											\
  TestInternal::assertNotEqual(A, B, #A, #B, __nameOfTest, __FILE__, __LINE__, false)

  /** Prints out a vector. This is useful when vectors are used in
   tests. The operator is only defined in this namespace so it does
   not escape to pollute the global namespce. */
  template<class T>
	ostream& operator<<(ostream& out, const vector<T>& v) {
	out << " std::vector<>: ";
	for (typename vector<T>::const_iterator it = v.begin(); it != v.end(); ++it)
	  out << (it == v.begin() ? "" : ", ") << *it;
	out << '\n';
	return out;
  }
}

#endif
