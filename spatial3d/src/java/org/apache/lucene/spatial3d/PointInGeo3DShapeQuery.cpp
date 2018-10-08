using namespace std;

#include "PointInGeo3DShapeQuery.h"

namespace org::apache::lucene::spatial3d
{
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BasePlanetObject = org::apache::lucene::spatial3d::geom::BasePlanetObject;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using XYZBounds = org::apache::lucene::spatial3d::geom::XYZBounds;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;

PointInGeo3DShapeQuery::PointInGeo3DShapeQuery(const wstring &field,
                                               shared_ptr<GeoShape> shape)
    : field(field), shape(shape), shapeBounds(make_shared<XYZBounds>())
{
  shape->getBounds(shapeBounds);

  if (std::dynamic_pointer_cast<BasePlanetObject>(shape) != nullptr) {
    shared_ptr<BasePlanetObject> planetObject =
        std::static_pointer_cast<BasePlanetObject>(shape);
    if (planetObject->getPlanetModel()->equals(PlanetModel::WGS84) == false) {
      throw invalid_argument(
          L"this qurey requires PlanetModel.WGS84, but got: " +
          planetObject->getPlanetModel());
    }
  }
}

shared_ptr<Weight>
PointInGeo3DShapeQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                     bool needsScores,
                                     float boost) 
{

  // I don't use RandomAccessWeight here: it's no good to approximate with
  // "match all docs"; this is an inverted structure and should be used in the
  // first pass:

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

PointInGeo3DShapeQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<PointInGeo3DShapeQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
PointInGeo3DShapeQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<PointValues> values = reader->getPointValues(outerInstance->field);
  if (values == nullptr) {
    return nullptr;
  }

  /*
  XYZBounds bounds = new XYZBounds();
  shape.getBounds(bounds);

  final double planetMax = planetModel.getMaximumMagnitude();
  if (planetMax != treeDV.planetMax) {
    throw new IllegalStateException(planetModel + " is not the same one used
  during indexing: planetMax=" + planetMax + " vs indexing planetMax=" +
  treeDV.planetMax);
  }
  */

  /*
  GeoArea xyzSolid = GeoAreaFactory.makeGeoArea(planetModel,
                                                bounds.getMinimumX(),
                                                bounds.getMaximumX(),
                                                bounds.getMinimumY(),
                                                bounds.getMaximumY(),
                                                bounds.getMinimumZ(),
                                                bounds.getMaximumZ());

  assert xyzSolid.getRelationship(shape) == GeoArea.WITHIN ||
  xyzSolid.getRelationship(shape) == GeoArea.OVERLAPS: "expected WITHIN (1) or
  OVERLAPS (2) but got " + xyzSolid.getRelationship(shape) + "; shape="+shape+";
  XYZSolid="+xyzSolid;
  */

  shared_ptr<DocIdSetBuilder> result = make_shared<DocIdSetBuilder>(
      reader->maxDoc(), values, outerInstance->field);

  values->intersect(make_shared<PointInShapeIntersectVisitor>(
      result, outerInstance->shape, outerInstance->shapeBounds));

  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          result->build()->begin());
}

bool PointInGeo3DShapeQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

wstring PointInGeo3DShapeQuery::getField() { return field; }

shared_ptr<GeoShape> PointInGeo3DShapeQuery::getShape() { return shape; }

bool PointInGeo3DShapeQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool PointInGeo3DShapeQuery::equalsTo(shared_ptr<PointInGeo3DShapeQuery> other)
{
  return field == other->field && shape->equals(other->shape);
}

int PointInGeo3DShapeQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + field.hashCode();
  result = 31 * result + shape->hashCode();
  return result;
}

wstring PointInGeo3DShapeQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  sb->append(getClass().getSimpleName());
  sb->append(L':');
  if (this->field == field == false) {
    sb->append(L" field=");
    sb->append(this->field);
    sb->append(L':');
  }
  sb->append(L" Shape: ");
  sb->append(shape);
  return sb->toString();
}
} // namespace org::apache::lucene::spatial3d