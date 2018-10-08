using namespace std;

#include "GlobalOrdinalsWithScoreCollector.h"

namespace org::apache::lucene::search::join
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

GlobalOrdinalsWithScoreCollector::GlobalOrdinalsWithScoreCollector(
    const wstring &field, shared_ptr<OrdinalMap> ordinalMap,
    int64_t valueCount, ScoreMode scoreMode, int min, int max)
    : field(field), doMinMax(!(min <= 0 && max == numeric_limits<int>::max())),
      min(min), max(max), ordinalMap(ordinalMap),
      collectedOrds(make_shared<LongBitSet>(valueCount))
{
  if (valueCount > numeric_limits<int>::max()) {
    // We simply don't support more than
    throw make_shared<IllegalStateException>(
        L"Can't collect more than [" + numeric_limits<int>::max() + L"] ids");
  }
  if (scoreMode != ScoreMode::None) {
    this->scores = make_shared<Scores>(valueCount, unset());
  } else {
    this->scores.reset();
  }
  if (scoreMode == ScoreMode::Avg || doMinMax) {
    this->occurrences = make_shared<Occurrences>(valueCount);
  } else {
    this->occurrences.reset();
  }
}

bool GlobalOrdinalsWithScoreCollector::match(int globalOrd)
{
  if (collectedOrds->get(globalOrd)) {
    if (doMinMax) {
      constexpr int occurrence = occurrences->getOccurrence(globalOrd);
      return occurrence >= min && occurrence <= max;
    } else {
      return true;
    }
  }
  return false;
}

float GlobalOrdinalsWithScoreCollector::score(int globalOrdinal)
{
  return scores->getScore(globalOrdinal);
}

shared_ptr<LeafCollector> GlobalOrdinalsWithScoreCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedDocValues> docTermOrds =
      DocValues::getSorted(context->reader(), field);
  if (ordinalMap != nullptr) {
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup =
        ordinalMap->getGlobalOrds(context->ord);
    return make_shared<OrdinalMapCollector>(shared_from_this(), docTermOrds,
                                            segmentOrdToGlobalOrdLookup);
  } else {
    return make_shared<SegmentOrdinalCollector>(shared_from_this(),
                                                docTermOrds);
  }
}

bool GlobalOrdinalsWithScoreCollector::needsScores() { return true; }

GlobalOrdinalsWithScoreCollector::OrdinalMapCollector::OrdinalMapCollector(
    shared_ptr<GlobalOrdinalsWithScoreCollector> outerInstance,
    shared_ptr<SortedDocValues> docTermOrds,
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup)
    : docTermOrds(docTermOrds),
      segmentOrdToGlobalOrdLookup(segmentOrdToGlobalOrdLookup),
      outerInstance(outerInstance)
{
}

void GlobalOrdinalsWithScoreCollector::OrdinalMapCollector::collect(
    int doc) 
{
  if (docTermOrds->advanceExact(doc)) {
    constexpr int globalOrd = static_cast<int>(
        segmentOrdToGlobalOrdLookup->get(docTermOrds->ordValue()));
    outerInstance->collectedOrds->set(globalOrd);
    float existingScore = outerInstance->scores->getScore(globalOrd);
    float newScore = scorer->score();
    outerInstance->doScore(globalOrd, existingScore, newScore);
    if (outerInstance->occurrences != nullptr) {
      outerInstance->occurrences->increment(globalOrd);
    }
  }
}

void GlobalOrdinalsWithScoreCollector::OrdinalMapCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

GlobalOrdinalsWithScoreCollector::SegmentOrdinalCollector::
    SegmentOrdinalCollector(
        shared_ptr<GlobalOrdinalsWithScoreCollector> outerInstance,
        shared_ptr<SortedDocValues> docTermOrds)
    : docTermOrds(docTermOrds), outerInstance(outerInstance)
{
}

void GlobalOrdinalsWithScoreCollector::SegmentOrdinalCollector::collect(
    int doc) 
{
  if (docTermOrds->advanceExact(doc)) {
    int segmentOrd = docTermOrds->ordValue();
    outerInstance->collectedOrds->set(segmentOrd);
    float existingScore = outerInstance->scores->getScore(segmentOrd);
    float newScore = scorer->score();
    outerInstance->doScore(segmentOrd, existingScore, newScore);
    if (outerInstance->occurrences != nullptr) {
      outerInstance->occurrences->increment(segmentOrd);
    }
  }
}

