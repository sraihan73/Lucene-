using namespace std;

#include "BBoxOverlapRatioValueSource.h"

namespace org::apache::lucene::spatial::bbox
{
using Explanation = org::apache::lucene::search::Explanation;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::shape::Rectangle;

BBoxOverlapRatioValueSource::BBoxOverlapRatioValueSource(
    shared_ptr<ShapeValuesSource> rectValueSource, bool isGeo,
    shared_ptr<Rectangle> queryExtent, double queryTargetProportion,
    double minSideLength)
    : BBoxSimilarityValueSource(rectValueSource), isGeo(isGeo),
      queryExtent(queryExtent),
      queryArea(calcArea(queryExtent->getWidth(), queryExtent->getHeight())),
      minSideLength(minSideLength), queryTargetProportion(queryTargetProportion)
{
  assert(queryArea >= 0);
  if (queryTargetProportion < 0 || queryTargetProportion > 1.0) {
    throw invalid_argument(L"queryTargetProportion must be >= 0 and <= 1");
  }
}

BBoxOverlapRatioValueSource::BBoxOverlapRatioValueSource(
    shared_ptr<ShapeValuesSource> rectValueSource,
    shared_ptr<Rectangle> queryExtent)
    : BBoxOverlapRatioValueSource(rectValueSource, true, queryExtent, 0.25, 0.0)
{
}

bool BBoxOverlapRatioValueSource::equals(any o)
{
  if (!BBoxSimilarityValueSource::equals(o)) {
    return false;
  }

  shared_ptr<BBoxOverlapRatioValueSource> that =
      any_cast<std::shared_ptr<BBoxOverlapRatioValueSource>>(o);

  if (Double::compare(that->minSideLength, minSideLength) != 0) {
    return false;
  }
  if (Double::compare(that->queryTargetProportion, queryTargetProportion) !=
      0) {
    return false;
  }
  if (!queryExtent->equals(that->queryExtent)) {
    return false;
  }

  return true;
}

int BBoxOverlapRatioValueSource::hashCode()
{
  int result = BBoxSimilarityValueSource::hashCode();
  int64_t temp;
  result = 31 * result + queryExtent->hashCode();
  temp = Double::doubleToLongBits(minSideLength);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  temp = Double::doubleToLongBits(queryTargetProportion);
  result = 31 * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  return result;
}

wstring BBoxOverlapRatioValueSource::similarityDescription()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return queryExtent->toString() + L"," + to_wstring(queryTargetProportion);
}

