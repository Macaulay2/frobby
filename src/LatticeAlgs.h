/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2011 Bjarke Hammersholt Roune (www.broune.com)

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
#ifndef LATTICE_ALGS_GUARD
#define LATTICE_ALGS_GUARD

#include "SatBinomIdeal.h"
#include "IOFacade.h"
#include "Scanner.h"
#include "IOHandler.h"
#include "DataType.h"
#include "BigIdeal.h"
#include "MsmStrategy.h"
#include "TermTranslator.h"
#include "TranslatingTermConsumer.h"
#include "DebugStrategy.h"
#include "Matrix.h"
#include "BigTermRecorder.h"
#include "SliceParams.h"
#include "SliceFacade.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <limits>
#include <fstream>
#include <map>

// Computations and data structures on lattices.
// Support for LatticeAnalyzeAction.

#include <iostream>

// wrapped in do .. while(false) to make it act like a single statement.
#define CHECK(X)									\
  do {												\
	if (!(X)) {										\
	  cout << "Check condition on line "			\
		   << __LINE__ << " of file " << __FILE__	\
		   << " not satisfied:\n  "#X << endl;		\
	  exit(1);										\
	}				                                \
  } while (false)

enum NeighborPlace {
  InPlane,
  UnderPlane,
  OverPlane,
  NoPlace
};

char getPlaceCode(NeighborPlace place);

class Mlfb;

struct SeqPos {
  SeqPos();
  SeqPos(const Mlfb* mlfb, size_t nextFacet, size_t previousFacet);
  size_t getForwardFacet() const;
  size_t getBackFacet() const;
  SeqPos getReverse() const;
  void order();
  bool operator<(const SeqPos& pos) const;

  const Mlfb* mlfb;
  size_t fixFacet1;
  size_t fixFacet2;
  size_t comingFromFacet;
};

class GrobLat;

class Neighbor {
 public:
  Neighbor(); // invalid
  Neighbor(const GrobLat& lat); // zero
  Neighbor(const GrobLat& lat, const size_t row); // row of lat
  Neighbor(const Neighbor&) = default;

  Neighbor& operator=(const Neighbor& neighbor) {
	_lat = neighbor._lat;
	_row = neighbor._row;
	return *this;
  }

  bool operator==(const Neighbor& neighbor) const {
	return _lat == neighbor._lat && getRow() == neighbor.getRow();
  }

  const mpq_class& getH(size_t i) const;
  size_t getHDim() const;

  const mpq_class& getY(size_t i) const;
  size_t getYDim() const;

  size_t getRow() const {return _row;}

  bool isZero() const;
  bool isValid() const;
  bool isSpecial() const;
  bool isGenerator() const;

  string getName() const;
  const GrobLat& getGrobLat() const {return *_lat;}

 private:
  const GrobLat* _lat;
  size_t _row;
};

/** A lattice with associated Grobner basis/neighbors. */
class GrobLat {
 public:
  GrobLat(const Matrix& matrix, const SatBinomIdeal& ideal);

  Neighbor getNeighbor(size_t row) const {
	ASSERT(row < getNeighborCount());
	return Neighbor(*this, row);
  }

  size_t getNeighborCount() const {
	ASSERT(_h.getRowCount() == _y.getRowCount());
	return _y.getRowCount();
  }

  const Matrix& getYMatrix() const {return _y;}
  const Matrix& getHMatrix() const {return _h;}
  const Matrix& getMatrix() const {return _mat;}

  const SatBinomIdeal& getIdeal() const {return _ideal;}

  Neighbor getSum(Neighbor a, Neighbor b) const {
	vector<mpq_class> sum(getHDim());
	for (size_t i = 0; i < getHDim(); ++i)
	  sum[i] = _h(a.getRow(), i) + _h(b.getRow(), i);
	for (size_t row = 0; row < _h.getRowCount(); ++row) {
	  bool match = true;
	  for (size_t col = 0; col < _h.getColCount(); ++col)
		if (sum[col] != _h(row, col))
		  match = false;
	  if (match)
		return Neighbor(*this, row);
	}
	return Neighbor();
  }

