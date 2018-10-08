using namespace std;

#include "PointInSetQuery.h"

namespace org::apache::lucene::search
{
using IntPoint = org::apache::lucene::document::IntPoint;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;

PointInSetQuery::PointInSetQuery(const wstring &field, int numDims,
                                 int bytesPerDim,
                                 shared_ptr<Stream> packedPoints)
    : sortedPackedPoints(builder::finish()),
      sortedPackedPointsHashCode(sortedPackedPoints->hashCode()), field(field),
      numDims(numDims), bytesPerDim(bytesPerDim)
{
  if (bytesPerDim < 1 || bytesPerDim > PointValues::MAX_NUM_BYTES) {
    throw invalid_argument(L"bytesPerDim must be > 0 and <= " +
                           to_wstring(PointValues::MAX_NUM_BYTES) + L"; got " +
                           to_wstring(bytesPerDim));
  }
  if (numDims < 1 || numDims > PointValues::MAX_DIMENSIONS) {
    throw invalid_argument(L"numDims must be > 0 and <= " +
                           to_wstring(PointValues::MAX_DIMENSIONS) + L"; got " +
                           to_wstring(numDims));
  }

  // In the 1D case this works well (the more points, the more common prefixes
  // they share, typically), but in the > 1 D case, where we are only looking at
  // the first dimension's prefix bytes, it can at worst not hurt:
  shared_ptr<PrefixCodedTerms::Builder> builder =
      make_shared<PrefixCodedTerms::Builder>();
  shared_ptr<BytesRefBuilder> previous = nullptr;
  shared_ptr<BytesRef> current;
  while ((current = packedPoints->next()) != nullptr) {
    if (current->length != numDims * bytesPerDim) {
      throw invalid_argument(L"packed point length should be " +
                             to_wstring(numDims * bytesPerDim) + L" but got " +
                             to_wstring(current->length) + L"; field=\"" +
                             field + L"\" numDims=" + to_wstring(numDims) +
                             L" bytesPerDim=" + to_wstring(bytesPerDim));
    }
    if (previous == nullptr) {
      previous = make_shared<BytesRefBuilder>();
    } else {
      int cmp = previous->get()->compareTo(current);
      if (cmp == 0) {
        continue; // deduplicate
      } else if (cmp > 0) {
        throw invalid_argument(L"values are out of order: saw " + previous +
                               L" before " + current);
      }
    }
    builder->add(field, current);
    previous->copyBytes(current);
  }
}

shared_ptr<Weight>
PointInSetQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                              bool needsScores, float boost) 
{

  // We don't use RandomAccessWeight here: it's no good to approximate with
  // "match all docs". This is an inverted structure and should be used in the
  // first pass:

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

PointInSetQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<PointInSetQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
PointInSetQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
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
  if (values->getBytesPerDimension() != outerInstance->bytesPerDim) {
    throw invalid_argument(L"field=\"" + outerInstance->field +
                           L"\" was indexed with bytesPerDim=" +
                           to_wstring(values->getBytesPerDimension()) +
                           L" but this query has bytesPerDim=" +
                           to_wstring(outerInstance->bytesPerDim));
  }

  shared_ptr<DocIdSetBuilder> result = make_shared<DocIdSetBuilder>(
      reader->maxDoc(), values, outerInstance->field);

  if (outerInstance->numDims == 1) {

    // We optimize this common case, effectively doing a merge sort of the
    // indexed values vs the queried set:
    values->intersect(make_shared<MergePointVisitor>(
        outerInstance, outerInstance->sortedPackedPoints, result));

  } else {
    // NOTE: this is naive implementation, where for each point we re-walk the
    // KD tree to intersect.  We could instead do a similar optimization as the
    // 1D case, but I think it'd mean building a query-time KD tree so we could
    // efficiently intersect against the index, which is probably tricky!
    shared_ptr<SinglePointVisitor> visitor =
        make_shared<SinglePointVisitor>(outerInstance, result);
    shared_ptr<PrefixCodedTerms::TermIterator> iterator =
        outerInstance->sortedPackedPoints->begin();
    for (shared_ptr<BytesRef> point = iterator->next(); point != nullptr;
         point = iterator->next()) {
      visitor->setPoint(point);
      values->intersect(visitor);
    }
  }

  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          result->build()->begin());
}

