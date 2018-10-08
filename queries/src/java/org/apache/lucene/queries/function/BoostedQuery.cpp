using namespace std;

#include "BoostedQuery.h"

namespace org::apache::lucene::queries::function
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using FilterScorer = org::apache::lucene::search::FilterScorer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

BoostedQuery::BoostedQuery(shared_ptr<Query> subQuery,
                           shared_ptr<ValueSource> boostVal)
    : q(subQuery), boostVal(boostVal)
{
}

shared_ptr<Query> BoostedQuery::getQuery() { return q; }

shared_ptr<ValueSource> BoostedQuery::getValueSource() { return boostVal; }

shared_ptr<Query>
BoostedQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> newQ = q->rewrite(reader);
  if (newQ != q) {
    return make_shared<BoostedQuery>(newQ, boostVal);
  }
  return Query::rewrite(reader);
}

shared_ptr<Weight>
BoostedQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                           float boost) 
{
  return make_shared<BoostedQuery::BoostedWeight>(shared_from_this(), searcher,
                                                  needsScores, boost);
}

BoostedQuery::BoostedWeight::BoostedWeight(
    shared_ptr<BoostedQuery> outerInstance, shared_ptr<IndexSearcher> searcher,
    bool needsScores, float boost) 
    : org::apache::lucene::search::Weight(BoostedQuery::this),
      outerInstance(outerInstance)
{
  this->qWeight = searcher->createWeight(outerInstance->q, needsScores, boost);
  this->fcontext = ValueSource::newContext(searcher);
  outerInstance->boostVal->createWeight(fcontext, searcher);
}

void BoostedQuery::BoostedWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  qWeight->extractTerms(terms);
}

shared_ptr<Scorer> BoostedQuery::BoostedWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Scorer> subQueryScorer = qWeight->scorer(context);
  if (subQueryScorer == nullptr) {
    return nullptr;
  }
  return make_shared<BoostedQuery::CustomScorer>(
      outerInstance, context, shared_from_this(), subQueryScorer,
      outerInstance->boostVal);
}

bool BoostedQuery::BoostedWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<Explanation> BoostedQuery::BoostedWeight::explain(
    shared_ptr<LeafReaderContext> readerContext, int doc) 
{
  shared_ptr<Explanation> subQueryExpl = qWeight->explain(readerContext, doc);
  if (!subQueryExpl->isMatch()) {
    return subQueryExpl;
  }
  shared_ptr<FunctionValues> vals =
      outerInstance->boostVal->getValues(fcontext, readerContext);
  float sc = subQueryExpl->getValue() * vals->floatVal(doc);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(sc, outerInstance->toString() + L", product of:",
                            {subQueryExpl, vals->explain(doc)});
}

BoostedQuery::CustomScorer::CustomScorer(
    shared_ptr<BoostedQuery> outerInstance,
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<BoostedQuery::BoostedWeight> w, shared_ptr<Scorer> scorer,
    shared_ptr<ValueSource> vs) 
    : org::apache::lucene::search::FilterScorer(scorer), weight(w),
      vals(vs->getValues(weight->fcontext, readerContext)),
      readerContext(readerContext), outerInstance(outerInstance)
{
}

float BoostedQuery::CustomScorer::score() 
{
  float score = in_->score() * vals->floatVal(in_->docID());

  // Current Lucene priority queues can't handle NaN and -Infinity, so
  // map_obj to -Float.MAX_VALUE. This conditional handles both -infinity
  // and NaN since comparisons with NaN are always false.
  return score > -numeric_limits<float>::infinity()
             ? score
             : -numeric_limits<float>::max();
}

shared_ptr<deque<std::shared_ptr<Scorer::ChildScorer>>>
BoostedQuery::CustomScorer::getChildren()
{
  return Collections::singleton(
      make_shared<Scorer::ChildScorer>(in_, L"CUSTOM"));
}

shared_ptr<Explanation>
BoostedQuery::CustomScorer::explain(int doc) 
{
  shared_ptr<Explanation> subQueryExpl =
      weight->qWeight->explain(readerContext, doc);
  if (!subQueryExpl->isMatch()) {
    return subQueryExpl;
  }
  float sc = subQueryExpl->getValue() * vals->floatVal(doc);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::match(sc, outerInstance->toString() + L", product of:",
                            {subQueryExpl, vals->explain(doc)});
}

wstring BoostedQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  sb->append(L"boost(")
      ->append(q->toString(field))
      ->append(L',')
      ->append(boostVal)
      ->append(L')');
  return sb->toString();
}

bool BoostedQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool BoostedQuery::equalsTo(shared_ptr<BoostedQuery> other)
{
  return q->equals(other->q) && boostVal->equals(other->boostVal);
}

int BoostedQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + q->hashCode();
  h = 31 * h + boostVal->hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function