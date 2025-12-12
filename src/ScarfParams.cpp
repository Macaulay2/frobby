/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2010 University of Aarhus
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
#include "ScarfParams.h"

#include "CliParams.h"
#include "StringParameter.h"
#include "BoolParameter.h"

namespace {
  static const char* DeformToStrongName = "deformStrong";
  static const bool DeformToStrongDefault = false;

  static const char* EnumerationOrderName = "enum";
  static const char* EnumerationOrderDefault = "revlex";

  static const char* DeformationOrderName = "deformationOrder";
  static const char* DeformationOrderDefault = "tdeg_lex";
}

ScarfParams::ScarfParams(CliParams& cli):
  _deformStrong(DeformToStrongDefault),
  _enumerationOrder(EnumerationOrderDefault),
  _deformationOrder(DeformationOrderDefault) {
  extractCliValues(*this, cli);
}

void addScarfParams(CliParams& params) {
  ASSERT(!params.hasParam(DeformToStrongName));
  params.add
    (unique_ptr<Parameter>
     (new BoolParameter
      (DeformToStrongName,
       "Deform to a strongly generic ideal if true. "
       "Otherwise deform to a weakly generic ideal.",
       DeformToStrongDefault)));

  ASSERT(!params.hasParam(EnumerationOrderName));
  params.add
    (unique_ptr<Parameter>
     (new StringParameter
      (EnumerationOrderName,
       "The enumeration order used for the deformation algorithm.",
       EnumerationOrderDefault)));

  ASSERT(!params.hasParam(DeformationOrderName));
  params.add
    (unique_ptr<Parameter>
     (new StringParameter
      (DeformationOrderName,
       "The deformation order used for the deformation algorithm.",
       DeformationOrderDefault)));
}

void extractCliValues(ScarfParams& scarf, const CliParams& cli) {
  extractCliValues(static_cast<CommonParams&>(scarf), cli);
  scarf.setDeformToStronglyGeneric(getBool(cli, DeformToStrongName));
  scarf.setEnumerationOrder(getString(cli, EnumerationOrderName));
  scarf.setDeformationOrder(getString(cli, DeformationOrderName));
}
