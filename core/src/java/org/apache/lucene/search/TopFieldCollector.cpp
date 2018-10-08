using namespace std;

#include "TopFieldCollector.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Entry = org::apache::lucene::search::FieldValueHitQueue::Entry;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

TopFieldCollector::MultiComparatorLeafCollector::MultiComparatorLeafCollector(
    std::deque<std::shared_ptr<LeafFieldComparator>> &comparators,
    std::deque<int> &reverseMul, bool mayNeedScoresTwice)
    : mayNeedScoresTwice(mayNeedScoresTwice)
{
  if (comparators.size() == 1) {
    this->reverseMul = reverseMul[0];
    this->comparator = comparators[0];
  } else {
    this->reverseMul = 1;
    this->comparator =
        make_shared<MultiLeafFieldComparator>(comparators, reverseMul);
  }
}

void TopFieldCollector::MultiComparatorLeafCollector::setScorer(
    shared_ptr<Scorer> scorer) 
{
  if (mayNeedScoresTwice &&
      std::dynamic_pointer_cast<ScoreCachingWrappingScorer>(scorer) !=
          nullptr == false) {
    scorer = make_shared<ScoreCachingWrappingScorer>(scorer);
  }
  comparator->setScorer(scorer);
  this->scorer = scorer;
}

bool TopFieldCollector::canEarlyTerminate(shared_ptr<Sort> searchSort,
                                          shared_ptr<Sort> indexSort)
{
  std::deque<std::shared_ptr<SortField>> fields1 = searchSort->getSort();
  std::deque<std::shared_ptr<SortField>> fields2 = indexSort->getSort();
  // early termination is possible if fields1 is a prefix of fields2
  if (fields1.size() > fields2.size()) {
    return false;
  }
  return Arrays::asList(fields1).equals(
      Arrays::asList(fields2).subList(0, fields1.size()));
}

int TopFieldCollector::estimateRemainingHits(int hitCount, int doc, int maxDoc)
{
  double hitRatio = static_cast<double>(hitCount) / (doc + 1);
  int remainingDocs = maxDoc - doc - 1;
  int remainingHits = static_cast<int>(remainingDocs * hitRatio);
  return remainingHits;
}

TopFieldCollector::SimpleFieldCollector::SimpleFieldCollector(
    shared_ptr<Sort> sort,
    shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue, int numHits,
    bool fillFields, bool trackDocScores, bool trackMaxScore,
    bool trackTotalHits)
    : TopFieldCollector(queue, numHits, fillFields,
                        sort->needsScores() || trackDocScores || trackMaxScore),
      sort(sort), queue(queue), trackDocScores(trackDocScores),
      trackMaxScore(trackMaxScore),
      mayNeedScoresTwice(sort->needsScores() &&
                         (trackDocScores || trackMaxScore)),
      trackTotalHits(trackTotalHits)
{
  if (trackMaxScore) {
    maxScore =
        -numeric_limits<float>::infinity(); // otherwise we would keep NaN
  }
  // If one of the sort fields needs scores, and if we also track scores, then
  // we might call scorer.score() several times per doc so wrapping the scorer
  // to cache scores would help
}

shared_ptr<LeafCollector>
TopFieldCollector::SimpleFieldCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;

  std::deque<std::shared_ptr<LeafFieldComparator>> comparators =
      queue->getComparators(context);
  const std::deque<int> reverseMul = queue->getReverseMul();
  shared_ptr<Sort> *const indexSort =
      context->reader()->getMetaData()->getSort();
  constexpr bool canEarlyTerminate =
      trackTotalHits == false && trackMaxScore == false &&
      indexSort != nullptr &&
      SimpleFieldCollector::canEarlyTerminate(sort, indexSort);
  constexpr int initialTotalHits = totalHits;

  return make_shared<MultiComparatorLeafCollectorAnonymousInnerClass>(
      shared_from_this(), mayNeedScoresTwice, context, reverseMul,
      canEarlyTerminate, initialTotalHits);
}

TopFieldCollector::SimpleFieldCollector::
    MultiComparatorLeafCollectorAnonymousInnerClass::
        MultiComparatorLeafCollectorAnonymousInnerClass(
            shared_ptr<SimpleFieldCollector> outerInstance,
            bool mayNeedScoresTwice, shared_ptr<LeafReaderContext> context,
            deque<int> &reverseMul, bool canEarlyTerminate,
            int initialTotalHits)
    : MultiComparatorLeafCollector(comparators, reverseMul, mayNeedScoresTwice)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->reverseMul = reverseMul;
  this->canEarlyTerminate = canEarlyTerminate;
  this->initialTotalHits = initialTotalHits;
}