bool PointInSetQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

PointInSetQuery::MergePointVisitor::MergePointVisitor(
    shared_ptr<PointInSetQuery> outerInstance,
    shared_ptr<PrefixCodedTerms> sortedPackedPoints,
    shared_ptr<DocIdSetBuilder> result) 
    : result(result), sortedPackedPoints(sortedPackedPoints),
      outerInstance(outerInstance)
{
  scratch->length = outerInstance->bytesPerDim;
  this->iterator = this->sortedPackedPoints->begin();
  nextQueryPoint = iterator->next();
}

void PointInSetQuery::MergePointVisitor::grow(int count)
{
  adder = result->grow(count);
}

void PointInSetQuery::MergePointVisitor::visit(int docID) { adder->add(docID); }

void PointInSetQuery::MergePointVisitor::visit(int docID,
                                               std::deque<char> &packedValue)
{
  scratch->bytes = packedValue;
  while (nextQueryPoint != nullptr) {
    int cmp = nextQueryPoint->compareTo(scratch);
    if (cmp == 0) {
      // Query point equals index point, so collect and return
      adder->add(docID);
      break;
    } else if (cmp < 0) {
      // Query point is before index point, so we move to next query point
      nextQueryPoint = iterator->next();
    } else {
      // Query point is after index point, so we don't collect and we return:
      break;
    }
  }
}

PointValues::Relation
PointInSetQuery::MergePointVisitor::compare(std::deque<char> &minPackedValue,
                                            std::deque<char> &maxPackedValue)
{
  while (nextQueryPoint != nullptr) {
    scratch->bytes = minPackedValue;
    int cmpMin = nextQueryPoint->compareTo(scratch);
    if (cmpMin < 0) {
      // query point is before the start of this cell
      nextQueryPoint = iterator->next();
      continue;
    }
    scratch->bytes = maxPackedValue;
    int cmpMax = nextQueryPoint->compareTo(scratch);
    if (cmpMax > 0) {
      // query point is after the end of this cell
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }

    if (cmpMin == 0 && cmpMax == 0) {
      // NOTE: we only hit this if we are on a cell whose min and max values are
      // exactly equal to our point, which can easily happen if many (> 1024)
      // docs share this one value
      return PointValues::Relation::CELL_INSIDE_QUERY;
    } else {
      return PointValues::Relation::CELL_CROSSES_QUERY;
    }
  }

  // We exhausted all points in the query:
  return PointValues::Relation::CELL_OUTSIDE_QUERY;
}

PointInSetQuery::SinglePointVisitor::SinglePointVisitor(
    shared_ptr<PointInSetQuery> outerInstance,
    shared_ptr<DocIdSetBuilder> result)
    : result(result), pointBytes(std::deque<char>(outerInstance->bytesPerDim *
                                                   outerInstance->numDims)),
      outerInstance(outerInstance)
{
}

void PointInSetQuery::SinglePointVisitor::setPoint(shared_ptr<BytesRef> point)
{
  // we verified this up front in query's ctor:
  assert(point->length == pointBytes.size());
  System::arraycopy(point->bytes, point->offset, pointBytes, 0,
                    pointBytes.size());
}

void PointInSetQuery::SinglePointVisitor::grow(int count)
{
  adder = result->grow(count);
}

void PointInSetQuery::SinglePointVisitor::visit(int docID)
{
  adder->add(docID);
}

void PointInSetQuery::SinglePointVisitor::visit(int docID,
                                                std::deque<char> &packedValue)
{
  assert(packedValue.size() == pointBytes.size());
  if (Arrays::equals(packedValue, pointBytes)) {
    // The point for this doc matches the point we are querying on
    adder->add(docID);
  }
}

