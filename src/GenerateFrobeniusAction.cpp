#include "stdinc.h"
#include "GenerateFrobeniusAction.h"

#include "BigIdeal.h"
#include "RandomDataFacade.h"
#include "IOFacade.h"

const char* GenerateFrobeniusAction::getName() const {
  return "genfrob";
}

const char* GenerateFrobeniusAction::getShortDescription() const {
  return "Generate a random Frobenius problem instance.";
}

const char* GenerateFrobeniusAction::getDescription() const {
  return "Generate a random Frobenius problem instance.";
}

Action* GenerateFrobeniusAction::createNew() const {
  return new GenerateFrobeniusAction();
}

void GenerateFrobeniusAction::
obtainParameters(vector<Parameter*>& parameters) {
  Action::obtainParameters(parameters);
}

void GenerateFrobeniusAction::perform() {
  vector<mpz_class> instance;

  RandomDataFacade randomDataFacade(_printActions);
  randomDataFacade.generateFrobeniusInstance(instance);

  IOFacade ioFacade(_printActions);
  ioFacade.writeFrobeniusInstance(cout, instance);
}