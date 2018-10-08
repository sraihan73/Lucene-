using namespace std;

#include "SpanMultiTermQueryWrapper.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using ScoringRewrite = org::apache::lucene::search::ScoringRewrite;
using TopTermsRewrite = org::apache::lucene::search::TopTermsRewrite;

SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::
    SpanRewriteMethodAnonymousInnerClass()
{
}

SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::
    ScoringRewriteAnonymousInnerClass::ScoringRewriteAnonymousInnerClass()
{
}

deque<std::shared_ptr<SpanQuery>>
SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::
    ScoringRewriteAnonymousInnerClass::getTopLevelBuilder()
{
  return deque<std::shared_ptr<SpanQuery>>();
}

shared_ptr<Query>
SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::
    ScoringRewriteAnonymousInnerClass::build(
        deque<std::shared_ptr<SpanQuery>> &builder)
{
  return make_shared<SpanOrQuery>(
      builder.toArray(std::deque<std::shared_ptr<SpanQuery>>(builder.size())));
}

void SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::
    ScoringRewriteAnonymousInnerClass::checkMaxClauseCount(int count)
{
  // we accept all terms as SpanOrQuery has no limits
}

void SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::
    ScoringRewriteAnonymousInnerClass::addClause(
        deque<std::shared_ptr<SpanQuery>> &topLevel, shared_ptr<Term> term,
        int docCount, float boost, shared_ptr<TermContext> states)
{
  shared_ptr<SpanTermQuery> *const q = make_shared<SpanTermQuery>(term, states);
  topLevel.push_back(q);
}

shared_ptr<SpanQuery>
SpanMultiTermQueryWrapper<Q>::SpanRewriteMethodAnonymousInnerClass::rewrite(
    shared_ptr<IndexReader> reader,
    shared_ptr<MultiTermQuery> query) 
{
  return std::static_pointer_cast<SpanQuery>(delegate_::rewrite(reader, query));
}

SpanMultiTermQueryWrapper<Q>::TopTermsSpanBooleanQueryRewrite::
    TopTermsSpanBooleanQueryRewrite(int size)
    : delegate_(
          make_shared<TopTermsRewrite<deque<std::shared_ptr<SpanQuery>>>>(
              size))
{
  {protected : int getMaxSize(){return numeric_limits<int>::max();
}

deque<std::shared_ptr<SpanQuery>> getTopLevelBuilder()
{
  return deque<std::shared_ptr<SpanQuery>>();
}

Query build(deque<std::shared_ptr<SpanQuery>> builder)
{
  return make_shared<SpanOrQuery>(builder::toArray(
      std::deque<std::shared_ptr<SpanQuery>>(builder->size())));
}

void addClause(deque<std::shared_ptr<SpanQuery>> topLevel, Term term,
               int docFreq, float boost, TermContext states)
{
  shared_ptr<SpanTermQuery> *const q = make_shared<SpanTermQuery>(term, states);
  topLevel->add(q);
}
}; // namespace org::apache::lucene::search::spans
}

int SpanMultiTermQueryWrapper<Q>::TopTermsSpanBooleanQueryRewrite::getSize()
{
  return delegate_->getSize();
}

shared_ptr<SpanQuery>
SpanMultiTermQueryWrapper<Q>::TopTermsSpanBooleanQueryRewrite::rewrite(
    shared_ptr<IndexReader> reader,
    shared_ptr<MultiTermQuery> query) 
{
  return std::static_pointer_cast<SpanQuery>(delegate_->rewrite(reader, query));
}

int SpanMultiTermQueryWrapper<Q>::TopTermsSpanBooleanQueryRewrite::hashCode()
{
  return 31 * delegate_->hashCode();
}

bool SpanMultiTermQueryWrapper<Q>::TopTermsSpanBooleanQueryRewrite::equals(
    any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<TopTermsSpanBooleanQueryRewrite> *const other =
      any_cast<std::shared_ptr<TopTermsSpanBooleanQueryRewrite>>(obj);
  return delegate_->equals(other->delegate_);
}
}