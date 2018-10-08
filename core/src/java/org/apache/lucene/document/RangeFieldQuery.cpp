using namespace std;

#include "RangeFieldQuery.h"

namespace org::apache::lucene::document
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using ScorerSupplier = org::apache::lucene::search::ScorerSupplier;
using Weight = org::apache::lucene::search::Weight;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;

deque<QueryType> QueryType::valueList;

QueryType::StaticConstructor::StaticConstructor() {}

QueryType::StaticConstructor QueryType::staticConstructor;
int QueryType::nextOrdinal = 0;
QueryType::QueryType(const wstring &name, InnerEnum innerEnum)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

org::apache::lucene::index::PointValues::Relation
RangeFieldQuery::QueryType::compare(std::deque<char> &queryPackedValue,
                                    std::deque<char> &minPackedValue,
                                    std::deque<char> &maxPackedValue,
                                    int numDims, int bytesPerDim)
{
  bool inside = true;
  for (int dim = 0; dim < numDims; ++dim) {
    PointValues::Relation relation =
        compare(queryPackedValue, minPackedValue, maxPackedValue, numDims,
                bytesPerDim, dim);
    if (relation == PointValues::Relation::CELL_OUTSIDE_QUERY) {
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    } else if (relation != PointValues::Relation::CELL_INSIDE_QUERY) {
      inside = false;
    }
  }
  return inside ? PointValues::Relation::CELL_INSIDE_QUERY
                : PointValues::Relation::CELL_CROSSES_QUERY;
}

bool RangeFieldQuery::QueryType::matches(std::deque<char> &queryPackedValue,
                                         std::deque<char> &packedValue,
                                         int numDims, int bytesPerDim)
{
  for (int dim = 0; dim < numDims; ++dim) {
    if (matches(queryPackedValue, packedValue, numDims, bytesPerDim, dim) ==
        false) {
      return false;
    }
  }
  return true;
}

