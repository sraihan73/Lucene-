using namespace std;

#include "PointInSetIncludingScoreQuery.h"

namespace org::apache::lucene::search::join
{
using DoublePoint = org::apache::lucene::document::DoublePoint;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
function<wstring *(char[], type_info *)>
    PointInSetIncludingScoreQuery::toString = [&](value, numericType) {
      if (Integer::typeid->equals(numericType)) {
        return Integer::toString(IntPoint::decodeDimension(value, 0));
      } else if (Long::typeid->equals(numericType)) {
        return Long::toString(LongPoint::decodeDimension(value, 0));
      } else if (Float::typeid->equals(numericType)) {
        return Float::toString(FloatPoint::decodeDimension(value, 0));
      } else if (Double::typeid->equals(numericType)) {
        return Double::toString(DoublePoint::decodeDimension(value, 0));
      } else {
        return L"unsupported";
      }
    };

PointInSetIncludingScoreQuery::PointInSetIncludingScoreQuery(
    ScoreMode scoreMode, shared_ptr<Query> originalQuery,
    bool multipleValuesPerDocument, const wstring &field, int bytesPerDim,
    shared_ptr<PointInSetQuery::Stream> packedPoints)
    : scoreMode(scoreMode), originalQuery(originalQuery),
      multipleValuesPerDocument(multipleValuesPerDocument),
      sortedPackedPoints(builder::finish()),
      sortedPackedPointsHashCode(sortedPackedPoints->hashCode()), field(field),
      bytesPerDim(bytesPerDim), aggregatedJoinScores(deque<>())
{
  if (bytesPerDim < 1 || bytesPerDim > PointValues::MAX_NUM_BYTES) {
    throw invalid_argument(L"bytesPerDim must be > 0 and <= " +
                           to_wstring(PointValues::MAX_NUM_BYTES) + L"; got " +
                           to_wstring(bytesPerDim));
  }

  shared_ptr<PrefixCodedTerms::Builder> builder =
      make_shared<PrefixCodedTerms::Builder>();
  shared_ptr<BytesRefBuilder> previous = nullptr;
  shared_ptr<BytesRef> current;
  while ((current = packedPoints->next()) != nullptr) {
    if (current->length != bytesPerDim) {
      throw invalid_argument(
          L"packed point length should be " + to_wstring(bytesPerDim) +
          L" but got " + to_wstring(current->length) + L"; field=\"" + field +
          L"\"bytesPerDim=" + to_wstring(bytesPerDim));
    }
    if (previous == nullptr) {
      previous = make_shared<BytesRefBuilder>();
    } else {
      int cmp = previous->get()->compareTo(current);
      if (cmp == 0) {
        throw invalid_argument(L"unexpected duplicated value: " + current);
      } else if (cmp >= 0) {
        throw invalid_argument(L"values are out of order: saw " + previous +
                               L" before " + current);
      }
    }
    builder->add(field, current);
    aggregatedJoinScores.push_back(packedPoints->score);
    previous->copyBytes(current);
  }
}

shared_ptr<Weight>
PointInSetIncludingScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                            bool needsScores,
                                            float boost) 
{
  return make_shared<WeightAnonymousInnerClass>(shared_from_this());
}

PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::
    WeightAnonymousInnerClass(
        shared_ptr<PointInSetIncludingScoreQuery> outerInstance)
    : org::apache::lucene::search::Weight(outerInstance)
{
  this->outerInstance = outerInstance;
}

void PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation>
PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<Scorer> scorer = scorer(context);
  if (scorer != nullptr) {
    int target = scorer->begin().advance(doc);
    if (doc == target) {
      return Explanation::match(scorer->score(), L"A match");
    }
  }
  return Explanation::noMatch(L"Not a match");
}

shared_ptr<Scorer>
PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<FieldInfo> fieldInfo =
      reader->getFieldInfos()->fieldInfo(outerInstance->field);
  if (fieldInfo == nullptr) {
    return nullptr;
  }
  if (fieldInfo->getPointDimensionCount() != 1) {
    throw invalid_argument(L"field=\"" + outerInstance->field +
                           L"\" was indexed with numDims=" +
                           to_wstring(fieldInfo->getPointDimensionCount()) +
                           L" but this query has numDims=1");
  }
  if (fieldInfo->getPointNumBytes() != outerInstance->bytesPerDim) {
    throw invalid_argument(L"field=\"" + outerInstance->field +
                           L"\" was indexed with bytesPerDim=" +
                           to_wstring(fieldInfo->getPointNumBytes()) +
                           L" but this query has bytesPerDim=" +
                           to_wstring(outerInstance->bytesPerDim));
  }
  shared_ptr<PointValues> values = reader->getPointValues(outerInstance->field);
  if (values == nullptr) {
    return nullptr;
  }

  shared_ptr<FixedBitSet> result = make_shared<FixedBitSet>(reader->maxDoc());
  std::deque<float> scores(reader->maxDoc());
  values->intersect(make_shared<PointInSetQuery::MergePointVisitor>(
      outerInstance->sortedPackedPoints, result, scores));
  return make_shared<ScorerAnonymousInnerClass>(shared_from_this(), result,
                                                scores);
}

PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::ScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<FixedBitSet> result, deque<float> &scores)
    : org::apache::lucene::search::Scorer(outerInstance)
{
  this->outerInstance = outerInstance;
  this->result = result;
  this->scores = scores;
  disi = make_shared<BitSetIterator>(result, 10LL);
}

float PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::score() 
{
  return scores[docID()];
}

int PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::
    ScorerAnonymousInnerClass::docID()
{
  return disi::docID();
}

shared_ptr<DocIdSetIterator> PointInSetIncludingScoreQuery::
    WeightAnonymousInnerClass::ScorerAnonymousInnerClass::iterator()
{
  return disi;
}

bool PointInSetIncludingScoreQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

PointInSetIncludingScoreQuery::MergePointVisitor::MergePointVisitor(
    shared_ptr<PointInSetIncludingScoreQuery> outerInstance,
    shared_ptr<PrefixCodedTerms> sortedPackedPoints,
    shared_ptr<FixedBitSet> result,
    std::deque<float> &scores) 
    : result(result), scores(scores), outerInstance(outerInstance)
{
  scratch->length = outerInstance->bytesPerDim;
  this->iterator = sortedPackedPoints->begin();
  this->scoreIterator = outerInstance->aggregatedJoinScores.begin();
  nextQueryPoint = iterator->next();
  if (scoreIterator->hasNext()) {
    nextScore = scoreIterator->next();
  }
}

void PointInSetIncludingScoreQuery::MergePointVisitor::visit(int docID)
{
  throw make_shared<IllegalStateException>(
      L"shouldn't get here, since CELL_INSIDE_QUERY isn't emitted");
}

void PointInSetIncludingScoreQuery::MergePointVisitor::visit(
    int docID, std::deque<char> &packedValue)
{
  scratch->bytes = packedValue;
  while (nextQueryPoint != nullptr) {
    int cmp = nextQueryPoint->compareTo(scratch);
    if (cmp == 0) {
      // Query point equals index point, so collect and return
      if (outerInstance->multipleValuesPerDocument) {
        if (result->get(docID) == false) {
          result->set(docID);
          scores[docID] = nextScore;
        }
      } else {
        result->set(docID);
        scores[docID] = nextScore;
      }
      break;
    } else if (cmp < 0) {
      // Query point is before index point, so we move to next query point
      nextQueryPoint = iterator->next();
      if (scoreIterator->hasNext()) {
        nextScore = scoreIterator->next();
      }
    } else {
      // Query point is after index point, so we don't collect and we return:
      break;
    }
  }
}

PointValues::Relation PointInSetIncludingScoreQuery::MergePointVisitor::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  while (nextQueryPoint != nullptr) {
    scratch->bytes = minPackedValue;
    int cmpMin = nextQueryPoint->compareTo(scratch);
    if (cmpMin < 0) {
      // query point is before the start of this cell
      nextQueryPoint = iterator->next();
      if (scoreIterator->hasNext()) {
        nextScore = scoreIterator->next();
      }
      continue;
    }
    scratch->bytes = maxPackedValue;
    int cmpMax = nextQueryPoint->compareTo(scratch);
    if (cmpMax > 0) {
      // query point is after the end of this cell
      return PointValues::Relation::CELL_OUTSIDE_QUERY;
    }

    return PointValues::Relation::CELL_CROSSES_QUERY;
  }

  // We exhausted all points in the query:
  return PointValues::Relation::CELL_OUTSIDE_QUERY;
}

int PointInSetIncludingScoreQuery::hashCode()
{
  int hash = classHash();
  hash = 31 * hash + scoreMode.hashCode();
  hash = 31 * hash + field.hashCode();
  hash = 31 * hash + originalQuery->hashCode();
  hash = 31 * hash + sortedPackedPointsHashCode;
  hash = 31 * hash + bytesPerDim;
  return hash;
}

bool PointInSetIncludingScoreQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool PointInSetIncludingScoreQuery::equalsTo(
    shared_ptr<PointInSetIncludingScoreQuery> other)
{
  return other->scoreMode.equals(scoreMode) && other->field == field &&
         other->originalQuery->equals(originalQuery) &&
         other->bytesPerDim == bytesPerDim &&
         other->sortedPackedPointsHashCode == sortedPackedPointsHashCode &&
         other->sortedPackedPoints->equals(sortedPackedPoints);
}

wstring PointInSetIncludingScoreQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
  if (this->field == field == false) {
    sb->append(this->field);
    sb->append(L':');
  }

  sb->append(L"{");

  shared_ptr<PrefixCodedTerms::TermIterator> iterator =
      sortedPackedPoints->begin();
  std::deque<char> pointBytes(bytesPerDim);
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
} // namespace org::apache::lucene::search::join