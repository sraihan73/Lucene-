using namespace std;

#include "RandomApproximationQuery.h"

namespace org::apache::lucene::search
{
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

RandomApproximationQuery::RandomApproximationQuery(shared_ptr<Query> query,
                                                   shared_ptr<Random> random)
    : query(query), random(random)
{
}

shared_ptr<Query> RandomApproximationQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const rewritten = query->rewrite(reader);
  if (rewritten != query) {
    return make_shared<RandomApproximationQuery>(rewritten, random);
  }
  return Query::rewrite(reader);
}

bool RandomApproximationQuery::equals(any other)
{
  return sameClassAs(other) &&
         query->equals(
             (any_cast<std::shared_ptr<RandomApproximationQuery>>(other))
                 .query);
}

int RandomApproximationQuery::hashCode()
{
  return 31 * classHash() + query->hashCode();
}

wstring RandomApproximationQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return query->toString(field);
}

shared_ptr<Weight>
RandomApproximationQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost) 
{
  shared_ptr<Weight> *const weight =
      query->createWeight(searcher, needsScores, boost);
  return make_shared<RandomApproximationWeight>(
      weight, make_shared<Random>(random->nextLong()));
}

RandomApproximationQuery::RandomApproximationWeight::RandomApproximationWeight(
    shared_ptr<Weight> weight, shared_ptr<Random> random)
    : FilterWeight(weight), random(random)
{
}

shared_ptr<Scorer> RandomApproximationQuery::RandomApproximationWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> *const scorer = in_->scorer(context);
  if (scorer == nullptr) {
    return nullptr;
  }
  return make_shared<RandomApproximationScorer>(
      scorer, make_shared<Random>(random->nextLong()));
}

RandomApproximationQuery::RandomApproximationScorer::RandomApproximationScorer(
    shared_ptr<Scorer> scorer, shared_ptr<Random> random)
    : Scorer(scorer->getWeight()), scorer(scorer),
      twoPhaseView(make_shared<RandomTwoPhaseView>(random, scorer->begin()))
{
}

shared_ptr<TwoPhaseIterator>
RandomApproximationQuery::RandomApproximationScorer::twoPhaseIterator()
{
  return twoPhaseView;
}

float RandomApproximationQuery::RandomApproximationScorer::score() throw(
    IOException)
{
  return scorer->score();
}

int RandomApproximationQuery::RandomApproximationScorer::docID()
{
  return scorer->docID();
}

shared_ptr<DocIdSetIterator>
RandomApproximationQuery::RandomApproximationScorer::iterator()
{
  return scorer->begin();
}

RandomApproximationQuery::RandomTwoPhaseView::RandomTwoPhaseView(
    shared_ptr<Random> random, shared_ptr<DocIdSetIterator> disi)
    : TwoPhaseIterator(new RandomApproximation(random, disi)), disi(disi),
      randomMatchCost(random->nextFloat() * 200) / *between 0 and 200 * /
{
}

bool RandomApproximationQuery::RandomTwoPhaseView::matches() 
{
  if (approximation_->docID() == -1 ||
      approximation_->docID() == DocIdSetIterator::NO_MORE_DOCS) {
    throw make_shared<AssertionError>(
        L"matches() should not be called on doc ID " +
        to_wstring(approximation_->docID()));
  }
  if (lastDoc == approximation_->docID()) {
    throw make_shared<AssertionError>(
        L"matches() has been called twice on doc ID " +
        to_wstring(approximation_->docID()));
  }
  lastDoc = approximation_->docID();
  return approximation_->docID() == disi->docID();
}

float RandomApproximationQuery::RandomTwoPhaseView::matchCost()
{
  return randomMatchCost;
}

RandomApproximationQuery::RandomApproximation::RandomApproximation(
    shared_ptr<Random> random, shared_ptr<DocIdSetIterator> disi)
    : random(random), disi(disi)
{
}

int RandomApproximationQuery::RandomApproximation::docID() { return doc; }

int RandomApproximationQuery::RandomApproximation::nextDoc() 
{
  return advance(doc + 1);
}

int RandomApproximationQuery::RandomApproximation::advance(int target) throw(
    IOException)
{
  if (disi->docID() < target) {
    disi->advance(target);
  }
  if (disi->docID() == NO_MORE_DOCS) {
    return doc = NO_MORE_DOCS;
  }
  return doc = RandomNumbers::randomIntBetween(random, target, disi->docID());
}

int64_t RandomApproximationQuery::RandomApproximation::cost()
{
  return disi->cost();
}
} // namespace org::apache::lucene::search