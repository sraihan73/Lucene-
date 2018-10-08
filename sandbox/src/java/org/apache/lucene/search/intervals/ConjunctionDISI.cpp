using namespace std;

#include "ConjunctionDISI.h"

namespace org::apache::lucene::search::intervals
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static
// org.apache.lucene.search.DocIdSetIterator intersectIterators(java.util.List<?
// extends org.apache.lucene.search.DocIdSetIterator> iterators)
shared_ptr<DocIdSetIterator>
ConjunctionDISI::intersectIterators(deque<T1> iterators)
{
  if (iterators.size() < 2) {
    throw invalid_argument(
        L"Cannot make a ConjunctionDISI of less than 2 iterators");
  }
  const deque<std::shared_ptr<DocIdSetIterator>> allIterators =
      deque<std::shared_ptr<DocIdSetIterator>>();
  for (auto iterator : iterators) {
    addIterator(iterator, allIterators);
  }

  return make_shared<ConjunctionDISI>(allIterators);
}

void ConjunctionDISI::addIterator(
    shared_ptr<DocIdSetIterator> disi,
    deque<std::shared_ptr<DocIdSetIterator>> &allIterators)
{
  if (disi->getClass() ==
      ConjunctionDISI::typeid) { // Check for exactly this class for collapsing
    shared_ptr<ConjunctionDISI> conjunction =
        std::static_pointer_cast<ConjunctionDISI>(disi);
    // subconjuctions have already split themselves into two phase iterators and
    // others, so we can take those iterators as they are and move them up to
    // this conjunction
    allIterators.push_back(conjunction->lead1);
    allIterators.push_back(conjunction->lead2);
    Collections::addAll(allIterators, conjunction->others);
  } else {
    allIterators.push_back(disi);
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private ConjunctionDISI(java.util.List<? extends
// org.apache.lucene.search.DocIdSetIterator> iterators)
ConjunctionDISI::ConjunctionDISI(deque<T1> iterators)
    : lead1(iterators[0]), lead2(iterators[1]),
      others(iterators.subList(2, iterators.size())
                 .toArray(std::deque<std::shared_ptr<DocIdSetIterator>>(0)))
{
  assert(iterators.size() >= 2);
  // Sort the array the first time to allow the least frequent DocsEnum to
  // lead the matching.
  CollectionUtil::timSort(iterators,
                          Comparator::comparingLong(DocIdSetIterator::cost));
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
} // namespace org::apache::lucene::search::intervals