  Neighbor getSum(size_t a, size_t b) const {
	vector<mpq_class> sum(getHDim());
	for (size_t i = 0; i < getHDim(); ++i)
	  sum[i] = _h(a, i) + _h(b, i);
	for (size_t row = 0; row < _h.getRowCount(); ++row) {
	  bool match = true;
	  for (size_t col = 0; col < _h.getColCount(); ++col)
		if (sum[col] != _h(row, col))
		  match = false;
	  if (match)
		return Neighbor(*this, row);
	}
	return Neighbor();
  }

  size_t getYDim() const {
	ASSERT(_y.getColCount() == _ideal.getVarCount());
	return _y.getColCount();
  }

  size_t getHDim() const {
	return _h.getColCount();
  }

  bool hasZeroEntryY() const {
	return _ideal.hasZeroEntry();
  }

  void getInitialIdeal(BigIdeal& ideal) const {
	_ideal.getInitialIdeal(ideal);
  }

  bool isSum(Neighbor n) const {
	ASSERT(n.isValid());
	ASSERT(&n.getGrobLat() == this);
	ASSERT(n.getRow() < _isSumRow.size());
	return _isSumRow[n.getRow()];
  }
  const vector<Neighbor>& getNonSums() const {return _nonSums;}
  const mpq_class& getZero() const {return _zero;} // todo: remove

  /// Returns true if the smallest body containing zero, a and b has
  /// no neighbor in its interior.
  bool isPointFreeBody(Neighbor a, Neighbor b) const {
	return _ideal.isPointFreeBody(_ideal.getGenerator(a.getRow()),
								  _ideal.getGenerator(b.getRow()));
  }

  /// Returns true if the smallest body containing zero, a, b and c
  /// has no neighbor in its interior.
  bool isPointFreeBody(Neighbor a, Neighbor b, Neighbor c) const {
	return _ideal.isPointFreeBody(_ideal.getGenerator(a.getRow()),
								  _ideal.getGenerator(b.getRow()),
								  _ideal.getGenerator(c.getRow()));
  }

  bool isInterior(Neighbor a, Neighbor b) const {
    if (!isPointFreeBody(a, b))
      return false;
    for (size_t var = 1; var < a.getYDim(); ++var)
      if (a.getY(var) <= 0 && b.getY(var) <= 0)
        return false;
    return true;
  }

 private:
  vector<bool> _isSumRow;
  vector<Neighbor> _nonSums;

  Matrix _y; // rows are neighbors in y-space
  Matrix _h; // rows are neighbors in h-space
  Matrix _mat; // matrix that defines lattice
  SatBinomIdeal _ideal; // other representation of _y, necessary for now
  mpq_class _zero;
};

class Tri {
 public:
  Tri(Neighbor a, Neighbor b, Neighbor sum,
	  const vector<Mlfb>& mlfbs, const GrobLat& lat);

  Neighbor getA() const {return _a;}
  Neighbor getB() const {return _b;}
  Neighbor getSum() const {return _sum;}
  const vector<const Mlfb*>& getASideMlfbs() const {return _aSideMlfbs;}
  const vector<const Mlfb*>& getBSideMlfbs() const {return _bSideMlfbs;}
  const vector<Neighbor>& getNeighborsOnBoundary() const {return _boundary;}
  const vector<Neighbor>& getNeighborsInInterior() const {return _interior;}

 private:
  Neighbor _a;
  Neighbor _b;
  Neighbor _sum; // neighbor that is sum of a and b
  vector<const Mlfb*> _aSideMlfbs; // MLFBs containing {0,a,sum}
  vector<const Mlfb*> _bSideMlfbs; // MLFBs containing {0,b,sum}
  vector<Neighbor> _interior; // neighbors on boundary of <0,a,b,sum>
  vector<Neighbor> _boundary; // neighbors in interior of <0,a,b,sum>
};

class Plane {
 public:
Plane(Neighbor a, Neighbor b, Neighbor sum,
	  const vector<Mlfb>& mlfbs, const GrobLat& lat):
  tri(a, b, sum, mlfbs, lat) {}

