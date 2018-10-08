using namespace std;

#include "AssertingSpanQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

AssertingSpanQuery::AssertingSpanQuery(shared_ptr<SpanQuery> in_) : in_(in_) {}

wstring AssertingSpanQuery::getField() { return in_->getField(); }

wstring AssertingSpanQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"AssertingSpanQuery(" + in_->toString(field) + L")";
}

shared_ptr<SpanWeight>
AssertingSpanQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  shared_ptr<SpanWeight> weight =
      in_->createWeight(searcher, needsScores, boost);
  return make_shared<AssertingSpanWeight>(searcher, weight);
}

shared_ptr<Query>
AssertingSpanQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> q = in_->rewrite(reader);
  if (q == in_) {
    return SpanQuery::rewrite(reader);
  } else if (std::dynamic_pointer_cast<SpanQuery>(q) != nullptr) {
    return make_shared<AssertingSpanQuery>(
        std::static_pointer_cast<SpanQuery>(q));
  } else {
    return q;
  }
}

shared_ptr<Query> AssertingSpanQuery::clone()
{
  return make_shared<AssertingSpanQuery>(in_);
}

bool AssertingSpanQuery::equals(any o)
{
  return sameClassAs(o) && equalsTo(getClass().cast(o));
}

bool AssertingSpanQuery::equalsTo(shared_ptr<AssertingSpanQuery> other)
{
  return Objects::equals(in_, other->in_);
}

int AssertingSpanQuery::hashCode()
{
  return (in_ == nullptr) ? 0 : in_->hashCode();
}
} // namespace org::apache::lucene::search::spans