using namespace std;

#include "FieldMaskingSpanQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

FieldMaskingSpanQuery::FieldMaskingSpanQuery(shared_ptr<SpanQuery> maskedQuery,
                                             const wstring &maskedField)
    : maskedQuery(Objects::requireNonNull(maskedQuery)),
      field(Objects::requireNonNull(maskedField))
{
}

wstring FieldMaskingSpanQuery::getField() { return field; }

shared_ptr<SpanQuery> FieldMaskingSpanQuery::getMaskedQuery()
{
  return maskedQuery;
}

shared_ptr<SpanWeight>
FieldMaskingSpanQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  return maskedQuery->createWeight(searcher, needsScores, boost);
}

shared_ptr<Query> FieldMaskingSpanQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<SpanQuery> rewritten =
      std::static_pointer_cast<SpanQuery>(maskedQuery->rewrite(reader));
  if (rewritten != maskedQuery) {
    return make_shared<FieldMaskingSpanQuery>(rewritten, field);
  }

  return SpanQuery::rewrite(reader);
}

wstring FieldMaskingSpanQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"mask(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(maskedQuery->toString(field));
  buffer->append(L")");
  buffer->append(L" as ");
  buffer->append(this->field);
  return buffer->toString();
}

bool FieldMaskingSpanQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool FieldMaskingSpanQuery::equalsTo(shared_ptr<FieldMaskingSpanQuery> other)
{
  return getField() == other->getField() &&
         getMaskedQuery()->equals(other->getMaskedQuery());
}

int FieldMaskingSpanQuery::hashCode()
{
  return classHash() ^ getMaskedQuery()->hashCode() ^ getField().hashCode();
}
} // namespace org::apache::lucene::search::spans