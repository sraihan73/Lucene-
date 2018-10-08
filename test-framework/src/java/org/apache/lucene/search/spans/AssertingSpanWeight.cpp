using namespace std;

#include "AssertingSpanWeight.h"

namespace org::apache::lucene::search::spans
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Similarity = org::apache::lucene::search::similarities::Similarity;

AssertingSpanWeight::AssertingSpanWeight(
    shared_ptr<IndexSearcher> searcher,
    shared_ptr<SpanWeight> in_) 
    : SpanWeight((SpanQuery)in_->getQuery(), searcher, nullptr, 1.0f), in_(in_)
{
}

void AssertingSpanWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  in_->extractTermContexts(contexts);
}

shared_ptr<Spans>
AssertingSpanWeight::getSpans(shared_ptr<LeafReaderContext> context,
                              Postings requiredPostings) 
{
  shared_ptr<Spans> spans = in_->getSpans(context, requiredPostings);
  if (spans == nullptr) {
    return nullptr;
  }
  return make_shared<AssertingSpans>(spans);
}

shared_ptr<Similarity::SimScorer> AssertingSpanWeight::getSimScorer(
    shared_ptr<LeafReaderContext> context) 
{
  return in_->getSimScorer(context);
}

void AssertingSpanWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  in_->extractTerms(terms);
}

shared_ptr<SpanScorer> AssertingSpanWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  return in_->scorer(context);
}

bool AssertingSpanWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return in_->isCacheable(ctx);
}

shared_ptr<Explanation>
AssertingSpanWeight::explain(shared_ptr<LeafReaderContext> context,
                             int doc) 
{
  return in_->explain(context, doc);
}
} // namespace org::apache::lucene::search::spans