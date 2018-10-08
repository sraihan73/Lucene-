using namespace std;

#include "FilterWeight.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

FilterWeight::FilterWeight(shared_ptr<Weight> weight)
    : FilterWeight(weight->getQuery(), weight)
{
}

FilterWeight::FilterWeight(shared_ptr<Query> query, shared_ptr<Weight> weight)
    : Weight(query), in_(weight)
{
}

bool FilterWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return in_->isCacheable(ctx);
}

void FilterWeight::extractTerms(shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  in_->extractTerms(terms);
}

shared_ptr<Explanation>
FilterWeight::explain(shared_ptr<LeafReaderContext> context,
                      int doc) 
{
  return in_->explain(context, doc);
}

shared_ptr<Scorer>
FilterWeight::scorer(shared_ptr<LeafReaderContext> context) 
{
  return in_->scorer(context);
}

shared_ptr<Matches> FilterWeight::matches(shared_ptr<LeafReaderContext> context,
                                          int doc) 
{
  return in_->matches(context, doc);
}
} // namespace org::apache::lucene::search