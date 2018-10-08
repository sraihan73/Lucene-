using namespace std;

#include "BooleanClause.h"

namespace org::apache::lucene::search
{

BooleanClause::BooleanClause(shared_ptr<Query> query, Occur occur)
    : query(Objects::requireNonNull(query, L"Query must not be null")),
      occur(Objects::requireNonNull(occur, L"Occur must not be null"))
{
}

BooleanClause::Occur BooleanClause::getOccur() { return occur; }

shared_ptr<Query> BooleanClause::getQuery() { return query; }

bool BooleanClause::isProhibited() { return Occur::MUST_NOT == occur; }

bool BooleanClause::isRequired()
{
  return occur == Occur::MUST || occur == Occur::FILTER;
}

bool BooleanClause::isScoring()
{
  return occur == Occur::MUST || occur == Occur::SHOULD;
}

bool BooleanClause::equals(any o)
{
  if (o == nullptr ||
      !(std::dynamic_pointer_cast<BooleanClause>(o) != nullptr)) {
    return false;
  }
  shared_ptr<BooleanClause> other = any_cast<std::shared_ptr<BooleanClause>>(o);
  return this->query->equals(other->query) && this->occur == other->occur;
}

int BooleanClause::hashCode()
{
  return 31 * query->hashCode() + occur.hashCode();
}

wstring BooleanClause::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return occur.toString() + query->toString();
}
} // namespace org::apache::lucene::search