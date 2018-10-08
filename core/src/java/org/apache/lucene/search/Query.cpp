using namespace std;

#include "Query.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;

wstring Query::toString() { return toString(L""); }

shared_ptr<Weight> Query::createWeight(shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost) 
{
  throw make_shared<UnsupportedOperationException>(
      L"Query " + shared_from_this() + L" does not implement createWeight");
}

shared_ptr<Query>
Query::rewrite(shared_ptr<IndexReader> reader) 
{
  return shared_from_this();
}

bool Query::sameClassAs(any other)
{
  return other != nullptr && getClass() == other.type();
}

int Query::classHash() { return CLASS_NAME_HASH; }
} // namespace org::apache::lucene::search