using namespace std;

#include "JoinUtil.h"

namespace org::apache::lucene::search::join
{
using DoublePoint = org::apache::lucene::document::DoublePoint;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Collector = org::apache::lucene::search::Collector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using org::apache::lucene::search::join::DocValuesTermsCollector::Function;
using BytesRef = org::apache::lucene::util::BytesRef;

JoinUtil::JoinUtil() {}

shared_ptr<Query>
JoinUtil::createJoinQuery(const wstring &fromField,
                          bool multipleValuesPerDocument,
                          const wstring &toField, shared_ptr<Query> fromQuery,
                          shared_ptr<IndexSearcher> fromSearcher,
                          ScoreMode scoreMode) 
{

  shared_ptr<GenericTermsCollector> *const termsWithScoreCollector;

  if (multipleValuesPerDocument) {
    Function<std::shared_ptr<SortedSetDocValues>> mvFunction =
        DocValuesTermsCollector::sortedSetDocValues(fromField);
    termsWithScoreCollector =
        GenericTermsCollector::createCollectorMV(mvFunction, scoreMode);
  } else {
    Function<std::shared_ptr<BinaryDocValues>> svFunction =
        DocValuesTermsCollector::binaryDocValues(fromField);
    termsWithScoreCollector =
        GenericTermsCollector::createCollectorSV(svFunction, scoreMode);
  }

  return createJoinQuery(multipleValuesPerDocument, toField, fromQuery,
                         fromField, fromSearcher, scoreMode,
                         termsWithScoreCollector);
}

shared_ptr<Query> JoinUtil::createJoinQuery(
    const wstring &fromField, bool multipleValuesPerDocument,
    const wstring &toField, type_info numericType, shared_ptr<Query> fromQuery,
    shared_ptr<IndexSearcher> fromSearcher,
    ScoreMode scoreMode) 
{
  set<int64_t> joinValues = set<int64_t>();
  unordered_map<int64_t, float> aggregatedScores =
      unordered_map<int64_t, float>();
  unordered_map<int64_t, int> occurrences = unordered_map<int64_t, int>();
  bool needsScore = scoreMode != ScoreMode::None;
  function<void(int64_t, float)> scoreAggregator;
  if (scoreMode == ScoreMode::Max) {
    scoreAggregator = [&](key, score) {
      optional<float> currentValue = aggregatedScores.putIfAbsent(key, score);
      if (currentValue != nullptr) {
        aggregatedScores.put(key, max(currentValue, score));
      }
    };
  } else if (scoreMode == ScoreMode::Min) {
    scoreAggregator = [&](key, score) {
      optional<float> currentValue = aggregatedScores.putIfAbsent(key, score);
      if (currentValue != nullptr) {
        aggregatedScores.put(key, min(currentValue, score));
      }
    };
  } else if (scoreMode == ScoreMode::Total) {
    scoreAggregator = [&](key, score) {
      optional<float> currentValue = aggregatedScores.putIfAbsent(key, score);
      if (currentValue != nullptr) {
        aggregatedScores.put(key, currentValue + score);
      }
    };
  } else if (scoreMode == ScoreMode::Avg) {
    scoreAggregator = [&](key, score) {
      optional<float> currentSore = aggregatedScores.putIfAbsent(key, score);
      if (currentSore != nullptr) {
        aggregatedScores.put(key, currentSore + score);
      }
      optional<int> currentOccurrence = occurrences.putIfAbsent(key, 1);
      if (currentOccurrence != nullptr) {
        occurrences.put(key, ++currentOccurrence);
      }
    };
  } else {
    scoreAggregator = [&](key, score) {
      throw make_shared<UnsupportedOperationException>();
    };
  }

  function<float(int64_t)> joinScorer;
  if (scoreMode == ScoreMode::Avg) {
    joinScorer = [&](joinValue) {
      optional<float> aggregatedScore = aggregatedScores.get(joinValue);
      optional<int> occurrence = occurrences.get(joinValue);
      return aggregatedScore / occurrence;
    };
  } else {
    joinScorer = aggregatedScores::get;
  }

  shared_ptr<Collector> collector;
  if (multipleValuesPerDocument) {
    collector = make_shared<SimpleCollectorAnonymousInnerClass>(
        fromField, joinValues, needsScore, scoreAggregator);
  } else {
    collector = make_shared<SimpleCollectorAnonymousInnerClass2>(
        fromField, joinValues, needsScore, scoreAggregator);
  }
  fromSearcher->search(fromQuery, collector);

  set<int64_t>::const_iterator iterator = joinValues.begin();

  constexpr int bytesPerDim;
  shared_ptr<BytesRef> *const encoded = make_shared<BytesRef>();
  shared_ptr<PointInSetIncludingScoreQuery::Stream> *const stream;
  if (Integer::typeid->equals(numericType)) {
    bytesPerDim = Integer::BYTES;
    stream = make_shared<StreamAnonymousInnerClass>(needsScore, joinScorer,
                                                    iterator, encoded);
  } else if (Long::typeid->equals(numericType)) {
    bytesPerDim = Long::BYTES;
    stream = make_shared<StreamAnonymousInnerClass2>(needsScore, joinScorer,
                                                     iterator, encoded);
  } else if (Float::typeid->equals(numericType)) {
    bytesPerDim = Float::BYTES;
    stream = make_shared<StreamAnonymousInnerClass3>(needsScore, joinScorer,
                                                     iterator, encoded);
  } else if (Double::typeid->equals(numericType)) {
    bytesPerDim = Double::BYTES;
    stream = make_shared<StreamAnonymousInnerClass4>(needsScore, joinScorer,
                                                     iterator, encoded);
  } else {
    throw invalid_argument(L"unsupported numeric type, only Integer, Long, "
                           L"Float and Double are supported");
  }

  encoded->bytes = std::deque<char>(bytesPerDim);
  encoded->length = bytesPerDim;

  if (needsScore) {
    return make_shared<PointInSetIncludingScoreQueryAnonymousInnerClass>(
        scoreMode, fromQuery, multipleValuesPerDocument, toField, numericType);
  } else {
    return make_shared<PointInSetQueryAnonymousInnerClass>(toField,
                                                           numericType);
  }
}

internal shared_ptr<public> JoinUtil::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        const wstring &fromField, set<int64_t> &joinValues, bool needsScore,
        function<void(int64_t, float)> &scoreAggregator)
{
  this->fromField = fromField;
  this->joinValues = joinValues;
  this->needsScore = needsScore;
  this->scoreAggregator = scoreAggregator;
}