double BBoxOverlapRatioValueSource::score(
    shared_ptr<Rectangle> target,
    shared_ptr<AtomicReference<std::shared_ptr<Explanation>>> exp)
{
  // calculate "height": the intersection height between two boxes.
  double top = min(queryExtent->getMaxY(), target->getMaxY());
  double bottom = max(queryExtent->getMinY(), target->getMinY());
  double height = top - bottom;
  if (height < 0) {
    if (exp != nullptr) {
      exp->set(Explanation::noMatch(L"No intersection"));
    }
    return 0; // no intersection
  }

  // calculate "width": the intersection width between two boxes.
  double width = 0;
  {
    shared_ptr<Rectangle> a = queryExtent;
    shared_ptr<Rectangle> b = target;
    if (a->getCrossesDateLine() == b->getCrossesDateLine()) {
      // both either cross or don't
      double left = max(a->getMinX(), b->getMinX());
      double right = min(a->getMaxX(), b->getMaxX());
      if (!a->getCrossesDateLine()) { // both don't
        if (left <= right) {
          width = right - left;
        } else if (isGeo &&
                   (abs(a->getMinX()) == 180 || abs(a->getMaxX()) == 180) &&
                   (abs(b->getMinX()) == 180 || abs(b->getMaxX()) == 180)) {
          width = 0; // both adjacent to dateline
        } else {
          if (exp != nullptr) {
            exp->set(Explanation::noMatch(L"No intersection"));
          }
          return 0; // no intersection
        }
      } else { // both cross
        width = right - left + 360;
      }
    } else {
      if (!a->getCrossesDateLine()) { // then flip
        a = target;
        b = queryExtent;
      }
      // a crosses, b doesn't
      double qryWestLeft = max(a->getMinX(), b->getMinX());
      double qryWestRight = b->getMaxX();
      if (qryWestLeft < qryWestRight) {
        width += qryWestRight - qryWestLeft;
      }

      double qryEastLeft = b->getMinX();
      double qryEastRight = min(a->getMaxX(), b->getMaxX());
      if (qryEastLeft < qryEastRight) {
        width += qryEastRight - qryEastLeft;
      }

      if (qryWestLeft > qryWestRight && qryEastLeft > qryEastRight) {
        if (exp != nullptr) {
          exp->set(Explanation::noMatch(L"No intersection"));
        }
        return 0; // no intersection
      }
    }
  }

  // calculate queryRatio and targetRatio
  double intersectionArea = calcArea(width, height);
  double queryRatio;
  if (queryArea > 0) {
    queryRatio = intersectionArea / queryArea;
  } else if (queryExtent->getHeight() > 0) { // vert line
    queryRatio = height / queryExtent->getHeight();
  } else if (queryExtent->getWidth() > 0) { // horiz line
    queryRatio = width / queryExtent->getWidth();
  } else {
    queryRatio =
        queryExtent->relate(target).intersects() ? 1 : 0; // could be optimized
  }

  double targetArea = calcArea(target->getWidth(), target->getHeight());
  assert(targetArea >= 0);
  double targetRatio;
  if (targetArea > 0) {
    targetRatio = intersectionArea / targetArea;
  } else if (target->getHeight() > 0) { // vert line
    targetRatio = height / target->getHeight();
  } else if (target->getWidth() > 0) { // horiz line
    targetRatio = width / target->getWidth();
  } else {
    targetRatio =
        target->relate(queryExtent).intersects() ? 1 : 0; // could be optimized
  }
  assert((queryRatio >= 0 && queryRatio <= 1, queryRatio));
  assert((targetRatio >= 0 && targetRatio <= 1, targetRatio));

  // combine ratios into a score

  double queryFactor = queryRatio * queryTargetProportion;
  double targetFactor = targetRatio * (1.0 - queryTargetProportion);
  double score = queryFactor + targetFactor;

  if (exp != nullptr) {
    wstring minSideDesc = minSideLength > 0.0
                              ? L" (minSide=" + to_wstring(minSideLength) + L")"
                              : L"";
    exp->set(Explanation::match(
        static_cast<float>(score),
        this->getClass().getSimpleName() + L": queryFactor + targetFactor",
        {Explanation::match(
             static_cast<float>(intersectionArea),
             L"IntersectionArea" + minSideDesc,
             {Explanation::match(static_cast<float>(width), L"width"),
              Explanation::match(static_cast<float>(height), L"height"),
              Explanation::match(static_cast<float>(queryTargetProportion),
                                 L"queryTargetProportion")}),
         Explanation::match(
             static_cast<float>(queryFactor), L"queryFactor",
             {Explanation::match(static_cast<float>(targetRatio), L"ratio"),
              Explanation::match(static_cast<float>(queryArea),
                                 L"area of " + queryExtent + minSideDesc)}),
         Explanation::match(
             static_cast<float>(targetFactor), L"targetFactor",
             {Explanation::match(static_cast<float>(targetRatio), L"ratio"),
              Explanation::match(static_cast<float>(targetArea),
                                 L"area of " + target + minSideDesc)})}));
  }

  return score;
}

double BBoxOverlapRatioValueSource::calcArea(double width, double height)
{
  return max(minSideLength, width) * max(minSideLength, height);
}
} // namespace org::apache::lucene::spatial::bbox