PointValues::Relation
PointInSetQuery::SinglePointVisitor::compare(std::deque<char> &minPackedValue,
                                             std::deque<char> &maxPackedValue)
{

  bool crosses = false;

  for (int dim = 0; dim < outerInstance->numDims; dim++) {
    int offset = dim * outerInstance->bytesPerDim;

    int cmpMin = StringHelper::compare(
        outerInstance->bytesPerDim, minPackedValue, offset, pointBytes, offset);
    if (cmpMin > 0) {
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }

    int cmpMax = StringHelper::compare(
        outerInstance->bytesPerDim, maxPackedValue, offset, pointBytes, offset);
    if (cmpMax < 0) {
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }

    if (cmpMin != 0 || cmpMax != 0) {
      crosses = true;
    }
  }

  if (crosses) {
    return PointValues::Relation::CELL_CROSSES_QUERY;
  } else {
    // NOTE: we only hit this if we are on a cell whose min and max values are
    // exactly equal to our point, which can easily happen if many docs share
    // this one value
    return PointValues::Relation::CELL_INSIDE_QUERY;
  }
}

shared_ptr<deque<std::deque<char>>> PointInSetQuery::getPackedPoints()
{
  return make_shared<AbstractCollectionAnonymousInnerClass>(shared_from_this());
}

PointInSetQuery::AbstractCollectionAnonymousInnerClass::
    AbstractCollectionAnonymousInnerClass(
        shared_ptr<PointInSetQuery> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Iterator<std::deque<char>>>
PointInSetQuery::AbstractCollectionAnonymousInnerClass::iterator()
{
  int size = static_cast<int>(outerInstance->sortedPackedPoints->size());
  shared_ptr<PrefixCodedTerms::TermIterator> iterator =
      outerInstance->sortedPackedPoints->begin();
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this(), size,
                                                  iterator);
}

PointInSetQuery::AbstractCollectionAnonymousInnerClass::
    IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<AbstractCollectionAnonymousInnerClass> outerInstance,
        int size, shared_ptr<PrefixCodedTerms::TermIterator> iterator)
{
  this->outerInstance = outerInstance;
  this->size = size;
  this->iterator = iterator;
  upto = 0;
}

bool PointInSetQuery::AbstractCollectionAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  return upto < size;
}

std::deque<char> PointInSetQuery::AbstractCollectionAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  if (upto == size) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }

  upto++;
  shared_ptr<BytesRef> next = iterator->next();
  return Arrays::copyOfRange(next->bytes, next->offset, next->length);
}

int PointInSetQuery::AbstractCollectionAnonymousInnerClass::size()
{
  return static_cast<int>(outerInstance->sortedPackedPoints->size());
}

wstring PointInSetQuery::getField() { return field; }

int PointInSetQuery::getNumDims() { return numDims; }

int PointInSetQuery::getBytesPerDim() { return bytesPerDim; }

int PointInSetQuery::hashCode()
{
  int hash = classHash();
  hash = 31 * hash + field.hashCode();
  hash = 31 * hash + sortedPackedPointsHashCode;
  hash = 31 * hash + numDims;
  hash = 31 * hash + bytesPerDim;
  return hash;
}

bool PointInSetQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool PointInSetQuery::equalsTo(shared_ptr<PointInSetQuery> other)
{
  return other->field == field && other->numDims == numDims &&
         other->bytesPerDim == bytesPerDim &&
         other->sortedPackedPointsHashCode == sortedPackedPointsHashCode &&
         other->sortedPackedPoints->equals(sortedPackedPoints);
}

wstring PointInSetQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  if (this->field == field == false) {
    sb->append(this->field);
    sb->append(L':');
  }

  sb->append(L"{");

  shared_ptr<PrefixCodedTerms::TermIterator> iterator =
      sortedPackedPoints->begin();
  std::deque<char> pointBytes(numDims * bytesPerDim);
  bool first = true;
  for (shared_ptr<BytesRef> point = iterator->next(); point != nullptr;
       point = iterator->next()) {
    if (first == false) {
      sb->append(L" ");
    }
    first = false;
    System::arraycopy(point->bytes, point->offset, pointBytes, 0,
                      pointBytes.size());
    sb->append(toString(pointBytes));
  }
  sb->append(L"}");
  return sb->toString();
}
} // namespace org::apache::lucene::search