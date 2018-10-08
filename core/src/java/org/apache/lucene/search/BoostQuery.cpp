using namespace std;

#include "BoostQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;

BoostQuery::BoostQuery(shared_ptr<Query> query, float boost)
    : query(Objects::requireNonNull(query)), boost(boost)
{
}

shared_ptr<Query> BoostQuery::getQuery() { return query; }

float BoostQuery::getBoost() { return boost; }

bool BoostQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool BoostQuery::equalsTo(shared_ptr<BoostQuery> other)
{
  return query->equals(other->query) &&
         Float::floatToIntBits(boost) == Float::floatToIntBits(other->boost);
}

int BoostQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + query->hashCode();
  h = 31 * h + Float::floatToIntBits(boost);
  return h;
}

shared_ptr<Query>
BoostQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const rewritten = query->rewrite(reader);

  if (boost == 1.0f) {
    return rewritten;
  }

  if (rewritten->getClass() == BoostQuery::typeid) {
    shared_ptr<BoostQuery> in_ =
        std::static_pointer_cast<BoostQuery>(rewritten);
    return make_shared<BoostQuery>(in_->query, boost * in_->boost);
  }

  if (boost == 0.0f && rewritten->getClass() != ConstantScoreQuery::typeid) {
    // so that we pass needScores=false
    return make_shared<BoostQuery>(make_shared<ConstantScoreQuery>(rewritten),
                                   0.0f);
  }

  if (query != rewritten) {
    return make_shared<BoostQuery>(rewritten, boost);
  }

  return Query::rewrite(reader);
}

wstring BoostQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  builder->append(L"(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  builder->append(query->toString(field));
  builder->append(L")");
  builder->append(L"^");
  builder->append(boost);
  return builder->toString();
}

shared_ptr<Weight> BoostQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                            bool needsScores,
                                            float boost) 
{
  return query->createWeight(searcher, needsScores,
                             BoostQuery::this->boost * boost);
}
} // namespace org::apache::lucene::search