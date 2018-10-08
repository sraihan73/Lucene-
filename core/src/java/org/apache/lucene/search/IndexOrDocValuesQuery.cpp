using namespace std;

#include "IndexOrDocValuesQuery.h"

namespace org::apache::lucene::search
{
using LongPoint = org::apache::lucene::document::LongPoint;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

IndexOrDocValuesQuery::IndexOrDocValuesQuery(shared_ptr<Query> indexQuery,
                                             shared_ptr<Query> dvQuery)
    : indexQuery(indexQuery), dvQuery(dvQuery)
{
}

shared_ptr<Query> IndexOrDocValuesQuery::getIndexQuery() { return indexQuery; }

shared_ptr<Query> IndexOrDocValuesQuery::getRandomAccessQuery()
{
  return dvQuery;
}

wstring IndexOrDocValuesQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return indexQuery->toString(field);
}

bool IndexOrDocValuesQuery::equals(any obj)
{
  if (sameClassAs(obj) == false) {
    return false;
  }
  shared_ptr<IndexOrDocValuesQuery> that =
      any_cast<std::shared_ptr<IndexOrDocValuesQuery>>(obj);
  return indexQuery->equals(that->indexQuery) && dvQuery->equals(that->dvQuery);
}

int IndexOrDocValuesQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + indexQuery->hashCode();
  h = 31 * h + dvQuery->hashCode();
  return h;
}

shared_ptr<Query> IndexOrDocValuesQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> indexRewrite = indexQuery->rewrite(reader);
  shared_ptr<Query> dvRewrite = dvQuery->rewrite(reader);
  if (indexQuery != indexRewrite || dvQuery != dvRewrite) {
    return make_shared<IndexOrDocValuesQuery>(indexRewrite, dvRewrite);
  }
  return shared_from_this();
}

shared_ptr<Weight>
IndexOrDocValuesQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  shared_ptr<Weight> *const indexWeight =
      indexQuery->createWeight(searcher, needsScores, boost);
  shared_ptr<Weight> *const dvWeight =
      dvQuery->createWeight(searcher, needsScores, boost);
  return make_shared<WeightAnonymousInnerClass>(shared_from_this(), indexWeight,
                                                dvWeight);
}

IndexOrDocValuesQuery::WeightAnonymousInnerClass::WeightAnonymousInnerClass(
    shared_ptr<IndexOrDocValuesQuery> outerInstance,
    shared_ptr<org::apache::lucene::search::Weight> indexWeight,
    shared_ptr<org::apache::lucene::search::Weight> dvWeight)
    : Weight(outerInstance)
{
  this->outerInstance = outerInstance;
  this->indexWeight = indexWeight;
  this->dvWeight = dvWeight;
}

void IndexOrDocValuesQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  indexWeight->extractTerms(terms);
}

shared_ptr<Matches> IndexOrDocValuesQuery::WeightAnonymousInnerClass::matches(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  // We need to check a single doc, so the dv query should perform better
  return dvWeight->matches(context, doc);
}

shared_ptr<Explanation>
IndexOrDocValuesQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  // We need to check a single doc, so the dv query should perform better
  return dvWeight->explain(context, doc);
}

shared_ptr<BulkScorer>
IndexOrDocValuesQuery::WeightAnonymousInnerClass::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  // Bulk scorers need to consume the entire set of docs, so using an
  // index structure should perform better
  return indexWeight->bulkScorer(context);
}

shared_ptr<ScorerSupplier>
IndexOrDocValuesQuery::WeightAnonymousInnerClass::scorerSupplier(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> *const indexScorerSupplier =
      indexWeight->scorerSupplier(context);
  shared_ptr<ScorerSupplier> *const dvScorerSupplier =
      dvWeight->scorerSupplier(context);
  if (indexScorerSupplier == nullptr || dvScorerSupplier == nullptr) {
    return nullptr;
  }
  return make_shared<ScorerSupplierAnonymousInnerClass>(
      shared_from_this(), indexScorerSupplier, dvScorerSupplier);
}

IndexOrDocValuesQuery::WeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::ScorerSupplierAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::search::ScorerSupplier>
            indexScorerSupplier,
        shared_ptr<org::apache::lucene::search::ScorerSupplier>
            dvScorerSupplier)
{
  this->outerInstance = outerInstance;
  this->indexScorerSupplier = indexScorerSupplier;
  this->dvScorerSupplier = dvScorerSupplier;
}

shared_ptr<Scorer> IndexOrDocValuesQuery::WeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::get(int64_t leadCost) throw(
        IOException)
{
  // At equal costs, doc values tend to be worse than points since they
  // still need to perform one comparison per document while points can
  // do much better than that given how values are organized. So we give
  // an arbitrary 8x penalty to doc values.
  constexpr int64_t threshold =
      static_cast<int>(static_cast<unsigned int>(cost()) >> 3);
  if (threshold <= leadCost) {
    return indexScorerSupplier->get(leadCost);
  } else {
    return dvScorerSupplier->get(leadCost);
  }
}

int64_t IndexOrDocValuesQuery::WeightAnonymousInnerClass::
    ScorerSupplierAnonymousInnerClass::cost()
{
  return indexScorerSupplier->cost();
}

shared_ptr<Scorer> IndexOrDocValuesQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<ScorerSupplier> scorerSupplier = scorerSupplier(context);
  if (scorerSupplier == nullptr) {
    return nullptr;
  }
  return scorerSupplier->get(numeric_limits<int64_t>::max());
}

bool IndexOrDocValuesQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  // Both index and dv query should return the same values, so we can use
  // the index query's cachehelper here
  return indexWeight->isCacheable(ctx);
}
} // namespace org::apache::lucene::search