void TopFieldCollector::SimpleFieldCollector::
    MultiComparatorLeafCollectorAnonymousInnerClass::collect(int doc) throw(
        IOException)
{
  float score = NAN;
  if (outerInstance->trackMaxScore) {
    score = scorer::score();
    if (score > outerInstance->maxScore) {
      outerInstance->maxScore = score;
    }
  }

  ++outerInstance->totalHits;
  if (outerInstance->queueFull) {
    if (reverseMul * comparator::compareBottom(doc) <= 0) {
      // since docs are visited in doc Id order, if compare is 0, it means
      // this document is largest than anything else in the queue, and
      // therefore not competitive.
      if (canEarlyTerminate) {
        // scale totalHits linearly based on the number of docs
        // and terminate collection
        outerInstance->totalHits +=
            estimateRemainingHits(outerInstance->totalHits - initialTotalHits,
                                  doc, context->reader()->maxDoc());
        outerInstance->earlyTerminated = true;
        throw make_shared<CollectionTerminatedException>();
      } else {
        // just move to the next doc
        return;
      }
    }

    if (outerInstance->trackDocScores && !outerInstance->trackMaxScore) {
      score = scorer::score();
    }

    // This hit is competitive - replace bottom element in queue & adjustTop
    comparator::copy(outerInstance->bottom->slot, doc);
    outerInstance->updateBottom(doc, score);
    comparator::setBottom(outerInstance->bottom->slot);
  } else {
    // Startup transient: queue hasn't gathered numHits yet
    constexpr int slot = outerInstance->totalHits - 1;

    if (outerInstance->trackDocScores && !outerInstance->trackMaxScore) {
      score = scorer::score();
    }

    // Copy hit into queue
    comparator::copy(slot, doc);
    outerInstance->add(slot, doc, score);
    if (outerInstance->queueFull) {
      comparator::setBottom(outerInstance->bottom->slot);
    }
  }
}

TopFieldCollector::PagingFieldCollector::PagingFieldCollector(
    shared_ptr<Sort> sort,
    shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue,
    shared_ptr<FieldDoc> after, int numHits, bool fillFields,
    bool trackDocScores, bool trackMaxScore, bool trackTotalHits)
    : TopFieldCollector(queue, numHits, fillFields,
                        trackDocScores || trackMaxScore || sort->needsScores()),
      sort(sort), queue(queue), trackDocScores(trackDocScores),
      trackMaxScore(trackMaxScore), after(after),
      mayNeedScoresTwice(sort->needsScores() &&
                         (trackDocScores || trackMaxScore)),
      trackTotalHits(trackTotalHits)
{

  // Must set maxScore to NEG_INF, or otherwise Math.max always returns NaN.
  if (trackMaxScore) {
    maxScore = -numeric_limits<float>::infinity();
  }

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: FieldComparator<?>[] comparators = queue.comparators;
  std::deque < FieldComparator < ? >> comparators = queue->comparators;
  // Tell all comparators their top value:
  for (int i = 0; i < comparators.size(); i++) {
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") FieldComparator<Object>
    // comparator = (FieldComparator<Object>) comparators[i];
    shared_ptr<FieldComparator<any>> comparator =
        std::static_pointer_cast<FieldComparator<any>>(comparators[i]);
    comparator->setTopValue(after->fields[i]);
  }
}

shared_ptr<LeafCollector>
TopFieldCollector::PagingFieldCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
  constexpr int afterDoc = after->doc - docBase;
  shared_ptr<Sort> *const indexSort =
      context->reader()->getMetaData()->getSort();
  constexpr bool canEarlyTerminate =
      trackTotalHits == false && trackMaxScore == false &&
      indexSort != nullptr &&
      PagingFieldCollector::canEarlyTerminate(sort, indexSort);
  constexpr int initialTotalHits = totalHits;
  return make_shared<MultiComparatorLeafCollectorAnonymousInnerClass>(
      shared_from_this(), queue->getComparators(context),
      queue->getReverseMul(), mayNeedScoresTwice, context, afterDoc,
      canEarlyTerminate, initialTotalHits);
}

