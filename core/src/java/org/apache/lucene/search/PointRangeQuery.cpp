using namespace std;

#include "PointRangeQuery.h"

namespace org::apache::lucene::search
{
using IntPoint = org::apache::lucene::document::IntPoint;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using StringHelper = org::apache::lucene::util::StringHelper;

PointRangeQuery::PointRangeQuery(const wstring &field,
                                 std::deque<char> &lowerPoint,
                                 std::deque<char> &upperPoint, int numDims)
    : field(field), numDims(numDims), bytesPerDim(lowerPoint.size() / numDims),
      lowerPoint(lowerPoint), upperPoint(upperPoint)
{
  checkArgs(field, lowerPoint, upperPoint);
  if (numDims <= 0) {
    throw invalid_argument(L"numDims must be positive, got " +
                           to_wstring(numDims));
  }
  if (lowerPoint.empty()) {
    throw invalid_argument(L"lowerPoint has length of zero");
  }
  if (lowerPoint.size() % numDims != 0) {
    throw invalid_argument(L"lowerPoint is not a fixed multiple of numDims");
  }
  if (lowerPoint.size() != upperPoint.size()) {
    throw invalid_argument(L"lowerPoint has length=" + lowerPoint.size() +
                           L" but upperPoint has different length=" +
                           upperPoint.size());
  }
}

void PointRangeQuery::checkArgs(const wstring &field, any lowerPoint,
                                any upperPoint)
{
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (lowerPoint == nullptr) {
    throw invalid_argument(L"lowerPoint must not be null");
  }
  if (upperPoint == nullptr) {
    throw invalid_argument(L"upperPoint must not be null");
  }
}

shared_ptr<Weight>
PointRangeQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                              bool needsScores, float boost) 
{

  // We don't use RandomAccessWeight here: it's no good to approximate with
  // "match all docs". This is an inverted structure and should be used in the
  // first pass:

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<PointRangeQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointValues::IntersectVisitor>
PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::getIntersectVisitor(
    shared_ptr<DocIdSetBuilder> result)
{
  return make_shared<IntersectVisitorAnonymousInnerClass>(shared_from_this(),
                                                          result);
}

PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::IntersectVisitorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetBuilder> result)
{
  this->outerInstance = outerInstance;
  this->result = result;
}

void PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::grow(int count)
{
  adder = result->grow(count);
}

void PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID)
{
  adder->add(docID);
}

void PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID,
                                               std::deque<char> &packedValue)
{
  for (int dim = 0; dim < outerInstance->outerInstance.numDims; dim++) {
    int offset = dim * outerInstance->outerInstance.bytesPerDim;
    if (StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, packedValue, offset,
            outerInstance->outerInstance.lowerPoint, offset) < 0) {
      // Doc's value is too low, in this dimension
      return;
    }
    if (StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, packedValue, offset,
            outerInstance->outerInstance.upperPoint, offset) > 0) {
      // Doc's value is too high, in this dimension
      return;
    }
  }

  // Doc is in-bounds
  adder->add(docID);
}

PointValues::Relation PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{

  bool crosses = false;

  for (int dim = 0; dim < outerInstance->outerInstance.numDims; dim++) {
    int offset = dim * outerInstance->outerInstance.bytesPerDim;

    if (StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, minPackedValue, offset,
            outerInstance->outerInstance.upperPoint, offset) > 0 ||
        StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, maxPackedValue, offset,
            outerInstance->outerInstance.lowerPoint, offset) < 0) {
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }

    crosses |= StringHelper::compare(outerInstance->outerInstance.bytesPerDim,
                                     minPackedValue, offset,
                                     outerInstance->outerInstance.lowerPoint,
                                     offset) < 0 ||
               StringHelper::compare(
                   outerInstance->outerInstance.bytesPerDim, maxPackedValue,
                   offset, outerInstance->outerInstance.upperPoint, offset) > 0;
  }

  if (crosses) {
    return PointValues::Relation::CELL_CROSSES_QUERY;
  } else {
    return PointValues::Relation::CELL_INSIDE_QUERY;
  }
}

shared_ptr<PointValues::IntersectVisitor>
PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    getInverseIntersectVisitor(shared_ptr<FixedBitSet> result,
                               std::deque<int> &cost)
{
  return make_shared<IntersectVisitorAnonymousInnerClass2>(shared_from_this(),
                                                           result, cost);
}

PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::IntersectVisitorAnonymousInnerClass2(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<FixedBitSet> result, deque<int> &cost)
{
  this->outerInstance = outerInstance;
  this->result = result;
  this->cost = cost;
}

void PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::visit(int docID)
{
  result->clear(docID);
  cost[0]--;
}

void PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::visit(int docID,
                                                std::deque<char> &packedValue)
{
  for (int dim = 0; dim < outerInstance->outerInstance.numDims; dim++) {
    int offset = dim * outerInstance->outerInstance.bytesPerDim;
    if (StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, packedValue, offset,
            outerInstance->outerInstance.lowerPoint, offset) < 0) {
      // Doc's value is too low, in this dimension
      result->clear(docID);
      cost[0]--;
      return;
    }
    if (StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, packedValue, offset,
            outerInstance->outerInstance.upperPoint, offset) > 0) {
      // Doc's value is too high, in this dimension
      result->clear(docID);
      cost[0]--;
      return;
    }
  }
}

PointValues::Relation PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass2::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{

  bool crosses = false;

  for (int dim = 0; dim < outerInstance->outerInstance.numDims; dim++) {
    int offset = dim * outerInstance->outerInstance.bytesPerDim;

    if (StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, minPackedValue, offset,
            outerInstance->outerInstance.upperPoint, offset) > 0 ||
        StringHelper::compare(
            outerInstance->outerInstance.bytesPerDim, maxPackedValue, offset,
            outerInstance->outerInstance.lowerPoint, offset) < 0) {
      // This dim is not in the range
      return PointValues::Relation::CELL_INSIDE_QUERY;
    }

    crosses |= StringHelper::compare(outerInstance->outerInstance.bytesPerDim,
                                     minPackedValue, offset,
                                     outerInstance->outerInstance.lowerPoint,
                                     offset) < 0 ||
               StringHelper::compare(
                   outerInstance->outerInstance.bytesPerDim, maxPackedValue,
                   offset, outerInstance->outerInstance.upperPoint, offset) > 0;
  }

  if (crosses) {
    return PointValues::Relation::CELL_CROSSES_QUERY;
  } else {
    return PointValues::Relation::CELL_OUTSIDE_QUERY;
  }
}

shared_ptr<ScorerSupplier>
PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();

  shared_ptr<PointValues> values = reader->getPointValues(outerInstance->field);
  if (values == nullptr) {
    // No docs in this segment/field indexed any points
    return nullptr;
  }

  if (values->getNumDimensions() != outerInstance->numDims) {
    throw invalid_argument(
        L"field=\"" + outerInstance->field + L"\" was indexed with numDims=" +
        to_wstring(values->getNumDimensions()) +
        L" but this query has numDims=" + to_wstring(outerInstance->numDims));
  }
  if (outerInstance->bytesPerDim != values->getBytesPerDimension()) {
    throw invalid_argument(L"field=\"" + outerInstance->field +
                           L"\" was indexed with bytesPerDim=" +
                           to_wstring(values->getBytesPerDimension()) +
                           L" but this query has bytesPerDim=" +
                           to_wstring(outerInstance->bytesPerDim));
  }

  bool allDocsMatch;
  if (values->getDocCount() == reader->maxDoc()) {
    const std::deque<char> fieldPackedLower = values->getMinPackedValue();
    const std::deque<char> fieldPackedUpper = values->getMaxPackedValue();
    allDocsMatch = true;
    for (int i = 0; i < outerInstance->numDims; ++i) {
      int offset = i * outerInstance->bytesPerDim;
      if (StringHelper::compare(outerInstance->bytesPerDim,
                                outerInstance->lowerPoint, offset,
                                fieldPackedLower, offset) > 0 ||
          StringHelper::compare(outerInstance->bytesPerDim,
                                outerInstance->upperPoint, offset,
                                fieldPackedUpper, offset) < 0) {
        allDocsMatch = false;
        break;
      }
    }
  } else {
    allDocsMatch = false;
  }

  shared_ptr<Weight> *const weight = shared_from_this();
  if (allDocsMatch) {
    // all docs have a value and all points are within bounds, so everything
    // matches
    return make_shared<ScorerSupplierAnonymousInnerClass>(shared_from_this(),
                                                          reader, weight);
  } else {
    return make_shared<ScorerSupplierAnonymousInnerClass2>(
        shared_from_this(), reader, values, weight);
  }
}

PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::ScorerSupplierAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReader> reader,
        shared_ptr<org::apache::lucene::search::Weight> weight)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->weight = weight;
}

shared_ptr<Scorer> PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::get(int64_t leadCost)
{
  return make_shared<ConstantScoreScorer>(
      weight, score(), DocIdSetIterator::all(reader->maxDoc()));
}

int64_t PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::cost()
{
  return reader->maxDoc();
}

PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass2::ScorerSupplierAnonymousInnerClass2(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReader> reader, shared_ptr<PointValues> values,
        shared_ptr<org::apache::lucene::search::Weight> weight)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->values = values;
  this->weight = weight;
  result = make_shared<DocIdSetBuilder>(reader->maxDoc(), values,
                                        outerInstance->outerInstance.field);
  visitor = getIntersectVisitor(result);
  cost = -1;
}

shared_ptr<Scorer> PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass2::get(int64_t leadCost) throw(
        IOException)
{
  if (values->getDocCount() == reader->maxDoc() &&
      values->getDocCount() == values->size() &&
      cost() > reader->maxDoc() / 2) {
    // If all docs have exactly one value and the cost is greater
    // than half the leaf size then maybe we can make things faster
    // by computing the set of documents that do NOT match the range
    shared_ptr<FixedBitSet> *const result =
        make_shared<FixedBitSet>(reader->maxDoc());
    result->set(0, reader->maxDoc());
    std::deque<int> cost = {reader->maxDoc()};
    values->intersect(getInverseIntersectVisitor(result, cost));
    shared_ptr<DocIdSetIterator> *const iterator =
        make_shared<BitSetIterator>(result, cost[0]);
    return make_shared<ConstantScoreScorer>(weight, score(), iterator);
  }

  values->intersect(visitor);
  shared_ptr<DocIdSetIterator> iterator = result::build().begin();
  return make_shared<ConstantScoreScorer>(weight, score(), iterator);
}

int64_t PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass2::cost()
{
  if (cost == -1) {
    // Computing the cost may be expensive, so only do it if necessary
    cost = values->estimatePointCount(visitor);
    assert(cost >= 0);
  }
  return cost;
}

shared_ptr<Scorer>
PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool PointRangeQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

wstring PointRangeQuery::getField() { return field; }

int PointRangeQuery::getNumDims() { return numDims; }

int PointRangeQuery::getBytesPerDim() { return bytesPerDim; }

std::deque<char> PointRangeQuery::getLowerPoint()
{
  return lowerPoint.clone();
}

std::deque<char> PointRangeQuery::getUpperPoint()
{
  return upperPoint.clone();
}

int PointRangeQuery::hashCode()
{
  int hash = classHash();
  hash = 31 * hash + field.hashCode();
  hash = 31 * hash + Arrays::hashCode(lowerPoint);
  hash = 31 * hash + Arrays::hashCode(upperPoint);
  hash = 31 * hash + numDims;
  hash = 31 * hash + Objects::hashCode(bytesPerDim);
  return hash;
}

bool PointRangeQuery::equals(any o)
{
  return sameClassAs(o) && equalsTo(getClass().cast(o));
}

bool PointRangeQuery::equalsTo(shared_ptr<PointRangeQuery> other)
{
  return Objects::equals(field, other->field) && numDims == other->numDims &&
         bytesPerDim == other->bytesPerDim &&
         Arrays::equals(lowerPoint, other->lowerPoint) &&
         Arrays::equals(upperPoint, other->upperPoint);
}

wstring PointRangeQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  if (this->field == field == false) {
    sb->append(this->field);
    sb->append(L':');
  }

  // print ourselves as "range per dimension"
  for (int i = 0; i < numDims; i++) {
    if (i > 0) {
      sb->append(L',');
    }

    int startOffset = bytesPerDim * i;

    sb->append(L'[');
    sb->append(toString(i, Arrays::copyOfRange(lowerPoint, startOffset,
                                               startOffset + bytesPerDim)));
    sb->append(L" TO ");
    sb->append(toString(i, Arrays::copyOfRange(upperPoint, startOffset,
                                               startOffset + bytesPerDim)));
    sb->append(L']');
  }

  return sb->toString();
}
} // namespace org::apache::lucene::search