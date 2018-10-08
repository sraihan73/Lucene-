using namespace std;

#include "PointInShapeIntersectVisitor.h"

namespace org::apache::lucene::spatial3d
{
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using GeoArea = org::apache::lucene::spatial3d::geom::GeoArea;
using GeoAreaFactory = org::apache::lucene::spatial3d::geom::GeoAreaFactory;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using XYZBounds = org::apache::lucene::spatial3d::geom::XYZBounds;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using NumericUtils = org::apache::lucene::util::NumericUtils;

PointInShapeIntersectVisitor::PointInShapeIntersectVisitor(
    shared_ptr<DocIdSetBuilder> hits, shared_ptr<GeoShape> shape,
    shared_ptr<XYZBounds> bounds)
    : hits(hits), shape(shape),
      minimumX(Geo3DDocValuesField::roundDownX(bounds->getMinimumX())),
      maximumX(Geo3DDocValuesField::roundUpX(bounds->getMaximumX())),
      minimumY(Geo3DDocValuesField::roundDownY(bounds->getMinimumY())),
      maximumY(Geo3DDocValuesField::roundUpY(bounds->getMaximumY())),
      minimumZ(Geo3DDocValuesField::roundDownZ(bounds->getMinimumZ())),
      maximumZ(Geo3DDocValuesField::roundUpZ(bounds->getMaximumZ()))
{
}

void PointInShapeIntersectVisitor::grow(int count)
{
  adder = hits->grow(count);
}

void PointInShapeIntersectVisitor::visit(int docID) { adder->add(docID); }

void PointInShapeIntersectVisitor::visit(int docID,
                                         std::deque<char> &packedValue)
{
  assert(packedValue.size() == 12);
  double x = Geo3DPoint::decodeDimension(packedValue, 0);
  double y = Geo3DPoint::decodeDimension(packedValue, Integer::BYTES);
  double z = Geo3DPoint::decodeDimension(packedValue, 2 * Integer::BYTES);
  if (x >= minimumX && x <= maximumX && y >= minimumY && y <= maximumY &&
      z >= minimumZ && z <= maximumZ) {
    if (shape->isWithin(x, y, z)) {
      adder->add(docID);
    }
  }
}

Relation
PointInShapeIntersectVisitor::compare(std::deque<char> &minPackedValue,
                                      std::deque<char> &maxPackedValue)
{
  // Because the dimensional format operates in quantized (64 bit -> 32 bit)
  // space, and the cell bounds here are inclusive, we need to extend the bounds
  // to the largest un-quantized values that could quantize into these bounds.
  // The encoding (Geo3DUtil.encodeValue) does a Math.round from double to long,
  // so e.g. 1.4 -> 1, and -1.4 -> -1:
  double xMin = Geo3DUtil::decodeValueFloor(
      NumericUtils::sortableBytesToInt(minPackedValue, 0));
  double xMax = Geo3DUtil::decodeValueCeil(
      NumericUtils::sortableBytesToInt(maxPackedValue, 0));
  double yMin = Geo3DUtil::decodeValueFloor(
      NumericUtils::sortableBytesToInt(minPackedValue, 1 * Integer::BYTES));
  double yMax = Geo3DUtil::decodeValueCeil(
      NumericUtils::sortableBytesToInt(maxPackedValue, 1 * Integer::BYTES));
  double zMin = Geo3DUtil::decodeValueFloor(
      NumericUtils::sortableBytesToInt(minPackedValue, 2 * Integer::BYTES));
  double zMax = Geo3DUtil::decodeValueCeil(
      NumericUtils::sortableBytesToInt(maxPackedValue, 2 * Integer::BYTES));

  // System.out.println("  compare: x=" + cellXMin + "-" + cellXMax + " y=" +
  // cellYMin + "-" + cellYMax + " z=" + cellZMin + "-" + cellZMax);
  assert(xMin <= xMax);
  assert(yMin <= yMax);
  assert(zMin <= zMax);

  // First, check bounds.  If the shape is entirely contained, return
  // CELL_CROSSES_QUERY.
  if (minimumX >= xMin && maximumX <= xMax && minimumY >= yMin &&
      maximumY <= yMax && minimumZ >= zMin && maximumZ <= zMax) {
    return Relation::CELL_CROSSES_QUERY;
  }

  // Quick test failed so do slower one...
  shared_ptr<GeoArea> xyzSolid = GeoAreaFactory::makeGeoArea(
      PlanetModel::WGS84, xMin, xMax, yMin, yMax, zMin, zMax);

  switch (xyzSolid->getRelationship(shape)) {
  case GeoArea::CONTAINS:
    // Shape fully contains the cell
    // System.out.println("    inside");
    return Relation::CELL_INSIDE_QUERY;
  case GeoArea::OVERLAPS:
    // They do overlap but neither contains the other:
    // System.out.println("    crosses1");
    return Relation::CELL_CROSSES_QUERY;
  case GeoArea::WITHIN:
    // Cell fully contains the shape:
    // System.out.println("    crosses2");
    // return Relation.SHAPE_INSIDE_CELL;
    return Relation::CELL_CROSSES_QUERY;
  case GeoArea::DISJOINT:
    // They do not overlap at all
    // System.out.println("    outside");
    return Relation::CELL_OUTSIDE_QUERY;
  default:
    assert(false);
    return Relation::CELL_CROSSES_QUERY;
  }
}
} // namespace org::apache::lucene::spatial3d