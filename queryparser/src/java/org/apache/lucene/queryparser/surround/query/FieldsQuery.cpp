using namespace std;

#include "FieldsQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;
const wstring FieldsQuery::OR_OPERATOR_NAME = L"OR";

FieldsQuery::FieldsQuery(shared_ptr<SrndQuery> q, deque<wstring> &fieldNames,
                         wchar_t fieldOp)
    : fieldOp(fieldOp)
{
  this->q = q;
  this->fieldNames = fieldNames;
}

FieldsQuery::FieldsQuery(shared_ptr<SrndQuery> q, const wstring &fieldName,
                         wchar_t fieldOp)
    : fieldOp(fieldOp)
{
  this->q = q;
  fieldNames = deque<>();
  fieldNames.push_back(fieldName);
}

bool FieldsQuery::isFieldsSubQueryAcceptable() { return false; }

shared_ptr<Query>
FieldsQuery::makeLuceneQueryNoBoost(shared_ptr<BasicQueryFactory> qf)
{
  if (fieldNames.size() == 1) { // single field name: no new queries needed
    return q->makeLuceneQueryFieldNoBoost(fieldNames[0], qf);
  } else { // OR query over the fields
    deque<std::shared_ptr<SrndQuery>> queries =
        deque<std::shared_ptr<SrndQuery>>();
    Iterator<wstring> fni = getFieldNames().begin();
    shared_ptr<SrndQuery> qc;
    while (fni->hasNext()) {
      qc = q->clone();
      queries.push_back(make_shared<FieldsQuery>(qc, fni->next(), fieldOp));
      fni++;
    }
    shared_ptr<OrQuery> oq =
        make_shared<OrQuery>(queries, true, OR_OPERATOR_NAME);
    // System.out.println(getClass().toString() + ", fields expanded: " +
    // oq.toString()); /* needs testing */
    return oq->makeLuceneQueryField(L"", qf);
  }
}

shared_ptr<Query>
FieldsQuery::makeLuceneQueryFieldNoBoost(const wstring &fieldName,
                                         shared_ptr<BasicQueryFactory> qf)
{
  return makeLuceneQueryNoBoost(qf); // use this.fieldNames instead of fieldName
}

deque<wstring> FieldsQuery::getFieldNames() { return fieldNames; }

wchar_t FieldsQuery::getFieldOperator() { return fieldOp; }

wstring FieldsQuery::toString()
{
  shared_ptr<StringBuilder> r = make_shared<StringBuilder>();
  r->append(L"(");
  fieldNamesToString(r);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  r->append(q->toString());
  r->append(L")");
  return r->toString();
}

void FieldsQuery::fieldNamesToString(shared_ptr<StringBuilder> r)
{
  Iterator<wstring> fni = getFieldNames().begin();
  while (fni->hasNext()) {
    r->append(fni->next());
    r->append(getFieldOperator());
    fni++;
  }
}
} // namespace org::apache::lucene::queryparser::surround::query