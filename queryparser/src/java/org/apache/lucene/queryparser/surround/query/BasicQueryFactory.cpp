using namespace std;

#include "BasicQueryFactory.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Term = org::apache::lucene::index::Term;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;

BasicQueryFactory::BasicQueryFactory(int maxBasicQueries)
{
  this->maxBasicQueries = maxBasicQueries;
  this->queriesMade = 0;
}

BasicQueryFactory::BasicQueryFactory() : BasicQueryFactory(1024) {}

int BasicQueryFactory::getNrQueriesMade() { return queriesMade; }

int BasicQueryFactory::getMaxBasicQueries() { return maxBasicQueries; }

wstring BasicQueryFactory::toString()
{
  return getClassName() + L"(maxBasicQueries: " + to_wstring(maxBasicQueries) +
         L", queriesMade: " + to_wstring(queriesMade) + L")";
}

bool BasicQueryFactory::atMax() { return queriesMade >= maxBasicQueries; }

// C++ WARNING: The following method was originally marked 'synchronized':
void BasicQueryFactory::checkMax() 
{
  if (atMax()) {
    throw make_shared<TooManyBasicQueries>(getMaxBasicQueries());
  }
  queriesMade++;
}

shared_ptr<TermQuery> BasicQueryFactory::newTermQuery(
    shared_ptr<Term> term) 
{
  checkMax();
  return make_shared<TermQuery>(term);
}

shared_ptr<SpanTermQuery> BasicQueryFactory::newSpanTermQuery(
    shared_ptr<Term> term) 
{
  checkMax();
  return make_shared<SpanTermQuery>(term);
}

int BasicQueryFactory::hashCode()
{
  return getClass().hashCode() ^ (atMax() ? 7 : 31 * 32);
}

bool BasicQueryFactory::equals(any obj)
{
  if (!(std::dynamic_pointer_cast<BasicQueryFactory>(obj) != nullptr)) {
    return false;
  }
  shared_ptr<BasicQueryFactory> other =
      any_cast<std::shared_ptr<BasicQueryFactory>>(obj);
  return atMax() == other->atMax();
}
} // namespace org::apache::lucene::queryparser::surround::query