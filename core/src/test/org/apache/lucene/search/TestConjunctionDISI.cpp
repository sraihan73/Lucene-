using namespace std;

#include "TestConjunctionDISI.h"

namespace org::apache::lucene::search
{
using BitDocIdSet = org::apache::lucene::util::BitDocIdSet;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<TwoPhaseIterator>
TestConjunctionDISI::approximation(shared_ptr<DocIdSetIterator> iterator,
                                   shared_ptr<FixedBitSet> confirmed)
{
  shared_ptr<DocIdSetIterator> approximation;
  if (random()->nextBoolean()) {
    approximation = anonymizeIterator(iterator);
  } else {
    approximation = iterator;
  }
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(approximation,
                                                          confirmed);
}

TestConjunctionDISI::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> approximation,
        shared_ptr<FixedBitSet> confirmed)
    : TwoPhaseIterator(approximation)
{
  this->confirmed = confirmed;
  this->approximation = approximation;
}

bool TestConjunctionDISI::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  return confirmed->get(approximation->docID());
}

float TestConjunctionDISI::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 5; // #operations in FixedBitSet#get()
}

shared_ptr<DocIdSetIterator>
TestConjunctionDISI::anonymizeIterator(shared_ptr<DocIdSetIterator> it)
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(it);
}

TestConjunctionDISI::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> it)
{
  this->it = it;
}

int TestConjunctionDISI::DocIdSetIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return it->nextDoc();
}

int TestConjunctionDISI::DocIdSetIteratorAnonymousInnerClass::docID()
{
  return it->docID();
}

int64_t TestConjunctionDISI::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return it->docID();
}

int TestConjunctionDISI::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  return it->advance(target);
}

shared_ptr<Scorer>
TestConjunctionDISI::scorer(shared_ptr<TwoPhaseIterator> twoPhaseIterator)
{
  return scorer(TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator),
                twoPhaseIterator);
}

shared_ptr<Scorer>
TestConjunctionDISI::scorer(shared_ptr<DocIdSetIterator> it,
                            shared_ptr<TwoPhaseIterator> twoPhaseIterator)
{
  return make_shared<ScorerAnonymousInnerClass>(it, twoPhaseIterator);
}

TestConjunctionDISI::ScorerAnonymousInnerClass::ScorerAnonymousInnerClass(
    shared_ptr<org::apache::lucene::search::DocIdSetIterator> it,
    shared_ptr<org::apache::lucene::search::TwoPhaseIterator> twoPhaseIterator)
    : Scorer(nullptr)
{
  this->it = it;
  this->twoPhaseIterator = twoPhaseIterator;
}

shared_ptr<DocIdSetIterator>
TestConjunctionDISI::ScorerAnonymousInnerClass::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass2>(shared_from_this());
}

TestConjunctionDISI::ScorerAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass2::DocIdSetIteratorAnonymousInnerClass2(
        shared_ptr<ScorerAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestConjunctionDISI::ScorerAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass2::docID()
{
  return outerInstance->it.docID();
}

int TestConjunctionDISI::ScorerAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass2::nextDoc() 
{
  if (outerInstance->twoPhaseIterator != nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"ConjunctionDISI should call the two-phase iterator");
  }
  return outerInstance->it.nextDoc();
}

int TestConjunctionDISI::ScorerAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass2::advance(int target) 
{
  if (outerInstance->twoPhaseIterator != nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"ConjunctionDISI should call the two-phase iterator");
  }
  return outerInstance->it.advance(target);
}

int64_t TestConjunctionDISI::ScorerAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass2::cost()
{
  if (outerInstance->twoPhaseIterator != nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"ConjunctionDISI should call the two-phase iterator");
  }
  return outerInstance->it.cost();
}

shared_ptr<TwoPhaseIterator>
TestConjunctionDISI::ScorerAnonymousInnerClass::twoPhaseIterator()
{
  return twoPhaseIterator;
}

int TestConjunctionDISI::ScorerAnonymousInnerClass::docID()
{
  if (twoPhaseIterator != nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"ConjunctionDISI should call the two-phase iterator");
  }
  return it->docID();
}

float TestConjunctionDISI::ScorerAnonymousInnerClass::score() 
{
  return 0;
}

shared_ptr<FixedBitSet> TestConjunctionDISI::randomSet(int maxDoc)
{
  constexpr int step = TestUtil::nextInt(random(), 1, 10);
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(maxDoc);
  for (int doc = random()->nextInt(step); doc < maxDoc;
       doc += TestUtil::nextInt(random(), 1, step)) {
    set->set(doc);
  }
  return set;
}

shared_ptr<FixedBitSet>
TestConjunctionDISI::clearRandomBits(shared_ptr<FixedBitSet> other)
{
  shared_ptr<FixedBitSet> *const set =
      make_shared<FixedBitSet>(other->length());
  set->or (other);
  for (int i = 0; i < set->length(); ++i) {
    if (random()->nextBoolean()) {
      set->clear(i);
    }
  }
  return set;
}

