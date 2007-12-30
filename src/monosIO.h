#ifndef MONOS_IO_GUARD
#define MONOS_IO_GUARD

#include "io.h"

class Lexer;
class VarNames;
class BigIdeal;

class MonosIOHandler : public IOHandler {
public:
  MonosIOHandler();
  
  virtual void readIdeal(FILE* in, BigIdeal& ideal);

  virtual IdealWriter* createWriter(FILE* file,
				    const VarNames& names) const;

  virtual void readIrreducibleDecomposition(FILE* in,
					    BigIdeal& decom);

  virtual const char* getFormatName() const;

  virtual IOHandler* createNew() const;

private:
  void readIrreducibleIdeal(BigIdeal& ideal, Lexer& lexer);

  void readIrreducibleIdealList(BigIdeal& ideals, Lexer& lexer);

  void readVarsAndClearIdeal(BigIdeal& ideal, Lexer& lexer);
};

#endif
