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
#ifndef NAME_FACTORY_GUARD
#define NAME_FACTORY_GUARD

#include "error.h"
#include <vector>
#include <string>
#include <algorithm>

/** A NameFactory takes a name and then creates an instance of a class
 that has been previously registered under that name. This is done
 in a general way using templates.

 None of this is very efficient. However, the interface can be
 implemented much more efficiently if that becomes necessary.
*/
template<class AbstractProduct>
class NameFactory {
 public:
  /** @param abstractName The name for those things that are being
   generated in general. Used for error messages. */
  NameFactory(const char* abstractName): _abstractName(abstractName) {}

  typedef unique_ptr<AbstractProduct> (*FactoryFunction)();
  void registerProduct(const string& name, FactoryFunction function);

  /** Calls the function registered to the parameter name and returns
   the result. Returns null if name has not been registered. Can still
   throw an exception for example if out of memory. */
  unique_ptr<AbstractProduct> createNoThrow(const string& name) const;

  /** Calls the function registered to the parameter name and returns
   the result. Throws an exception if name has not been registered. */
  unique_ptr<AbstractProduct> create(const string& name) const;

  /** Inserts into names all registered names that have the indicated
   prefix in lexicographic increasing order. */
  void getNamesWithPrefix(const string& prefix, vector<string>& names) const;

  /** Returns true if no names have been registered. */
  bool empty() const;

  string getAbstractProductName() const;

 private:
  typedef pair<string, FactoryFunction> Pair;
  typedef typename vector<Pair>::const_iterator const_iterator;
  vector<Pair> _pairs;
  const string _abstractName;
};

/** Registers the string returned by ConcreteProduct::getStaticName()
 to a function that default-constructs a ConcreteProduct. */
template<class ConcreteProduct, class AbstractProduct>
void nameFactoryRegister(NameFactory<AbstractProduct>& factory);

/** Creates the unique product that has the indicated prefix, or
 create the actual product that has name equal to the indicated
 prefix. Exceptions thrown are as for getUniqueNamesWithPrefix(). */
template<class AbstractProduct>
unique_ptr<AbstractProduct> createWithPrefix
(const NameFactory<AbstractProduct>& factory, const string& prefix);

/** Returns the unique product name that has the indicated prefix, or
 return prefix itself if it is the actual name of a product.

 @exception UnknownNameException If no product has the indicated
 prefix.

 @exception AmbiguousNameException If more than one product has the
 indicated prefix and the prefix is not the actual name of any
 product. */
template<class AbstractProduct>
string getUniqueNameWithPrefix
(const NameFactory<AbstractProduct>& factory, const string& prefix);


// **************************************************************
// These are implementations that have to be included here due
// to being templates.

template<class AbstractProduct>
unique_ptr<AbstractProduct> NameFactory<AbstractProduct>::
createNoThrow(const string& name) const {
  for (const_iterator it = _pairs.begin(); it != _pairs.end(); ++it)
    if (it->first == name)
      return it->second();
  return unique_ptr<AbstractProduct>();
}

template<class AbstractProduct>
unique_ptr<AbstractProduct> NameFactory<AbstractProduct>::
create(const string& name) const {
  unique_ptr<AbstractProduct> product = createNoThrow(name);
  if (product.get() == 0)
    throwError<UnknownNameException>(
      "Unknown " + getAbstractProductName() + " \"" + name + "\".");
  return product;
}

template<class AbstractProduct>
void NameFactory<AbstractProduct>::
registerProduct(const string& name, FactoryFunction function) {
  _pairs.push_back(Pair(name, function));
}

template<class AbstractProduct>
void NameFactory<AbstractProduct>::
getNamesWithPrefix(const string& prefix, vector<string>& names) const {
  for (const_iterator it = _pairs.begin(); it != _pairs.end(); ++it)
    if (it->first.compare(0, prefix.size(), prefix) == 0)
      names.push_back(it->first);
  sort(names.begin(), names.end());
}

template<class AbstractProduct>
bool NameFactory<AbstractProduct>::empty() const {
  return _pairs.empty();
}

template<class AbstractProduct>
string NameFactory<AbstractProduct>::getAbstractProductName() const {
  return _abstractName;
}

template<class ConcreteProduct, class AbstractProduct>
void nameFactoryRegister(NameFactory<AbstractProduct>& factory) {
  struct HoldsFunction {
    static unique_ptr<AbstractProduct> createConcreteProduct() {
      return unique_ptr<AbstractProduct>(new ConcreteProduct());
    }
  };
  factory.registerProduct(ConcreteProduct::staticGetName(),
                          HoldsFunction::createConcreteProduct);
}

template<class AbstractProduct>
unique_ptr<AbstractProduct> createWithPrefix
(const NameFactory<AbstractProduct>& factory, const string& prefix) {
  return factory.createNoThrow(getUniqueNameWithPrefix(factory, prefix));
}

template<class AbstractProduct>
string getUniqueNameWithPrefix
(const NameFactory<AbstractProduct>& factory, const string& prefix) {
  vector<string> names;
  factory.getNamesWithPrefix(prefix, names);

  if (find(names.begin(), names.end(), prefix) != names.end()) {
    names.clear();
    names.push_back(prefix);
  }

  if (names.empty()) {
    throwError<UnknownNameException>
      ("No " + factory.getAbstractProductName() +
       " has the prefix \"" + prefix + "\".");
  }

  if (names.size() >= 2) {
    string errorMsg = "More than one " + factory.getAbstractProductName() +
      " has prefix \"" + prefix + "\":\n ";
    for (size_t name = 0; name < names.size(); ++name)
      errorMsg += ' ' + names[name];
    throwError<AmbiguousNameException>(errorMsg);
  }

  ASSERT(names.size() == 1);
  return names.back();
}

#endif
