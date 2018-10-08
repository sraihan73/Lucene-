using namespace std;

#include "ConjunctionDISI.h"

namespace org::apache::lucene::search
{
using Spans = org::apache::lucene::search::spans::Spans;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;

shared_ptr<DocIdSetIterator> ConjunctionDISI::intersectScorers(
    shared_ptr<deque<std::shared_ptr<Scorer>>> scorers)
{
  if (scorers->size() < 2) {
    throw invalid_argument(
        L"Cannot make a ConjunctionDISI of less than 2 iterators");
  }
  const deque<std::shared_ptr<DocIdSetIterator>> allIterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  const deque<std::shared_ptr<TwoPhaseIterator>> twoPhaseIterators =
      deque<std::shared_ptr<TwoPhaseIterator>>();
  for (auto scorer : scorers) {
    addScorer(scorer, allIterators, twoPhaseIterators);
  }

  return createConjunction(allIterators, twoPhaseIterators);
}

shared_ptr<DocIdSetIterator> ConjunctionDISI::intersectIterators(
    deque<std::shared_ptr<DocIdSetIterator>> &iterators)
{
  if (iterators.size() < 2) {
    throw invalid_argument(
        L"Cannot make a ConjunctionDISI of less than 2 iterators");
  }
  const deque<std::shared_ptr<DocIdSetIterator>> allIterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  const deque<std::shared_ptr<TwoPhaseIterator>> twoPhaseIterators =
      deque<std::shared_ptr<TwoPhaseIterator>>();
  for (auto iterator : iterators) {
    addIterator(iterator, allIterators, twoPhaseIterators);
  }

  return createConjunction(allIterators, twoPhaseIterators);
}

shared_ptr<DocIdSetIterator>
ConjunctionDISI::intersectSpans(deque<std::shared_ptr<Spans>> &spanList)
{
  if (spanList.size() < 2) {
    throw invalid_argument(
        L"Cannot make a ConjunctionDISI of less than 2 iterators");
  }
  const deque<std::shared_ptr<DocIdSetIterator>> allIterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  const deque<std::shared_ptr<TwoPhaseIterator>> twoPhaseIterators =
      deque<std::shared_ptr<TwoPhaseIterator>>();
  for (auto spans : spanList) {
    addSpans(spans, allIterators, twoPhaseIterators);
  }

  return createConjunction(allIterators, twoPhaseIterators);
}

void ConjunctionDISI::addScorer(
    shared_ptr<Scorer> scorer,
    deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
    deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators)
{
  shared_ptr<TwoPhaseIterator> twoPhaseIter = scorer->twoPhaseIterator();
  if (twoPhaseIter != nullptr) {
    addTwoPhaseIterator(twoPhaseIter, allIterators, twoPhaseIterators);
  } else { // no approximation support, use the iterator as-is
    addIterator(scorer->begin(), allIterators, twoPhaseIterators);
  }
}

void ConjunctionDISI::addSpans(
    shared_ptr<Spans> spans,
    deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
    deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators)
{
  shared_ptr<TwoPhaseIterator> twoPhaseIter = spans->asTwoPhaseIterator();
  if (twoPhaseIter != nullptr) {
    addTwoPhaseIterator(twoPhaseIter, allIterators, twoPhaseIterators);
  } else { // no approximation support, use the iterator as-is
    addIterator(spans, allIterators, twoPhaseIterators);
  }
}

void ConjunctionDISI::addIterator(
    shared_ptr<DocIdSetIterator> disi,
    deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
    deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators)
{
  shared_ptr<TwoPhaseIterator> twoPhase = TwoPhaseIterator::unwrap(disi);
  if (twoPhase != nullptr) {
    addTwoPhaseIterator(twoPhase, allIterators, twoPhaseIterators);
  } else if (disi->getClass() ==
             ConjunctionDISI::typeid) { // Check for exactly this class for
                                        // collapsing
    shared_ptr<ConjunctionDISI> conjunction =
        std::static_pointer_cast<ConjunctionDISI>(disi);
    // subconjuctions have already split themselves into two phase iterators and
    // others, so we can take those iterators as they are and move them up to
    // this conjunction
    allIterators.push_back(conjunction->lead1);
    allIterators.push_back(conjunction->lead2);
    Collections::addAll(allIterators, conjunction->others);
  } else if (disi->getClass() == BitSetConjunctionDISI::typeid) {
    shared_ptr<BitSetConjunctionDISI> conjunction =
        std::static_pointer_cast<BitSetConjunctionDISI>(disi);
    allIterators.push_back(conjunction->lead);
    Collections::addAll(allIterators, conjunction->bitSetIterators);
  } else {
    allIterators.push_back(disi);
  }
}

void ConjunctionDISI::addTwoPhaseIterator(
    shared_ptr<TwoPhaseIterator> twoPhaseIter,
    deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
    deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators)
{
  addIterator(twoPhaseIter->approximation(), allIterators, twoPhaseIterators);
  if (twoPhaseIter->getClass() ==
      ConjunctionTwoPhaseIterator::typeid) { // Check for exactly this class for
                                             // collapsing
    Collections::addAll(
        twoPhaseIterators,
        (std::static_pointer_cast<ConjunctionTwoPhaseIterator>(twoPhaseIter))
            ->twoPhaseIterators);
  } else {
    twoPhaseIterators.push_back(twoPhaseIter);
  }
}

shared_ptr<DocIdSetIterator> ConjunctionDISI::createConjunction(
    deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
    deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators)
{
  int64_t minCost =
      allIterators.stream().mapToLong(DocIdSetIterator::cost).min().getAsLong();
  deque<std::shared_ptr<BitSetIterator>> bitSetIterators =
      deque<std::shared_ptr<BitSetIterator>>();
  deque<std::shared_ptr<DocIdSetIterator>> iterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  for (auto iterator : allIterators) {
    if (iterator->cost() > minCost &&
        std::dynamic_pointer_cast<BitSetIterator>(iterator) != nullptr) {
      // we put all bitset iterators into bitSetIterators
      // except if they have the minimum cost, since we need
      // them to lead the iteration in that case
      bitSetIterators.push_back(
          std::static_pointer_cast<BitSetIterator>(iterator));
    } else {
      iterators.push_back(iterator);
    }
  }

  shared_ptr<DocIdSetIterator> disi;
  if (iterators.size() == 1) {
    disi = iterators[0];
  } else {
    disi = make_shared<ConjunctionDISI>(iterators);
  }

  if (bitSetIterators.size() > 0) {
    disi = make_shared<BitSetConjunctionDISI>(disi, bitSetIterators);
  }

  if (twoPhaseIterators.empty() == false) {
    disi = TwoPhaseIterator::asDocIdSetIterator(
        make_shared<ConjunctionTwoPhaseIterator>(disi, twoPhaseIterators));
  }

  return disi;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private ConjunctionDISI(java.util.List<? extends
// DocIdSetIterator> iterators)
ConjunctionDISI::ConjunctionDISI(deque<T1> iterators)
    : lead1(iterators[0]), lead2(iterators[1]),
      others(iterators.subList(2, iterators.size())
                 .toArray(std::deque<std::shared_ptr<DocIdSetIterator>>(0)))
{
  assert(iterators.size() >= 2);
  // Sort the array the first time to allow the least frequent DocsEnum to
  // lead the matching.
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  CollectionUtil::timSort(iterators, make_shared<ComparatorAnonymousInnerClass>(
                                         shared_from_this()));
}

ConjunctionDISI::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<ConjunctionDISI> outerInstance)
{
  this->outerInstance = outerInstance;
}

int ConjunctionDISI::ComparatorAnonymousInnerClass::compare(
    shared_ptr<DocIdSetIterator> o1, shared_ptr<DocIdSetIterator> o2)
{
  return Long::compare(o1->cost(), o2->cost());
}

int ConjunctionDISI::doNext(int doc) 
{
  for (;;) {
    assert(doc == lead1->docID());

    // find agreement between the two iterators with the lower costs
    // we special case them because they do not need the
    // 'other.docID() < doc' check that the 'others' iterators need
    constexpr int next2 = lead2->advance(doc);
    if (next2 != doc) {
      doc = lead1->advance(next2);
      if (next2 != doc) {
        continue;
      }
    }

    // then find agreement with other iterators
    for (auto other : others) {
      // other.doc may already be equal to doc if we "continued advanceHead"
      // on the previous iteration and the advance on the lead scorer exactly
      // matched.
      if (other->docID() < doc) {
        constexpr int next = other->advance(doc);

        if (next > doc) {
          // iterator beyond the current doc - advance lead and continue to the
          // new highest doc.
          doc = lead1->advance(next);
          goto advanceHeadContinue;
        }
      }
    }

    // success - all iterators are on the same doc
    return doc;
  advanceHeadContinue:;
  }
advanceHeadBreak:;
}

int ConjunctionDISI::advance(int target) 
{
  return doNext(lead1->advance(target));
}

int ConjunctionDISI::docID() { return lead1->docID(); }

int ConjunctionDISI::nextDoc() 
{
  return doNext(lead1->nextDoc());
}

int64_t ConjunctionDISI::cost()
{
  return lead1->cost(); // overestimate
}

ConjunctionDISI::BitSetConjunctionDISI::BitSetConjunctionDISI(
    shared_ptr<DocIdSetIterator> lead,
    shared_ptr<deque<std::shared_ptr<BitSetIterator>>> bitSetIterators)
    : lead(lead), bitSetIterators(bitSetIterators->toArray(
                      std::deque<std::shared_ptr<BitSetIterator>>(0))),
      bitSets(
          std::deque<std::shared_ptr<BitSet>>(this->bitSetIterators.size())),
      minLength(minLen)
{
  assert(bitSetIterators->size() > 0);
  // Put the least costly iterators first so that we exit as soon as possible
  ArrayUtil::timSort(this->bitSetIterators,
                     [&](a, b) { Long::compare(a::cost(), b::cost()); });
  int minLen = numeric_limits<int>::max();
  for (int i = 0; i < this->bitSetIterators.size(); ++i) {
    shared_ptr<BitSet> bitSet = this->bitSetIterators[i]->getBitSet();
    this->bitSets[i] = bitSet;
    minLen = min(minLen, bitSet->length());
  }
}

int ConjunctionDISI::BitSetConjunctionDISI::docID() { return lead->docID(); }

int ConjunctionDISI::BitSetConjunctionDISI::nextDoc() 
{
  return doNext(lead->nextDoc());
}

int ConjunctionDISI::BitSetConjunctionDISI::advance(int target) throw(
    IOException)
{
  return doNext(lead->advance(target));
}

int ConjunctionDISI::BitSetConjunctionDISI::doNext(int doc) 
{
  for (;; doc = lead->nextDoc()) {
    if (doc >= minLength) {
      return NO_MORE_DOCS;
    }
    for (auto bitSet : bitSets) {
      if (bitSet->get(doc) == false) {
        goto advanceLeadContinue;
      }
    }
    for (auto iterator : bitSetIterators) {
      iterator->setDocId(doc);
    }
    return doc;
  advanceLeadContinue:;
  }
advanceLeadBreak:;
}

int64_t ConjunctionDISI::BitSetConjunctionDISI::cost()
{
  return lead->cost();
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private
// ConjunctionTwoPhaseIterator(DocIdSetIterator approximation, java.util.List<?
// extends TwoPhaseIterator> twoPhaseIterators)
ConjunctionDISI::ConjunctionTwoPhaseIterator::ConjunctionTwoPhaseIterator(
    shared_ptr<DocIdSetIterator> approximation, deque<T1> twoPhaseIterators)
    : TwoPhaseIterator(approximation),
      twoPhaseIterators(twoPhaseIterators.toArray(
          std::deque<std::shared_ptr<TwoPhaseIterator>>(
              twoPhaseIterators.size()))),
      matchCost(totalMatchCost)
{
  assert(twoPhaseIterators.size() > 0);

  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  CollectionUtil::timSort(
      twoPhaseIterators,
      make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  // Compute the matchCost as the total matchCost of the sub iterators.
  // TODO: This could be too high because the matching is done cheapest first:
  // give the lower matchCosts a higher weight.
  float totalMatchCost = 0;
  for (auto tpi : twoPhaseIterators) {
    totalMatchCost += tpi->matchCost();
  }
}

ConjunctionDISI::ConjunctionTwoPhaseIterator::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<ConjunctionTwoPhaseIterator> outerInstance)
{
  this->outerInstance = outerInstance;
}

int ConjunctionDISI::ConjunctionTwoPhaseIterator::
    ComparatorAnonymousInnerClass::compare(shared_ptr<TwoPhaseIterator> o1,
                                           shared_ptr<TwoPhaseIterator> o2)
{
  return Float::compare(o1->matchCost(), o2->matchCost());
}

bool ConjunctionDISI::ConjunctionTwoPhaseIterator::matches() 
{
  for (auto twoPhaseIterator : twoPhaseIterators) { // match cheapest first
    if (twoPhaseIterator->matches() == false) {
      return false;
    }
  }
  return true;
}

float ConjunctionDISI::ConjunctionTwoPhaseIterator::matchCost()
{
  return matchCost_;
}
} // namespace org::apache::lucene::search