  size_t getTypeCount(size_t type) const;
  size_t getMaxType() const;
  NeighborPlace getPlace(Neighbor neighbor) const;
  bool inPlane(Neighbor neighbor) const;
  bool isPivot(const Mlfb& mlfb) const;
  bool isSidePivot(const Mlfb& mlfb) const;
  bool isFlat(const Mlfb& mlfb) const;
  bool is22(const Mlfb& mlfb) const;
  size_t getType(const Mlfb& mlfb) const;

  bool hasFlat() const {
	return getTypeCount(4) > 0;
  }

  Matrix nullSpaceBasis;
  Tri tri;
  Matrix rowAB;
  size_t flatIntervalCount;

  map<size_t, size_t> typeCounts;
  vector<NeighborPlace> neighborPlace;
  vector<SeqPos> flatSeq;
  vector<const Mlfb*> pivots;
};

class Mlfb {
 public:
  size_t getMinInitialFacet() const {
	return minInitialFacet;
  }

  bool hasPoint(Neighbor n) const {
	for (size_t i = 0; i < _points.size(); ++i)
	  if (_points[i] == n)
		return true;
	return false;
  }

  Neighbor getPoint(size_t offset) const {
	ASSERT(offset < getPointCount());
	return _points[offset];
  }

  size_t getPointCount() const {
	return _points.size();
  }

  bool operator==(const Mlfb& mlfb) const {
	return _offset == mlfb._offset;
  }

  size_t getOffset() const {
	return _offset;
  }

  const vector<mpz_class>& getRhs() const {
	return _rhs;
  }

  string getName() const {
	ostringstream name;
	name << 'm' << (getOffset() + 1);
	return name.str();
  }

  string getName(const Plane& plane) const {
	if (plane.isPivot(*this))
	  return getName() + 'P';
	if (plane.isFlat(*this))
	  return getName() + 'F';
	return getName();
  }

  Neighbor getHitsNeighbor(size_t indexParam) const {
	ASSERT(indexParam < edgeHitsFacet.size());
	return getPoint(getHitsFacet(indexParam));
  }

  size_t getHitsFacet(size_t indexParam) const {
	ASSERT(indexParam < edgeHitsFacet.size());
	return edgeHitsFacet[indexParam];
  }

  const Mlfb* getEdge(size_t indexParam) const {
	ASSERT(indexParam < edges.size());
	return edges[indexParam];
  }

  Mlfb* getEdge(size_t indexParam) {
	ASSERT(indexParam < edges.size());
	return edges[indexParam];
  }

  size_t getFacetOf(const Mlfb& adjacent) const {
	for (size_t i = 0; i < 4; ++i)
	  if (*getEdge(i) == adjacent)
		return i;
	return 4;
  }

  bool isParallelogram() const {
	return _isParallelogram;
  }

  mpq_class index;
  mpz_class dotDegree;
  vector<Mlfb*> edges;
  vector<size_t> edgeHitsFacet;
  size_t minInitialFacet;

  void reset(size_t offset, const vector<Neighbor>& points);

 private:
  vector<mpz_class> _rhs;
  vector<Neighbor> _points;
  size_t _offset;
  bool _isParallelogram;
};

class TriPlane {
public:
  TriPlane(Neighbor a, Neighbor b, Neighbor c):
	_a(a), _b(b), _c(c) {

	Matrix mat(2, 3);
	for (size_t col = 0; col < 3; ++col) {
	  mat(0, col) = a.getH(col) - c.getH(col);
	  mat(1, col) = b.getH(col) - c.getH(col);
	}

	nullSpace(_normal, mat);
	transpose(_normal, _normal);
	_line = (_normal.getRowCount() != 1);
  }

  bool isLine() const {
	return _line;
  }

  bool closeToPlane(Neighbor a) {
	ASSERT(!isLine());
	mpz_class dn = dotNormal(a);
	return dn == 0 || dn == 1 || dn == -1;
  }

  bool inPlane(Neighbor a) const {
	return dotNormal(a) == 0;
  }

  mpz_class dotNormal(Neighbor a) const {
	mpz_class prod = 0;
	for (size_t i = 0; i < 3; ++i)
	  prod += a.getH(i) * _normal(0, i);
	return prod;
  }

