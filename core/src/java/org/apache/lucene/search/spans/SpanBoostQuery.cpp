using namespace std;

#include "SpanBoostQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

SpanBoostQuery::SpanBoostQuery(shared_ptr<SpanQuery> query, float boost)
    : query(Objects::requireNonNull(query)), boost(boost)
{
}

shared_ptr<SpanQuery> SpanBoostQuery::getQuery() { return query; }

float SpanBoostQuery::getBoost() { return boost; }

bool SpanBoostQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool SpanBoostQuery::equalsTo(shared_ptr<SpanBoostQuery> other)
{
  return query->equals(other->query) &&
         Float::floatToIntBits(boost) == Float::floatToIntBits(other->boost);
}

int SpanBoostQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + query->hashCode();
  h = 31 * h + Float::floatToIntBits(boost);
  return h;
}

shared_ptr<Query>
SpanBoostQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (boost == 1.0f) {
    return query;
  }

  shared_ptr<SpanQuery> *const rewritten =
      std::static_pointer_cast<SpanQuery>(query->rewrite(reader));
  if (query != rewritten) {
    return make_shared<SpanBoostQuery>(rewritten, boost);
  }

  if (query->getClass() == SpanBoostQuery::typeid) {
    shared_ptr<SpanBoostQuery> in_ =
        std::static_pointer_cast<SpanBoostQuery>(query);
    return make_shared<SpanBoostQuery>(in_->query, boost * in_->boost);
  }

  return SpanQuery::rewrite(reader);
}

wstring SpanBoostQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  builder->append(L"(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  builder->append(query->toString(field));
  builder->append(L")^");
  builder->append(boost);
  return builder->toString();
}

wstring SpanBoostQuery::getField() { return query->getField(); }

shared_ptr<SpanWeight>
SpanBoostQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                             bool needsScores, float boost) 
{
  return query->createWeight(searcher, needsScores,
                             SpanBoostQuery::this->boost * boost);
}
} // namespace org::apache::lucene::search::spans