bool QueryType::operator==(const QueryType &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool QueryType::operator!=(const QueryType &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<QueryType> QueryType::values() { return valueList; }

int QueryType::ordinal() { return ordinalValue; }

wstring QueryType::toString() { return nameValue; }

QueryType QueryType::valueOf(const wstring &name)
{
  for (auto enumInstance : QueryType::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

RangeFieldQuery::RangeFieldQuery(const wstring &field,
                                 std::deque<char> &ranges, int const numDims,
                                 QueryType const queryType)
    : field(field), queryType(queryType), numDims(numDims), ranges(ranges),
      bytesPerDim(ranges.size() / (2 * numDims))
{
  checkArgs(field, ranges, numDims);
  if (queryType == nullptr) {
    throw invalid_argument(L"Query type cannot be null");
  }
}

void RangeFieldQuery::checkArgs(const wstring &field, std::deque<char> &ranges,
                                int const numDims)
{
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (numDims > 4) {
    throw invalid_argument(L"dimension size cannot be greater than 4");
  }
  if (ranges.empty() || ranges.empty()) {
    throw invalid_argument(L"encoded ranges cannot be null or empty");
  }
}

void RangeFieldQuery::checkFieldInfo(shared_ptr<FieldInfo> fieldInfo)
{
  if (fieldInfo->getPointDimensionCount() / 2 != numDims) {
    throw invalid_argument(
        L"field=\"" + field + L"\" was indexed with numDims=" +
        to_wstring(fieldInfo->getPointDimensionCount() / 2) +
        L" but this query has numDims=" + to_wstring(numDims));
  }
}

shared_ptr<Weight>
RangeFieldQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                              bool needsScores, float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<RangeFieldQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PointValues::IntersectVisitor>
RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::getIntersectVisitor(
    shared_ptr<DocIdSetBuilder> result)
{
  return make_shared<IntersectVisitorAnonymousInnerClass>(shared_from_this(),
                                                          result);
}

RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::IntersectVisitorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<DocIdSetBuilder> result)
{
  this->outerInstance = outerInstance;
  this->result = result;
}

void RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::grow(int count)
{
  adder = result->grow(count);
}

void RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(int docID) 
{
  adder->add(docID);
}

void RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::visit(
        int docID, std::deque<char> &leaf) 
{
  if (outerInstance->outerInstance.queryType.matches(
          outerInstance->outerInstance.ranges, leaf,
          outerInstance->outerInstance.numDims,
          outerInstance->outerInstance.bytesPerDim)) {
    adder->add(docID);
  }
}

PointValues::Relation RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return outerInstance->outerInstance.queryType.compare(
      outerInstance->outerInstance.ranges, minPackedValue, maxPackedValue,
      outerInstance->outerInstance.numDims,
      outerInstance->outerInstance.bytesPerDim);
}

shared_ptr<ScorerSupplier>
RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<PointValues> values = reader->getPointValues(outerInstance->field);
  if (values == nullptr) {
    // no docs in this segment indexed any ranges
    return nullptr;
  }
  shared_ptr<FieldInfo> fieldInfo =
      reader->getFieldInfos()->fieldInfo(outerInstance->field);
  if (fieldInfo == nullptr) {
    // no docs in this segment indexed this field
    return nullptr;
  }
  outerInstance->checkFieldInfo(fieldInfo);
  bool allDocsMatch = false;
  if (values->getDocCount() == reader->maxDoc() &&
      outerInstance->queryType.compare(
          outerInstance->ranges, values->getMinPackedValue(),
          values->getMaxPackedValue(), outerInstance->numDims,
          outerInstance->bytesPerDim) ==
          PointValues::Relation::CELL_INSIDE_QUERY) {
    allDocsMatch = true;
  }

  shared_ptr<Weight> *const weight = shared_from_this();
  if (allDocsMatch) {
    return make_shared<ScorerSupplierAnonymousInnerClass>(shared_from_this(),
                                                          reader, weight);
  } else {
    return make_shared<ScorerSupplierAnonymousInnerClass2>(
        shared_from_this(), reader, values, weight);
  }
}

RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::ScorerSupplierAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReader> reader, shared_ptr<Weight> weight)
{
  this->outerInstance = outerInstance;
  this->reader = reader;
  this->weight = weight;
}

shared_ptr<Scorer> RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::get(int64_t leadCost)
{
  return make_shared<ConstantScoreScorer>(
      weight, score(), DocIdSetIterator::all(reader->maxDoc()));
}

int64_t RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::cost()
{
  return reader->maxDoc();
}

RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass2::ScorerSupplierAnonymousInnerClass2(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReader> reader, shared_ptr<PointValues> values,
        shared_ptr<Weight> weight)
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

shared_ptr<Scorer> RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass2::get(int64_t leadCost) throw(
        IOException)
{
  values->intersect(visitor);
  shared_ptr<DocIdSetIterator> iterator = result::build().begin();
  return make_shared<ConstantScoreScorer>(weight, score(), iterator);
}

int64_t RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::
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
RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool RangeFieldQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

int RangeFieldQuery::hashCode()
{
  int hash = classHash();
  hash = 31 * hash + field.hashCode();
  hash = 31 * hash + numDims;
  hash = 31 * hash + queryType.hashCode();
  hash = 31 * hash + Arrays::hashCode(ranges);

  return hash;
}

bool RangeFieldQuery::equals(any o)
{
  return sameClassAs(o) && equalsTo(getClass().cast(o));
}

bool RangeFieldQuery::equalsTo(shared_ptr<RangeFieldQuery> other)
{
  return Objects::equals(field, other->field) && numDims == other->numDims &&
         Arrays::equals(ranges, other->ranges) && other->queryType == queryType;
}

wstring RangeFieldQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  if (this->field == field == false) {
    sb->append(this->field);
    sb->append(L':');
  }
  sb->append(L"<ranges:");
  sb->append(toString(ranges, 0));
  for (int d = 1; d < numDims; ++d) {
    sb->append(L' ');
    sb->append(toString(ranges, d));
  }
  sb->append(L'>');

  return sb->toString();
}
} // namespace org::apache::lucene::document