  bool isParallel(const TriPlane& plane) const {
	ASSERT(!isLine());
	ASSERT(!plane.isLine());
	mpz_class da = plane.dotNormal(_a);
	return plane.dotNormal(_b) == da && plane.dotNormal(_c) == da;
  }

  bool isParallel(const Plane& plane) const {
	ASSERT(!isLine());
	return plane.nullSpaceBasis == getNormal();
  }

  /// returns the normal of the plane as the row of a matrix.
  const Matrix& getNormal() const {
	return _normal;
  }

private:
  Neighbor _a, _b, _c;
  Matrix _normal;
  bool _line;
};

void getThinPlanes(vector<TriPlane>& planes, const GrobLat& lat);
void checkPlanes(const vector<TriPlane>& thinPlanes,
				 const vector<Plane>& dtPlanes);

size_t pushOutFacetPositive(size_t facetPushOut,
							const vector<mpz_class>& rhs,
							const GrobLat& lat);

size_t pushOutFacetZero(const vector<mpz_class>& rhs, const GrobLat& lat);

void computeMlfbs(vector<Mlfb>& mlfbs, const GrobLat& lat);


void computeSeqs(vector<vector<SeqPos> >& left,
				 vector<vector<SeqPos> >& right,
				 const vector<Mlfb>& mlfbs,
				 const Plane& plane);

/** Starting at pivot (which must be a pivot), follow the three
	non-flat sequences starting at pivot. */
void computePivotSeqs(vector<vector<SeqPos> >& seqs, const Mlfb& pivot,
					  const Plane& plane);

void checkSeqs(const vector<vector<SeqPos> >& left,
			   const vector<vector<SeqPos> >& right,
			   const Plane& plane,
			   const vector<Mlfb>& mlfbs);
void checkMiddle(const Plane& plane,
				 const vector<Mlfb>& mlfbs);
void checkDoubleTriangle(const Plane& plane,
						 const vector<Mlfb>& mlfbs);
void checkGraph(const vector<Mlfb>& mlfbs);
void checkGraphOnPlane(const Plane& plane,
					   const vector<Mlfb>& mlfbs);


/** Perform checks where pivotSeqs are the 3 non-flat sequences on one
	side. */
void checkPivotSeqs(vector<vector<SeqPos> >& pivotSeqs,
					const Plane& plane,
					const vector<Mlfb>& mlfbs,
					const vector<SeqPos>& flatSeq);

void checkPlaneTri(const GrobLat& lat,
				   const vector<Mlfb>& mlfbs,
				   const vector<const Mlfb*>& pivots,
				   const Plane& plane);

void computePlanes(vector<Plane>& planes,
				   const GrobLat& lat,
				   vector<Mlfb>& mlfbs);

/** Set the plane vertex count for each mlfb and count how many MLFBs
	have each possible number of vertices. */
void setupPlaneCountsAndOrder(vector<Mlfb>& mlfbs,
							  const Plane& plane,
							  map<size_t, size_t>& typeCounts);

bool disjointSeqs(const vector<SeqPos>& a, const vector<SeqPos>& b);

/** Put all pivots into pivots. flatSeq must be the sequence of
	flats. If flatSeq is not empty, then offsets 0,1 will be the left
	pivots while 2,3 will be the right pivots. */
void computePivots(vector<const Mlfb*>& pivots,
				   const vector<Mlfb>& mlfbs,
				   const Plane& plane,
				   const vector<SeqPos>& flatSeq);

void checkNonSums(const GrobLat& lat);

void checkFlatSeq(const vector<SeqPos>& flatSeq,
				  const GrobLat& lat,
				  const Plane& plane);

const char* getEdgePos(size_t index);
mpq_class getIndexSum(const vector<Mlfb>& mlfbs);

void checkMlfbs(const vector<Mlfb>& mlfbs, const GrobLat& lat);
void checkDoubleTrianglePlanes(const vector<Plane>& planes,
							   const GrobLat& lat,
							   const vector<Mlfb>& mlfbs);
void checkPlane(const Plane& plane, const vector<Mlfb>& mlfbs);

#endif