TopFieldCollector::PagingFieldCollector::
    MultiComparatorLeafCollectorAnonymousInnerClass::
        MultiComparatorLeafCollectorAnonymousInnerClass(
            shared_ptr<PagingFieldCollector> outerInstance,
            deque<std::shared_ptr<
                org::apache::lucene::search::LeafFieldComparator>>
                &getComparators,
            deque<int> &getReverseMul, bool mayNeedScoresTwice,
            shared_ptr<LeafReaderContext> context, int afterDoc,
            bool canEarlyTerminate, int initialTotalHits)
    : MultiComparatorLeafCollector(getComparators, getReverseMul,
                                   mayNeedScoresTwice)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->afterDoc = afterDoc;
  this->canEarlyTerminate = canEarlyTerminate;
  this->initialTotalHits = initialTotalHits;
}

void TopFieldCollector::PagingFieldCollector::
    MultiComparatorLeafCollectorAnonymousInnerClass::collect(int doc) throw(
        IOException)
{
  // System.out.println("  collect doc=" + doc);

  outerInstance->totalHits++;

  float score = NAN;
  if (outerInstance->trackMaxScore) {
    score = scorer::score();
    if (score > outerInstance->maxScore) {
      outerInstance->maxScore = score;
    }
  }

  if (outerInstance->queueFull) {
    // Fastmatch: return if this hit is no better than
    // the worst hit currently in the queue:
    constexpr int cmp = reverseMul * comparator::compareBottom(doc);
    if (cmp <= 0) {
      // not competitive since documents are visited in doc id order
      if (canEarlyTerminate) {
        // scale totalHits linearly based on the number of docs
        // and terminate collection
        outerInstance->totalHits +=
            estimateRemainingHits(outerInstance->totalHits - initialTotalHits,
                                  doc, context->reader()->maxDoc());
        outerInstance->earlyTerminated = true;
        throw make_shared<CollectionTerminatedException>();
      } else {
        // just move to the next doc
        return;
      }
    }
  }

  constexpr int topCmp = reverseMul * comparator::compareTop(doc);
  if (topCmp > 0 || (topCmp == 0 && doc <= afterDoc)) {
    // Already collected on a previous page
    return;
  }

  if (outerInstance->queueFull) {
    // This hit is competitive - replace bottom element in queue & adjustTop
    comparator::copy(outerInstance->bottom->slot, doc);

    // Compute score only if it is competitive.
    if (outerInstance->trackDocScores && !outerInstance->trackMaxScore) {
      score = scorer::score();
    }
    outerInstance->updateBottom(doc, score);

    comparator::setBottom(outerInstance->bottom->slot);
  } else {
    outerInstance->collectedHits++;

    // Startup transient: queue hasn't gathered numHits yet
    constexpr int slot = outerInstance->collectedHits - 1;
    // System.out.println("    slot=" + slot);
    // Copy hit into queue
    comparator::copy(slot, doc);

    // Compute score only if it is competitive.
    if (outerInstance->trackDocScores && !outerInstance->trackMaxScore) {
      score = scorer::score();
    }
    outerInstance->bottom = outerInstance->pq->push_back(
        make_shared<Entry>(slot, outerInstance->docBase + doc, score));
    outerInstance->queueFull =
        outerInstance->collectedHits == outerInstance->numHits;
    if (outerInstance->queueFull) {
      comparator::setBottom(outerInstance->bottom->slot);
    }
  }
}

std::deque<std::shared_ptr<ScoreDoc>> const
    TopFieldCollector::EMPTY_SCOREDOCS =
        std::deque<std::shared_ptr<ScoreDoc>>(0);

TopFieldCollector::TopFieldCollector(
    shared_ptr<PriorityQueue<std::shared_ptr<Entry>>> pq, int numHits,
    bool fillFields, bool needsScores)
    : TopDocsCollector<org::apache::lucene::search::FieldValueHitQueue::Entry>(
          pq),
      fillFields(fillFields), numHits(numHits), needsScores(needsScores)
{
}

