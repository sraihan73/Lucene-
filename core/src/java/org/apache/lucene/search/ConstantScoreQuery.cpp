using namespace std;

#include "ConstantScoreQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Bits = org::apache::lucene::util::Bits;

ConstantScoreQuery::ConstantScoreQuery(shared_ptr<Query> query)
    : query(Objects::requireNonNull(query, L"Query must not be null"))
{
}

shared_ptr<Query> ConstantScoreQuery::getQuery() { return query; }

shared_ptr<Query>
ConstantScoreQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> rewritten = query->rewrite(reader);

  if (rewritten != query) {
    return make_shared<ConstantScoreQuery>(rewritten);
  }

  if (rewritten->getClass() == ConstantScoreQuery::typeid) {
    return rewritten;
  }

  if (rewritten->getClass() == BoostQuery::typeid) {
    return make_shared<ConstantScoreQuery>(
        (std::static_pointer_cast<BoostQuery>(rewritten))->getQuery());
  }

  return Query::rewrite(reader);
}

ConstantScoreQuery::ConstantBulkScorer::ConstantBulkScorer(
    shared_ptr<BulkScorer> bulkScorer, shared_ptr<Weight> weight,
    float theScore)
    : bulkScorer(bulkScorer), weight(weight), theScore(theScore)
{
}

int ConstantScoreQuery::ConstantBulkScorer::score(
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs, int min,
    int max) 
{
  return bulkScorer->score(wrapCollector(collector), acceptDocs, min, max);
}

shared_ptr<LeafCollector> ConstantScoreQuery::ConstantBulkScorer::wrapCollector(
    shared_ptr<LeafCollector> collector)
{
  return make_shared<FilterLeafCollectorAnonymousInnerClass>(shared_from_this(),
                                                             collector);
}

ConstantScoreQuery::ConstantBulkScorer::FilterLeafCollectorAnonymousInnerClass::
    FilterLeafCollectorAnonymousInnerClass(
        shared_ptr<ConstantBulkScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::LeafCollector> collector)
    : FilterLeafCollector(collector)
{
  this->outerInstance = outerInstance;
}

void ConstantScoreQuery::ConstantBulkScorer::
    FilterLeafCollectorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  // we must wrap again here, but using the scorer passed in as parameter:
  in_::setScorer(
      make_shared<FilterScorerAnonymousInnerClass>(shared_from_this(), scorer));
}

ConstantScoreQuery::ConstantBulkScorer::FilterLeafCollectorAnonymousInnerClass::
    FilterScorerAnonymousInnerClass::FilterScorerAnonymousInnerClass(
        shared_ptr<FilterLeafCollectorAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::search::Scorer> scorer)
    : FilterScorer(scorer)
{
  this->outerInstance = outerInstance;
}

float ConstantScoreQuery::ConstantBulkScorer::
    FilterLeafCollectorAnonymousInnerClass::FilterScorerAnonymousInnerClass::
        score() 
{
  return outerInstance->outerInstance.theScore;
}

int64_t ConstantScoreQuery::ConstantBulkScorer::cost()
{
  return bulkScorer->cost();
}

shared_ptr<Weight>
ConstantScoreQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  shared_ptr<Weight> *const innerWeight =
      searcher->createWeight(query, false, 1.0f);
  if (needsScores) {
    return make_shared<ConstantScoreWeightAnonymousInnerClass>(
        shared_from_this(), boost, innerWeight);
  } else {
    return innerWeight;
  }
}

ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<ConstantScoreQuery> outerInstance, float boost,
        shared_ptr<org::apache::lucene::search::Weight> innerWeight)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
  this->innerWeight = innerWeight;
}

shared_ptr<BulkScorer>
ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BulkScorer> *const innerScorer = innerWeight->bulkScorer(context);
  if (innerScorer == nullptr) {
    return nullptr;
  }
  return make_shared<ConstantBulkScorer>(innerScorer, shared_from_this(),
                                         score());
}

shared_ptr<ScorerSupplier>
ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> innerScorerSupplier =
      innerWeight->scorerSupplier(context);
  if (innerScorerSupplier == nullptr) {
    return nullptr;
  }
  return make_shared<ScorerSupplierAnonymousInnerClass>(shared_from_this(),
                                                        innerScorerSupplier);
}

ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::ScorerSupplierAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::search::ScorerSupplier>
            innerScorerSupplier)
{
  this->outerInstance = outerInstance;
  this->innerScorerSupplier = innerScorerSupplier;
}

shared_ptr<Scorer> ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::get(int64_t leadCost) throw(
        IOException)
{
  shared_ptr<Scorer> *const innerScorer = innerScorerSupplier->get(leadCost);
  constexpr float score = score();
  return make_shared<FilterScorerAnonymousInnerClass>(shared_from_this(),
                                                      innerScorer, score);
}

ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::FilterScorerAnonymousInnerClass::
        FilterScorerAnonymousInnerClass(
            shared_ptr<ScorerSupplierAnonymousInnerClass> outerInstance,
            shared_ptr<org::apache::lucene::search::Scorer> innerScorer,
            float score)
    : FilterScorer(innerScorer)
{
  this->outerInstance = outerInstance;
  this->innerScorer = innerScorer;
  this->score = score;
}

float ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::FilterScorerAnonymousInnerClass::
        score() 
{
  return score;
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>> ConstantScoreQuery::
    ConstantScoreWeightAnonymousInnerClass::ScorerSupplierAnonymousInnerClass::
        FilterScorerAnonymousInnerClass::getChildren()
{
  return Collections::singleton(
      make_shared<ChildScorer>(innerScorer, L"constant"));
}

int64_t ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::cost()
{
  return innerScorerSupplier->cost();
}

shared_ptr<Matches>
ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return innerWeight->matches(context, doc);
}

shared_ptr<Scorer>
ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool ConstantScoreQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return innerWeight->isCacheable(ctx);
}

wstring ConstantScoreQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return (make_shared<StringBuilder>(L"ConstantScore("))
      ->append(query->toString(field))
      ->append(L')')
      ->toString();
}

bool ConstantScoreQuery::equals(any other)
{
  return sameClassAs(other) &&
         query->equals(
             (any_cast<std::shared_ptr<ConstantScoreQuery>>(other)).query);
}

int ConstantScoreQuery::hashCode()
{
  return 31 * classHash() + query->hashCode();
}
} // namespace org::apache::lucene::search