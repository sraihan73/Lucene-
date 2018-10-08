using namespace std;

#include "AssertingQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;

AssertingQuery::AssertingQuery(shared_ptr<Random> random, shared_ptr<Query> in_)
    : random(random), in_(in_)
{
}

shared_ptr<Query> AssertingQuery::wrap(shared_ptr<Random> random,
                                       shared_ptr<Query> query)
{
  return std::dynamic_pointer_cast<AssertingQuery>(query) != nullptr
             ? query
             : make_shared<AssertingQuery>(random, query);
}

shared_ptr<Weight>
AssertingQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                             bool needsScores, float boost) 
{
  return make_shared<AssertingWeight>(
      make_shared<Random>(random->nextLong()),
      in_->createWeight(searcher, needsScores, boost), needsScores);
}

wstring AssertingQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return in_->toString(field);
}

bool AssertingQuery::equals(any other)
{
  return sameClassAs(other) &&
         in_->equals((any_cast<std::shared_ptr<AssertingQuery>>(other)).in_);
}

int AssertingQuery::hashCode() { return -in_->hashCode(); }

shared_ptr<Random> AssertingQuery::getRandom() { return random; }

shared_ptr<Query> AssertingQuery::getIn() { return in_; }

shared_ptr<Query>
AssertingQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const rewritten = in_->rewrite(reader);
  if (rewritten == in_) {
    return Query::rewrite(reader);
  } else {
    return wrap(make_shared<Random>(random->nextLong()), rewritten);
  }
}
} // namespace org::apache::lucene::search