bool TopFieldCollector::needsScores() { return needsScores_; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static TopFieldCollector create(Sort sort,
// int numHits, bool fillFields, bool trackDocScores, bool
// trackMaxScore)
shared_ptr<TopFieldCollector>
TopFieldCollector::create(shared_ptr<Sort> sort, int numHits, bool fillFields,
                          bool trackDocScores, bool trackMaxScore)
{
  return create(sort, numHits, fillFields, trackDocScores, trackMaxScore, true);
}

shared_ptr<TopFieldCollector>
TopFieldCollector::create(shared_ptr<Sort> sort, int numHits, bool fillFields,
                          bool trackDocScores, bool trackMaxScore,
                          bool trackTotalHits)
{
  return create(sort, numHits, nullptr, fillFields, trackDocScores,
                trackMaxScore, trackTotalHits);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static TopFieldCollector create(Sort sort,
// int numHits, FieldDoc after, bool fillFields, bool trackDocScores,
// bool trackMaxScore)
shared_ptr<TopFieldCollector>
TopFieldCollector::create(shared_ptr<Sort> sort, int numHits,
                          shared_ptr<FieldDoc> after, bool fillFields,
                          bool trackDocScores, bool trackMaxScore)
{
  return create(sort, numHits, after, fillFields, trackDocScores, trackMaxScore,
                true);
}

shared_ptr<TopFieldCollector>
TopFieldCollector::create(shared_ptr<Sort> sort, int numHits,
                          shared_ptr<FieldDoc> after, bool fillFields,
                          bool trackDocScores, bool trackMaxScore,
                          bool trackTotalHits)
{

  if (sort->fields.empty()) {
    throw invalid_argument(L"Sort must contain at least one field");
  }

  if (numHits <= 0) {
    throw invalid_argument(
        L"numHits must be > 0; please use TotalHitCountCollector if you just "
        L"need the total hit count");
  }

  shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue =
      FieldValueHitQueue::create(sort->fields, numHits);

  if (after == nullptr) {
    return make_shared<SimpleFieldCollector>(sort, queue, numHits, fillFields,
                                             trackDocScores, trackMaxScore,
                                             trackTotalHits);
  } else {
    if (after->fields.empty()) {
      throw invalid_argument(L"after.fields wasn't set; you must pass "
                             L"fillFields=true for the previous search");
    }

    if (after->fields.size() != sort->getSort().size()) {
      throw invalid_argument(L"after.fields has " + after->fields.size() +
                             L" values but sort has " + sort->getSort().size());
    }

    return make_shared<PagingFieldCollector>(sort, queue, after, numHits,
                                             fillFields, trackDocScores,
                                             trackMaxScore, trackTotalHits);
  }
}

void TopFieldCollector::add(int slot, int doc, float score)
{
  bottom = pq->push_back(make_shared<Entry>(slot, docBase + doc, score));
  queueFull = totalHits == numHits;
}

void TopFieldCollector::updateBottom(int doc)
{
  // bottom.score is already set to Float.NaN in add().
  bottom->doc = docBase + doc;
  bottom = pq->updateTop();
}

void TopFieldCollector::updateBottom(int doc, float score)
{
  bottom->doc = docBase + doc;
  bottom->score = score;
  bottom = pq->updateTop();
}

void TopFieldCollector::populateResults(
    std::deque<std::shared_ptr<ScoreDoc>> &results, int howMany)
{
  if (fillFields) {
    // avoid casting if unnecessary.
    shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue =
        std::static_pointer_cast<FieldValueHitQueue<std::shared_ptr<Entry>>>(
            pq);
    for (int i = howMany - 1; i >= 0; i--) {
      results[i] = queue->fillFields(queue->pop());
    }
  } else {
    for (int i = howMany - 1; i >= 0; i--) {
      shared_ptr<Entry> entry = pq->pop();
      results[i] = make_shared<FieldDoc>(entry->doc, entry->score);
    }
  }
}

shared_ptr<TopDocs>
TopFieldCollector::newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results,
                              int start)
{
  if (results.empty()) {
    results = EMPTY_SCOREDOCS;
    // Set maxScore to NaN, in case this is a maxScore tracking collector.
    maxScore = NAN;
  }

  // If this is a maxScoring tracking collector and there were no results,
  return make_shared<TopFieldDocs>(
      totalHits, results,
      (std::static_pointer_cast<FieldValueHitQueue<std::shared_ptr<Entry>>>(pq))
          ->getFields(),
      maxScore);
}

shared_ptr<TopFieldDocs> TopFieldCollector::topDocs()
{
  return std::static_pointer_cast<TopFieldDocs>(
      TopDocsCollector<Entry>::topDocs());
}

bool TopFieldCollector::isEarlyTerminated() { return earlyTerminated; }
} // namespace org::apache::lucene::search