shared_ptr<FixedBitSet> TestConjunctionDISI::intersect(
    std::deque<std::shared_ptr<FixedBitSet>> &bitSets)
{
  shared_ptr<FixedBitSet> *const intersection =
      make_shared<FixedBitSet>(bitSets[0]->length());
  intersection->or (bitSets[0]);
  for (int i = 1; i < bitSets.size(); ++i) {
    intersection->and (bitSets[i]);
  }
  return intersection;
}

shared_ptr<FixedBitSet> TestConjunctionDISI::toBitSet(
    int maxDoc, shared_ptr<DocIdSetIterator> iterator) 
{
  shared_ptr<FixedBitSet> *const set = make_shared<FixedBitSet>(maxDoc);
  for (int doc = iterator->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = iterator->nextDoc()) {
    set->set(doc);
  }
  return set;
}

void TestConjunctionDISI::testConjunction() 
{
  constexpr int iters = atLeast(100);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int maxDoc = TestUtil::nextInt(random(), 100, 10000);
    constexpr int numIterators = TestUtil::nextInt(random(), 2, 5);
    std::deque<std::shared_ptr<FixedBitSet>> sets(numIterators);
    std::deque<std::shared_ptr<Scorer>> iterators(numIterators);
    for (int i = 0; i < iterators.size(); ++i) {
      shared_ptr<FixedBitSet> *const set = randomSet(maxDoc);
      switch (random()->nextInt(3)) {
      case 0:
        // simple iterator
        sets[i] = set;
        iterators[i] = make_shared<ConstantScoreScorer>(
            nullptr, 0.0f,
            anonymizeIterator((make_shared<BitDocIdSet>(set))->begin()));
        break;
      case 1:
        // bitSet iterator
        sets[i] = set;
        iterators[i] = make_shared<ConstantScoreScorer>(
            nullptr, 0.0f, (make_shared<BitDocIdSet>(set))->begin());
        break;
      default:
        // scorer with approximation
        shared_ptr<FixedBitSet> *const confirmed = clearRandomBits(set);
        sets[i] = confirmed;
        shared_ptr<TwoPhaseIterator> *const approximation =
            TestConjunctionDISI::approximation(
                (make_shared<BitDocIdSet>(set))->begin(), confirmed);
        iterators[i] = scorer(approximation);
        break;
      }
    }

    shared_ptr<DocIdSetIterator> *const conjunction =
        ConjunctionDISI::intersectScorers(Arrays::asList(iterators));
    TestUtil::assertEquals(intersect(sets), toBitSet(maxDoc, conjunction));
  }
}

void TestConjunctionDISI::testConjunctionApproximation() 
{
  constexpr int iters = atLeast(100);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int maxDoc = TestUtil::nextInt(random(), 100, 10000);
    constexpr int numIterators = TestUtil::nextInt(random(), 2, 5);
    std::deque<std::shared_ptr<FixedBitSet>> sets(numIterators);
    std::deque<std::shared_ptr<Scorer>> iterators(numIterators);
    bool hasApproximation = false;
    for (int i = 0; i < iterators.size(); ++i) {
      shared_ptr<FixedBitSet> *const set = randomSet(maxDoc);
      if (random()->nextBoolean()) {
        // simple iterator
        sets[i] = set;
        iterators[i] = make_shared<ConstantScoreScorer>(
            nullptr, 0.0f, (make_shared<BitDocIdSet>(set))->begin());
      } else {
        // scorer with approximation
        shared_ptr<FixedBitSet> *const confirmed = clearRandomBits(set);
        sets[i] = confirmed;
        shared_ptr<TwoPhaseIterator> *const approximation =
            TestConjunctionDISI::approximation(
                (make_shared<BitDocIdSet>(set))->begin(), confirmed);
        iterators[i] = scorer(approximation);
        hasApproximation = true;
      }
    }

    shared_ptr<DocIdSetIterator> *const conjunction =
        ConjunctionDISI::intersectScorers(Arrays::asList(iterators));
    shared_ptr<TwoPhaseIterator> twoPhaseIterator =
        TwoPhaseIterator::unwrap(conjunction);
    TestUtil::assertEquals(hasApproximation, twoPhaseIterator != nullptr);
    if (hasApproximation) {
      TestUtil::assertEquals(
          intersect(sets),
          toBitSet(maxDoc,
                   TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator)));
    }
  }
}

