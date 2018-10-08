using namespace std;

#include "FunctionQuery.h"

namespace org::apache::lucene::queries::function
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

FunctionQuery::FunctionQuery(shared_ptr<ValueSource> func)
{
  this->func = func;
}

shared_ptr<ValueSource> FunctionQuery::getValueSource() { return func; }

FunctionQuery::FunctionWeight::FunctionWeight(
    shared_ptr<FunctionQuery> outerInstance, shared_ptr<IndexSearcher> searcher,
    float boost) 
    : org::apache::lucene::search::Weight(FunctionQuery::this),
      searcher(searcher), boost(boost),
      context(ValueSource::newContext(searcher)), outerInstance(outerInstance)
{
  outerInstance->func->createWeight(context, searcher);
}

void FunctionQuery::FunctionWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Scorer> FunctionQuery::FunctionWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  return make_shared<AllScorer>(outerInstance, context, shared_from_this(),
                                boost);
}

bool FunctionQuery::FunctionWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<Explanation>
FunctionQuery::FunctionWeight::explain(shared_ptr<LeafReaderContext> context,
                                       int doc) 
{
  return (std::static_pointer_cast<AllScorer>(scorer(context)))->explain(doc);
}

FunctionQuery::AllScorer::AllScorer(shared_ptr<FunctionQuery> outerInstance,
                                    shared_ptr<LeafReaderContext> context,
                                    shared_ptr<FunctionWeight> w,
                                    float boost) 
    : org::apache::lucene::search::Scorer(w), reader(context->reader()),
      weight(w), maxDoc(reader->maxDoc()), boost(boost),
      iterator(DocIdSetIterator::all(context->reader()->maxDoc())),
      vals(outerInstance->func->getValues(weight->context, context)),
      outerInstance(outerInstance)
{
}

shared_ptr<DocIdSetIterator> FunctionQuery::AllScorer::iterator()
{
  return iterator_;
}

int FunctionQuery::AllScorer::docID() { return iterator_->docID(); }

float FunctionQuery::AllScorer::score() 
{
  float score = boost * vals->floatVal(docID());

  // Current Lucene priority queues can't handle NaN and -Infinity, so
  // map_obj to -Float.MAX_VALUE. This conditional handles both -infinity
  // and NaN since comparisons with NaN are always false.
  return score > -numeric_limits<float>::infinity()
             ? score
             : -numeric_limits<float>::max();
}

shared_ptr<Explanation>
FunctionQuery::AllScorer::explain(int doc) 
{
  float sc = boost * vals->floatVal(doc);

  return Explanation::match(
      sc, L"FunctionQuery(" + outerInstance->func + L"), product of:",
      {vals->explain(doc), Explanation::match(weight->boost, L"boost")});
}

shared_ptr<Weight>
FunctionQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                            bool needsScores, float boost) 
{
  return make_shared<FunctionQuery::FunctionWeight>(shared_from_this(),
                                                    searcher, boost);
}

wstring FunctionQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return func->toString();
}

bool FunctionQuery::equals(any other)
{
  return sameClassAs(other) &&
         func->equals((any_cast<std::shared_ptr<FunctionQuery>>(other)).func);
}

int FunctionQuery::hashCode() { return classHash() ^ func->hashCode(); }
} // namespace org::apache::lucene::queries::function