void JoinUtil::SimpleCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  if (sortedNumericDocValues::advanceExact(doc)) {
    for (int i = 0; i < sortedNumericDocValues::docValueCount(); i++) {
      int64_t value = sortedNumericDocValues::nextValue();
      joinValues.insert(value);
      if (needsScore) {
        scoreAggregator(value, scorer::score());
      }
    }
  }
}

void JoinUtil::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  sortedNumericDocValues =
      DocValues::getSortedNumeric(context->reader(), fromField);
}

void JoinUtil::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

bool JoinUtil::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return needsScore;
}

internal shared_ptr<public> JoinUtil::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        const wstring &fromField, set<int64_t> &joinValues, bool needsScore,
        function<void(int64_t, float)> &scoreAggregator)
{
  this->fromField = fromField;
  this->joinValues = joinValues;
  this->needsScore = needsScore;
  this->scoreAggregator = scoreAggregator;
}

bool JoinUtil::SimpleCollectorAnonymousInnerClass2::docsInOrder(int docID)
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(L"docs out of order: lastDocID=" +
                                      lastDocID + L" vs docID=" +
                                      to_wstring(docID));
  }
  lastDocID = docID;
  return true;
}

void JoinUtil::SimpleCollectorAnonymousInnerClass2::collect(int doc) throw(
    IOException)
{
  assert(docsInOrder(doc));
  int64_t value = 0;
  if (numericDocValues::advanceExact(doc)) {
    value = numericDocValues::longValue();
  }
  joinValues.insert(value);
  if (needsScore) {
    scoreAggregator(value, scorer::score());
  }
}

void JoinUtil::SimpleCollectorAnonymousInnerClass2::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  numericDocValues = DocValues::getNumeric(context->reader(), fromField);
  lastDocID = -1;
}

void JoinUtil::SimpleCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

bool JoinUtil::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return needsScore;
}

JoinUtil::StreamAnonymousInnerClass::StreamAnonymousInnerClass(
    bool needsScore, function<float(int64_t)> &joinScorer,
    set<int64_t>::const_iterator iterator, shared_ptr<BytesRef> encoded)
{
  this->needsScore = needsScore;
  this->joinScorer = joinScorer;
  this->iterator = iterator;
  this->encoded = encoded;
}

shared_ptr<BytesRef> JoinUtil::StreamAnonymousInnerClass::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (iterator.hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    int64_t value = iterator.next();
    IntPoint::encodeDimension(static_cast<int>(value), encoded->bytes, 0);
    if (needsScore) {
      score = joinScorer(value);
    }
    return encoded;
  } else {
    return nullptr;
  }
}

JoinUtil::StreamAnonymousInnerClass2::StreamAnonymousInnerClass2(
    bool needsScore, function<float(int64_t)> &joinScorer,
    set<int64_t>::const_iterator iterator, shared_ptr<BytesRef> encoded)
{
  this->needsScore = needsScore;
  this->joinScorer = joinScorer;
  this->iterator = iterator;
  this->encoded = encoded;
}