void TestConjunctionDISI::testRecursiveConjunctionApproximation() throw(
    IOException)
{
  constexpr int iters = atLeast(100);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int maxDoc = TestUtil::nextInt(random(), 100, 10000);
    constexpr int numIterators = TestUtil::nextInt(random(), 2, 5);
    std::deque<std::shared_ptr<FixedBitSet>> sets(numIterators);
    shared_ptr<Scorer> conjunction = nullptr;
    bool hasApproximation = false;
    for (int i = 0; i < numIterators; ++i) {
      shared_ptr<FixedBitSet> *const set = randomSet(maxDoc);
      shared_ptr<Scorer> *const newIterator;
      switch (random()->nextInt(3)) {
      case 0:
        // simple iterator
        sets[i] = set;
        newIterator = make_shared<ConstantScoreScorer>(
            nullptr, 0.0f,
            anonymizeIterator((make_shared<BitDocIdSet>(set))->begin()));
        break;
      case 1:
        // bitSet iterator
        sets[i] = set;
        newIterator = make_shared<ConstantScoreScorer>(
            nullptr, 0.0f, (make_shared<BitDocIdSet>(set))->begin());
        break;
      default:
        // scorer with approximation
        shared_ptr<FixedBitSet> *const confirmed = clearRandomBits(set);
        sets[i] = confirmed;
        shared_ptr<TwoPhaseIterator> *const approximation =
            TestConjunctionDISI::approximation(
                (make_shared<BitDocIdSet>(set))->begin(), confirmed);
        newIterator = scorer(approximation);
        hasApproximation = true;
        break;
      }
      if (conjunction == nullptr) {
        conjunction = newIterator;
      } else {
        shared_ptr<DocIdSetIterator> *const conj =
            ConjunctionDISI::intersectScorers(
                Arrays::asList(conjunction, newIterator));
        conjunction = scorer(conj, TwoPhaseIterator::unwrap(conj));
      }
    }

    shared_ptr<TwoPhaseIterator> twoPhaseIterator =
        conjunction->twoPhaseIterator();
    TestUtil::assertEquals(hasApproximation, twoPhaseIterator != nullptr);
    if (hasApproximation) {
      TestUtil::assertEquals(
          intersect(sets),
          toBitSet(maxDoc,
                   TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator)));
    } else {
      TestUtil::assertEquals(intersect(sets),
                             toBitSet(maxDoc, conjunction->begin()));
    }
  }
}

void TestConjunctionDISI::testCollapseSubConjunctions(
    bool wrapWithScorer) 
{
  constexpr int iters = atLeast(100);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int maxDoc = TestUtil::nextInt(random(), 100, 10000);
    constexpr int numIterators = TestUtil::nextInt(random(), 5, 10);
    std::deque<std::shared_ptr<FixedBitSet>> sets(numIterators);
    const deque<std::shared_ptr<Scorer>> scorers =
        deque<std::shared_ptr<Scorer>>();
    for (int i = 0; i < numIterators; ++i) {
      shared_ptr<FixedBitSet> *const set = randomSet(maxDoc);
      if (random()->nextBoolean()) {
        // simple iterator
        sets[i] = set;
        scorers.push_back(make_shared<ConstantScoreScorer>(
            nullptr, 0.0f, (make_shared<BitDocIdSet>(set))->begin()));
      } else {
        // scorer with approximation
        shared_ptr<FixedBitSet> *const confirmed = clearRandomBits(set);
        sets[i] = confirmed;
        shared_ptr<TwoPhaseIterator> *const approximation =
            TestConjunctionDISI::approximation(
                (make_shared<BitDocIdSet>(set))->begin(), confirmed);
        scorers.push_back(scorer(approximation));
      }
    }

    // make some sub sequences into sub conjunctions
    constexpr int subIters = atLeast(3);
    for (int subIter = 0; subIter < subIters && scorers.size() > 3; ++subIter) {
      constexpr int subSeqStart =
          TestUtil::nextInt(random(), 0, scorers.size() - 2);
      constexpr int subSeqEnd =
          TestUtil::nextInt(random(), subSeqStart + 2, scorers.size());
      deque<std::shared_ptr<Scorer>> subIterators =
          scorers.subList(subSeqStart, subSeqEnd);
      shared_ptr<Scorer> subConjunction;
      if (wrapWithScorer) {
        subConjunction = make_shared<ConjunctionScorer>(
            nullptr, subIterators, Collections::emptyList());
      } else {
        subConjunction = make_shared<ConstantScoreScorer>(
            nullptr, 0.0f, ConjunctionDISI::intersectScorers(subIterators));
      }
      scorers[subSeqStart] = subConjunction;
      int toRemove = subSeqEnd - subSeqStart - 1;
      while (toRemove-- > 0) {
        scorers.erase(scorers.begin() + subSeqStart + 1);
      }
    }
    if (scorers.size() == 1) {
      // ConjunctionDISI needs two iterators
      scorers.push_back(make_shared<ConstantScoreScorer>(
          nullptr, 0.0f, DocIdSetIterator::all(maxDoc)));
    }

    shared_ptr<DocIdSetIterator> *const conjunction =
        ConjunctionDISI::intersectScorers(scorers);
    TestUtil::assertEquals(intersect(sets), toBitSet(maxDoc, conjunction));
  }
}

void TestConjunctionDISI::testCollapseSubConjunctionDISIs() 
{
  testCollapseSubConjunctions(false);
}

void TestConjunctionDISI::testCollapseSubConjunctionScorers() 
{
  testCollapseSubConjunctions(true);
}
} // namespace org::apache::lucene::search