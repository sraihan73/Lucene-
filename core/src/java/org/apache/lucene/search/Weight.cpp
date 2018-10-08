using namespace std;

#include "Weight.h"

namespace org::apache::lucene::search
{
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Bits = org::apache::lucene::util::Bits;

Weight::Weight(shared_ptr<Query> query) : parentQuery(query) {}

shared_ptr<Matches> Weight::matches(shared_ptr<LeafReaderContext> context,
                                    int doc) 
{
  shared_ptr<Scorer> scorer = this->scorer(context);
  if (scorer == nullptr) {
    return nullptr;
  }
  shared_ptr<TwoPhaseIterator> *const twoPhase = scorer->twoPhaseIterator();
  if (twoPhase == nullptr) {
    if (scorer->begin().advance(doc) != doc) {
      return nullptr;
    }
  } else {
    if (twoPhase->approximation()->advance(doc) != doc ||
        twoPhase->matches() == false) {
      return nullptr;
    }
  }
  return Matches::MATCH_WITH_NO_TERMS;
}

shared_ptr<Query> Weight::getQuery() { return parentQuery; }

shared_ptr<ScorerSupplier>
Weight::scorerSupplier(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> *const scorer = this->scorer(context);
  if (scorer == nullptr) {
    return nullptr;
  }
  return make_shared<ScorerSupplierAnonymousInnerClass>(shared_from_this(),
                                                        scorer);
}

Weight::ScorerSupplierAnonymousInnerClass::ScorerSupplierAnonymousInnerClass(
    shared_ptr<Weight> outerInstance,
    shared_ptr<org::apache::lucene::search::Scorer> scorer)
{
  this->outerInstance = outerInstance;
  this->scorer = scorer;
}

shared_ptr<Scorer>
Weight::ScorerSupplierAnonymousInnerClass::get(int64_t leadCost)
{
  return scorer;
}

int64_t Weight::ScorerSupplierAnonymousInnerClass::cost()
{
  return scorer->begin().cost();
}

shared_ptr<BulkScorer>
Weight::bulkScorer(shared_ptr<LeafReaderContext> context) 
{

  shared_ptr<Scorer> scorer = this->scorer(context);
  if (scorer == nullptr) {
    // No docs match
    return nullptr;
  }

  // This impl always scores docs in order, so we can
  // ignore scoreDocsInOrder:
  return make_shared<DefaultBulkScorer>(scorer);
}

Weight::DefaultBulkScorer::DefaultBulkScorer(shared_ptr<Scorer> scorer)
    : scorer(scorer), iterator(scorer->begin()),
      twoPhase(scorer->twoPhaseIterator())
{
  if (scorer == nullptr) {
    throw make_shared<NullPointerException>();
  }
}

int64_t Weight::DefaultBulkScorer::cost() { return iterator->cost(); }

int Weight::DefaultBulkScorer::score(shared_ptr<LeafCollector> collector,
                                     shared_ptr<Bits> acceptDocs, int min,
                                     int max) 
{
  collector->setScorer(scorer);
  if (scorer->docID() == -1 && min == 0 &&
      max == DocIdSetIterator::NO_MORE_DOCS) {
    scoreAll(collector, iterator, twoPhase, acceptDocs);
    return DocIdSetIterator::NO_MORE_DOCS;
  } else {
    int doc = scorer->docID();
    if (doc < min) {
      if (twoPhase == nullptr) {
        doc = iterator->advance(min);
      } else {
        doc = twoPhase->approximation()->advance(min);
      }
    }
    return scoreRange(collector, iterator, twoPhase, acceptDocs, doc, max);
  }
}

int Weight::DefaultBulkScorer::scoreRange(shared_ptr<LeafCollector> collector,
                                          shared_ptr<DocIdSetIterator> iterator,
                                          shared_ptr<TwoPhaseIterator> twoPhase,
                                          shared_ptr<Bits> acceptDocs,
                                          int currentDoc,
                                          int end) 
{
  if (twoPhase == nullptr) {
    while (currentDoc < end) {
      if (acceptDocs == nullptr || acceptDocs->get(currentDoc)) {
        collector->collect(currentDoc);
      }
      currentDoc = iterator->nextDoc();
    }
    return currentDoc;
  } else {
    shared_ptr<DocIdSetIterator> *const approximation =
        twoPhase->approximation();
    while (currentDoc < end) {
      if ((acceptDocs == nullptr || acceptDocs->get(currentDoc)) &&
          twoPhase->matches()) {
        collector->collect(currentDoc);
      }
      currentDoc = approximation->nextDoc();
    }
    return currentDoc;
  }
}

void Weight::DefaultBulkScorer::scoreAll(
    shared_ptr<LeafCollector> collector, shared_ptr<DocIdSetIterator> iterator,
    shared_ptr<TwoPhaseIterator> twoPhase,
    shared_ptr<Bits> acceptDocs) 
{
  if (twoPhase == nullptr) {
    for (int doc = iterator->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = iterator->nextDoc()) {
      if (acceptDocs == nullptr || acceptDocs->get(doc)) {
        collector->collect(doc);
      }
    }
  } else {
    // The scorer has an approximation, so run the approximation first, then
    // check acceptDocs, then confirm
    shared_ptr<DocIdSetIterator> *const approximation =
        twoPhase->approximation();
    for (int doc = approximation->nextDoc();
         doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = approximation->nextDoc()) {
      if ((acceptDocs == nullptr || acceptDocs->get(doc)) &&
          twoPhase->matches()) {
        collector->collect(doc);
      }
    }
  }
}
} // namespace org::apache::lucene::search