void GlobalOrdinalsWithScoreCollector::SegmentOrdinalCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

GlobalOrdinalsWithScoreCollector::Min::Min(const wstring &field,
                                           shared_ptr<OrdinalMap> ordinalMap,
                                           int64_t valueCount, int min,
                                           int max)
    : GlobalOrdinalsWithScoreCollector(field, ordinalMap, valueCount,
                                       ScoreMode::Min, min, max)
{
}

void GlobalOrdinalsWithScoreCollector::Min::doScore(int globalOrd,
                                                    float existingScore,
                                                    float newScore)
{
  scores->setScore(globalOrd, min(existingScore, newScore));
}

float GlobalOrdinalsWithScoreCollector::Min::unset()
{
  return numeric_limits<float>::infinity();
}

GlobalOrdinalsWithScoreCollector::Max::Max(const wstring &field,
                                           shared_ptr<OrdinalMap> ordinalMap,
                                           int64_t valueCount, int min,
                                           int max)
    : GlobalOrdinalsWithScoreCollector(field, ordinalMap, valueCount,
                                       ScoreMode::Max, min, max)
{
}

void GlobalOrdinalsWithScoreCollector::Max::doScore(int globalOrd,
                                                    float existingScore,
                                                    float newScore)
{
  scores->setScore(globalOrd, max(existingScore, newScore));
}

float GlobalOrdinalsWithScoreCollector::Max::unset()
{
  return -numeric_limits<float>::infinity();
}

GlobalOrdinalsWithScoreCollector::Sum::Sum(const wstring &field,
                                           shared_ptr<OrdinalMap> ordinalMap,
                                           int64_t valueCount, int min,
                                           int max)
    : GlobalOrdinalsWithScoreCollector(field, ordinalMap, valueCount,
                                       ScoreMode::Total, min, max)
{
}

void GlobalOrdinalsWithScoreCollector::Sum::doScore(int globalOrd,
                                                    float existingScore,
                                                    float newScore)
{
  scores->setScore(globalOrd, existingScore + newScore);
}

float GlobalOrdinalsWithScoreCollector::Sum::unset() { return 0.0f; }

GlobalOrdinalsWithScoreCollector::Avg::Avg(const wstring &field,
                                           shared_ptr<OrdinalMap> ordinalMap,
                                           int64_t valueCount, int min,
                                           int max)
    : GlobalOrdinalsWithScoreCollector(field, ordinalMap, valueCount,
                                       ScoreMode::Avg, min, max)
{
}

void GlobalOrdinalsWithScoreCollector::Avg::doScore(int globalOrd,
                                                    float existingScore,
                                                    float newScore)
{
  scores->setScore(globalOrd, existingScore + newScore);
}

float GlobalOrdinalsWithScoreCollector::Avg::score(int globalOrdinal)
{
  return scores->getScore(globalOrdinal) /
         occurrences->getOccurrence(globalOrdinal);
}

float GlobalOrdinalsWithScoreCollector::Avg::unset() { return 0.0f; }

GlobalOrdinalsWithScoreCollector::NoScore::NoScore(
    const wstring &field, shared_ptr<OrdinalMap> ordinalMap,
    int64_t valueCount, int min, int max)
    : GlobalOrdinalsWithScoreCollector(field, ordinalMap, valueCount,
                                       ScoreMode::None, min, max)
{
}

shared_ptr<LeafCollector>
GlobalOrdinalsWithScoreCollector::NoScore::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<SortedDocValues> docTermOrds =
      DocValues::getSorted(context->reader(), field);
  if (ordinalMap != nullptr) {
    shared_ptr<LongValues> segmentOrdToGlobalOrdLookup =
        ordinalMap->getGlobalOrds(context->ord);
    return make_shared<LeafCollectorAnonymousInnerClass>(
        shared_from_this(), docTermOrds, segmentOrdToGlobalOrdLookup);
  } else {
    return make_shared<LeafCollectorAnonymousInnerClass2>(shared_from_this(),
                                                          docTermOrds);
  }
}

GlobalOrdinalsWithScoreCollector::NoScore::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<NoScore> outerInstance,
        shared_ptr<SortedDocValues> docTermOrds,
        shared_ptr<LongValues> segmentOrdToGlobalOrdLookup)
{
  this->outerInstance = outerInstance;
  this->docTermOrds = docTermOrds;
  this->segmentOrdToGlobalOrdLookup = segmentOrdToGlobalOrdLookup;
}