shared_ptr<BytesRef> JoinUtil::StreamAnonymousInnerClass2::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (iterator.hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    int64_t value = iterator.next();
    LongPoint::encodeDimension(value, encoded->bytes, 0);
    if (needsScore) {
      score = joinScorer(value);
    }
    return encoded;
  } else {
    return nullptr;
  }
}

JoinUtil::StreamAnonymousInnerClass3::StreamAnonymousInnerClass3(
    bool needsScore, function<float(int64_t)> &joinScorer,
    set<int64_t>::const_iterator iterator, shared_ptr<BytesRef> encoded)
{
  this->needsScore = needsScore;
  this->joinScorer = joinScorer;
  this->iterator = iterator;
  this->encoded = encoded;
}

shared_ptr<BytesRef> JoinUtil::StreamAnonymousInnerClass3::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (iterator.hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    int64_t value = iterator.next();
    FloatPoint::encodeDimension(Float::intBitsToFloat(static_cast<int>(value)),
                                encoded->bytes, 0);
    if (needsScore) {
      score = joinScorer(value);
    }
    return encoded;
  } else {
    return nullptr;
  }
}

JoinUtil::StreamAnonymousInnerClass4::StreamAnonymousInnerClass4(
    bool needsScore, function<float(int64_t)> &joinScorer,
    set<int64_t>::const_iterator iterator, shared_ptr<BytesRef> encoded)
{
  this->needsScore = needsScore;
  this->joinScorer = joinScorer;
  this->iterator = iterator;
  this->encoded = encoded;
}

shared_ptr<BytesRef> JoinUtil::StreamAnonymousInnerClass4::next()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (iterator.hasNext()) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    int64_t value = iterator.next();
    DoublePoint::encodeDimension(Double::longBitsToDouble(value),
                                 encoded->bytes, 0);
    if (needsScore) {
      score = joinScorer(value);
    }
    return encoded;
  } else {
    return nullptr;
  }
}

JoinUtil::PointInSetIncludingScoreQueryAnonymousInnerClass::
    PointInSetIncludingScoreQueryAnonymousInnerClass(
        org::apache::lucene::search::join::ScoreMode scoreMode,
        shared_ptr<Query> fromQuery, bool multipleValuesPerDocument,
        const wstring &toField, type_info numericType)
    : PointInSetIncludingScoreQuery(scoreMode, fromQuery,
                                    multipleValuesPerDocument, toField,
                                    bytesPerDim, stream)
{
  this->numericType = numericType;
}

wstring JoinUtil::PointInSetIncludingScoreQueryAnonymousInnerClass::toString(
    std::deque<char> &value)
{
  return toString::apply(value, numericType);
}

JoinUtil::PointInSetQueryAnonymousInnerClass::
    PointInSetQueryAnonymousInnerClass(const wstring &toField,
                                       type_info numericType)
    : org::apache::lucene::search::PointInSetQuery(toField, 1, bytesPerDim,
                                                   stream)
{
  this->numericType = numericType;
}

wstring
JoinUtil::PointInSetQueryAnonymousInnerClass::toString(std::deque<char> &value)
{
  return PointInSetIncludingScoreQuery::toString_::apply(value, numericType);
}

shared_ptr<Query> JoinUtil::createJoinQuery(
    bool multipleValuesPerDocument, const wstring &toField,
    shared_ptr<Query> fromQuery, const wstring &fromField,
    shared_ptr<IndexSearcher> fromSearcher, ScoreMode scoreMode,
    shared_ptr<GenericTermsCollector> collector) 
{

  fromSearcher->search(fromQuery, collector);
  switch (scoreMode) {
  case org::apache::lucene::search::join::ScoreMode::None:
    return make_shared<TermsQuery>(toField, collector->getCollectedTerms(),
                                   fromField, fromQuery,
                                   fromSearcher->getTopReaderContext()->id());
  case org::apache::lucene::search::join::ScoreMode::Total:
  case org::apache::lucene::search::join::ScoreMode::Max:
  case org::apache::lucene::search::join::ScoreMode::Min:
  case org::apache::lucene::search::join::ScoreMode::Avg:
    return make_shared<TermsIncludingScoreQuery>(
        scoreMode, toField, multipleValuesPerDocument,
        collector->getCollectedTerms(), collector->getScoresPerTerm(),
        fromField, fromQuery, fromSearcher->getTopReaderContext()->id());
  default:
    throw invalid_argument(wstring::format(
        Locale::ROOT, L"Score mode %s isn't supported.", scoreMode));
  }
}

