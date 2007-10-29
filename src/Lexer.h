#ifndef LEXER_GUARD
#define LEXER_GUARD

class Lexer {
public:
  Lexer(istream& in);

  bool match(char c);

  void expect(char expected);
  void expect(const char* str);
  void expect(const string& str);
  void expectEOF();

  void readInteger(mpz_class& integer);
  void readInteger(unsigned int& i);

  void readIdentifier(string& identifier);

private:
  int getChar();

  int peek();

  void error(const string& expected);

  void eatWhite();

  mpz_class _integer;
  istream& _in;
  unsigned int _lineNumber;
};

#endif