void GlobalOrdinalsWithScoreCollector::NoScore::
    LeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
}

void GlobalOrdinalsWithScoreCollector::NoScore::
    LeafCollectorAnonymousInnerClass::collect(int doc) 
{
  if (docTermOrds->advanceExact(doc)) {
    constexpr int globalOrd = static_cast<int>(
        segmentOrdToGlobalOrdLookup->get(docTermOrds->ordValue()));
    outerInstance->collectedOrds->set(globalOrd);
    outerInstance->occurrences->increment(globalOrd);
  }
}

GlobalOrdinalsWithScoreCollector::NoScore::LeafCollectorAnonymousInnerClass2::
    LeafCollectorAnonymousInnerClass2(shared_ptr<NoScore> outerInstance,
                                      shared_ptr<SortedDocValues> docTermOrds)
{
  this->outerInstance = outerInstance;
  this->docTermOrds = docTermOrds;
}

void GlobalOrdinalsWithScoreCollector::NoScore::
    LeafCollectorAnonymousInnerClass2::setScorer(
        shared_ptr<Scorer> scorer) 
{
}

void GlobalOrdinalsWithScoreCollector::NoScore::
    LeafCollectorAnonymousInnerClass2::collect(int doc) 
{
  if (docTermOrds->advanceExact(doc)) {
    int segmentOrd = docTermOrds->ordValue();
    outerInstance->collectedOrds->set(segmentOrd);
    outerInstance->occurrences->increment(segmentOrd);
  }
}

void GlobalOrdinalsWithScoreCollector::NoScore::doScore(int globalOrd,
                                                        float existingScore,
                                                        float newScore)
{
}

float GlobalOrdinalsWithScoreCollector::NoScore::score(int globalOrdinal)
{
  return 1.0f;
}

float GlobalOrdinalsWithScoreCollector::NoScore::unset() { return 0.0f; }

bool GlobalOrdinalsWithScoreCollector::NoScore::needsScores() { return false; }

GlobalOrdinalsWithScoreCollector::Scores::Scores(int64_t valueCount,
                                                 float unset)
    : blocks(std::deque<std::deque<float>>(
          static_cast<int>((blockSize) / arraySize))),
      unset(unset)
{
  int64_t blockSize = valueCount + arraySize - 1;
}

void GlobalOrdinalsWithScoreCollector::Scores::setScore(int globalOrdinal,
                                                        float score)
{
  int block = globalOrdinal / arraySize;
  int offset = globalOrdinal % arraySize;
  std::deque<float> scores = blocks[block];
  if (scores.empty()) {
    blocks[block] = scores = std::deque<float>(arraySize);
    if (unset != 0.0f) {
      Arrays::fill(scores, unset);
    }
  }
  scores[offset] = score;
}

float GlobalOrdinalsWithScoreCollector::Scores::getScore(int globalOrdinal)
{
  int block = globalOrdinal / arraySize;
  int offset = globalOrdinal % arraySize;
  std::deque<float> scores = blocks[block];
  float score;
  if (scores.size() > 0) {
    score = scores[offset];
  } else {
    score = unset;
  }
  return score;
}

GlobalOrdinalsWithScoreCollector::Occurrences::Occurrences(int64_t valueCount)
    : blocks(std::deque<std::deque<int>>(
          static_cast<int>(blockSize / arraySize)))
{
  int64_t blockSize = valueCount + arraySize - 1;
}

void GlobalOrdinalsWithScoreCollector::Occurrences::increment(int globalOrdinal)
{
  int block = globalOrdinal / arraySize;
  int offset = globalOrdinal % arraySize;
  std::deque<int> occurrences = blocks[block];
  if (occurrences.empty()) {
    blocks[block] = occurrences = std::deque<int>(arraySize);
  }
  occurrences[offset]++;
}

int GlobalOrdinalsWithScoreCollector::Occurrences::getOccurrence(
    int globalOrdinal)
{
  int block = globalOrdinal / arraySize;
  int offset = globalOrdinal % arraySize;
  std::deque<int> occurrences = blocks[block];
  return occurrences[offset];
}
} // namespace org::apache::lucene::search::join