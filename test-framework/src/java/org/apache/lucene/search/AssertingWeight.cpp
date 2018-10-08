using namespace std;

#include "AssertingWeight.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

AssertingWeight::AssertingWeight(shared_ptr<Random> random,
                                 shared_ptr<Weight> in_, bool needsScores)
    : FilterWeight(in_), random(random), needsScores(needsScores)
{
}

shared_ptr<Matches>
AssertingWeight::matches(shared_ptr<LeafReaderContext> context,
                         int doc) 
{
  shared_ptr<Matches> matches = in_->matches(context, doc);
  if (matches->empty()) {
    return nullptr;
  }
  return make_shared<AssertingMatches>(matches);
}

shared_ptr<Scorer> AssertingWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  if (random->nextBoolean()) {
    shared_ptr<Scorer> *const inScorer = in_->scorer(context);
    assert(inScorer == nullptr || inScorer->docID() == -1);
    return AssertingScorer::wrap(make_shared<Random>(random->nextLong()),
                                 inScorer, needsScores);
  } else {
    shared_ptr<ScorerSupplier> *const scorerSupplier =
        this->scorerSupplier(context);
    if (scorerSupplier == nullptr) {
      return nullptr;
    }
    if (random->nextBoolean()) {
      // Evil: make sure computing the cost has no side effects
      scorerSupplier->cost();
    }
    return scorerSupplier->get(numeric_limits<int64_t>::max());
  }
}

shared_ptr<ScorerSupplier> AssertingWeight::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> *const inScorerSupplier =
      in_->scorerSupplier(context);
  if (inScorerSupplier == nullptr) {
    return nullptr;
  }
  return make_shared<ScorerSupplierAnonymousInnerClass>(shared_from_this(),
                                                        inScorerSupplier);
}

AssertingWeight::ScorerSupplierAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass(
        shared_ptr<AssertingWeight> outerInstance,
        shared_ptr<org::apache::lucene::search::ScorerSupplier>
            inScorerSupplier)
{
  this->outerInstance = outerInstance;
  this->inScorerSupplier = inScorerSupplier;
  getCalled = false;
}

shared_ptr<Scorer> AssertingWeight::ScorerSupplierAnonymousInnerClass::get(
    int64_t leadCost) 
{
  assert(getCalled == false);
  getCalled = true;
  assert((leadCost >= 0, leadCost));
  return AssertingScorer::wrap(
      make_shared<Random>(outerInstance->random->nextLong()),
      inScorerSupplier->get(leadCost), outerInstance->needsScores);
}

int64_t AssertingWeight::ScorerSupplierAnonymousInnerClass::cost()
{
  constexpr int64_t cost = inScorerSupplier->cost();
  assert(cost >= 0);
  return cost;
}

shared_ptr<BulkScorer> AssertingWeight::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BulkScorer> inScorer = in_->bulkScorer(context);
  if (inScorer == nullptr) {
    return nullptr;
  }

  return AssertingBulkScorer::wrap(make_shared<Random>(random->nextLong()),
                                   inScorer, context->reader()->maxDoc());
}
} // namespace org::apache::lucene::search