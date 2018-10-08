using namespace std;

#include "SrndQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;

SrndQuery::SrndQuery() {}

void SrndQuery::setWeight(float w)
{
  weight = w; // as parsed from the query text
  weighted = true;
}

bool SrndQuery::isWeighted() { return weighted; }

float SrndQuery::getWeight() { return weight; }

wstring SrndQuery::getWeightString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(getWeight());
}

wstring SrndQuery::getWeightOperator() { return L"^"; }

void SrndQuery::weightToString(shared_ptr<StringBuilder> r)
{ // append the weight part of a query
  if (isWeighted()) {
    r->append(getWeightOperator());
    r->append(getWeightString());
  }
}

shared_ptr<Query>
SrndQuery::makeLuceneQueryField(const wstring &fieldName,
                                shared_ptr<BasicQueryFactory> qf)
{
  shared_ptr<Query> q = makeLuceneQueryFieldNoBoost(fieldName, qf);
  if (isWeighted()) {
    q = make_shared<BoostQuery>(
        q, getWeight()); // weight may be at any level in a SrndQuery
  }
  return q;
}

bool SrndQuery::isFieldsSubQueryAcceptable() { return true; }

shared_ptr<SrndQuery> SrndQuery::clone()
{
  try {
    return std::static_pointer_cast<SrndQuery>(__super::clone());
  } catch (const CloneNotSupportedException &cns) {
    throw make_shared<Error>(cns);
  }
}

int SrndQuery::hashCode()
{
  return getClass().hashCode() ^ toString().hashCode();
}

bool SrndQuery::equals(any obj)
{
  if (obj == nullptr) {
    return false;
  }
  if (!getClass().equals(obj.type())) {
    return false;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return toString() == obj.toString();
}
} // namespace org::apache::lucene::queryparser::surround::query