shared_ptr<Query> JoinUtil::createJoinQuery(
    const wstring &joinField, shared_ptr<Query> fromQuery,
    shared_ptr<Query> toQuery, shared_ptr<IndexSearcher> searcher,
    ScoreMode scoreMode, shared_ptr<OrdinalMap> ordinalMap) 
{
  return createJoinQuery(joinField, fromQuery, toQuery, searcher, scoreMode,
                         ordinalMap, 0, numeric_limits<int>::max());
}

shared_ptr<Query> JoinUtil::createJoinQuery(const wstring &joinField,
                                            shared_ptr<Query> fromQuery,
                                            shared_ptr<Query> toQuery,
                                            shared_ptr<IndexSearcher> searcher,
                                            ScoreMode scoreMode,
                                            shared_ptr<OrdinalMap> ordinalMap,
                                            int min, int max) 
{
  int numSegments = searcher->getIndexReader()->leaves().size();
  constexpr int64_t valueCount;
  if (numSegments == 0) {
    return make_shared<MatchNoDocsQuery>(
        L"JoinUtil.createJoinQuery with no segments");
  } else if (numSegments == 1) {
    // No need to use the ordinal map_obj, because there is just one segment.
    ordinalMap.reset();
    shared_ptr<LeafReader> leafReader =
        searcher->getIndexReader()->leaves()[0]->reader();
    shared_ptr<SortedDocValues> joinSortedDocValues =
        leafReader->getSortedDocValues(joinField);
    if (joinSortedDocValues != nullptr) {
      valueCount = joinSortedDocValues->getValueCount();
    } else {
      return make_shared<MatchNoDocsQuery>(
          L"JoinUtil.createJoinQuery: no join values");
    }
  } else {
    if (ordinalMap == nullptr) {
      throw invalid_argument(
          L"OrdinalMap is required, because there is more than 1 segment");
    }
    valueCount = ordinalMap->getValueCount();
  }

  shared_ptr<Query> *const rewrittenFromQuery = searcher->rewrite(fromQuery);
  shared_ptr<Query> *const rewrittenToQuery = searcher->rewrite(toQuery);
  shared_ptr<GlobalOrdinalsWithScoreCollector> globalOrdinalsWithScoreCollector;
  switch (scoreMode) {
  case org::apache::lucene::search::join::ScoreMode::Total:
    globalOrdinalsWithScoreCollector =
        make_shared<GlobalOrdinalsWithScoreCollector::Sum>(
            joinField, ordinalMap, valueCount, min, max);
    break;
  case org::apache::lucene::search::join::ScoreMode::Min:
    globalOrdinalsWithScoreCollector =
        make_shared<GlobalOrdinalsWithScoreCollector::Min>(
            joinField, ordinalMap, valueCount, min, max);
    break;
  case org::apache::lucene::search::join::ScoreMode::Max:
    globalOrdinalsWithScoreCollector =
        make_shared<GlobalOrdinalsWithScoreCollector::Max>(
            joinField, ordinalMap, valueCount, min, max);
    break;
  case org::apache::lucene::search::join::ScoreMode::Avg:
    globalOrdinalsWithScoreCollector =
        make_shared<GlobalOrdinalsWithScoreCollector::Avg>(
            joinField, ordinalMap, valueCount, min, max);
    break;
  case org::apache::lucene::search::join::ScoreMode::None:
    if (min <= 0 && max == numeric_limits<int>::max()) {
      shared_ptr<GlobalOrdinalsCollector> globalOrdinalsCollector =
          make_shared<GlobalOrdinalsCollector>(joinField, ordinalMap,
                                               valueCount);
      searcher->search(rewrittenFromQuery, globalOrdinalsCollector);
      return make_shared<GlobalOrdinalsQuery>(
          globalOrdinalsCollector->getCollectorOrdinals(), joinField,
          ordinalMap, rewrittenToQuery, rewrittenFromQuery,
          searcher->getTopReaderContext()->id());
    } else {
      globalOrdinalsWithScoreCollector =
          make_shared<GlobalOrdinalsWithScoreCollector::NoScore>(
              joinField, ordinalMap, valueCount, min, max);
      break;
    }
  default:
    throw invalid_argument(wstring::format(
        Locale::ROOT, L"Score mode %s isn't supported.", scoreMode));
  }
  searcher->search(rewrittenFromQuery, globalOrdinalsWithScoreCollector);
  return make_shared<GlobalOrdinalsWithScoreQuery>(
      globalOrdinalsWithScoreCollector, scoreMode, joinField, ordinalMap,
      rewrittenToQuery, rewrittenFromQuery, min, max,
      searcher->getTopReaderContext()->id());
}
} // namespace org::apache::lucene::search::join